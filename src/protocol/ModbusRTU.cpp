#include "ModbusRTU.h"
#include "comm/CRC16.h"
#include "core/Logger.h"
#include <QThread>

namespace rcms {

ModbusRTU::ModbusRTU() = default;
ModbusRTU::~ModbusRTU() = default;

bool ModbusRTU::readHoldingRegisters(uint8_t address, uint16_t startReg,
                                      uint16_t count, std::vector<uint16_t>& values) {
    if (!m_port || !m_port->isOpen()) {
        m_lastError = "Port not open";
        return false;
    }

    // Build request: [addr][func][startHi][startLo][countHi][countLo][crcLo][crcHi]
    std::vector<uint8_t> request = {
        address,
        FUNC_READ_HOLDING,
        static_cast<uint8_t>(startReg >> 8),
        static_cast<uint8_t>(startReg & 0xFF),
        static_cast<uint8_t>(count >> 8),
        static_cast<uint8_t>(count & 0xFF)
    };
    CRC16::append(request);

    if (!sendRequest(request)) {
        return false;
    }

    // Expected response: [addr][func][byteCount][data...][crcLo][crcHi]
    int expectedLen = 3 + count * 2 + 2;
    std::vector<uint8_t> response;
    if (!readResponse(response, expectedLen)) {
        return false;
    }

    // Verify CRC
    if (!CRC16::verify(response.data(), response.size())) {
        m_lastError = "CRC error in response";
        Logger::error("Modbus CRC error");
        return false;
    }

    // Check for error response
    if (response[1] & 0x80) {
        m_lastError = QString("Modbus error: 0x%1").arg(response[2], 2, 16, QChar('0'));
        Logger::error("Modbus error response: 0x{:02X}", response[2]);
        return false;
    }

    // Extract register values
    values.clear();
    uint8_t byteCount = response[2];
    for (int i = 0; i < byteCount / 2; ++i) {
        uint16_t val = (static_cast<uint16_t>(response[3 + i * 2]) << 8) |
                        response[4 + i * 2];
        values.push_back(val);
    }

    return true;
}

bool ModbusRTU::writeSingleRegister(uint8_t address, uint16_t reg, uint16_t value) {
    if (!m_port || !m_port->isOpen()) {
        m_lastError = "Port not open";
        return false;
    }

    // Build request: [addr][func][regHi][regLo][valHi][valLo][crcLo][crcHi]
    std::vector<uint8_t> request = {
        address,
        FUNC_WRITE_SINGLE,
        static_cast<uint8_t>(reg >> 8),
        static_cast<uint8_t>(reg & 0xFF),
        static_cast<uint8_t>(value >> 8),
        static_cast<uint8_t>(value & 0xFF)
    };
    CRC16::append(request);

    if (!sendRequest(request)) {
        return false;
    }

    // Echo response expected
    std::vector<uint8_t> response;
    if (!readResponse(response, 8)) {
        return false;
    }

    if (!CRC16::verify(response.data(), response.size())) {
        m_lastError = "CRC error in response";
        return false;
    }

    if (response[1] & 0x80) {
        m_lastError = QString("Modbus error: 0x%1").arg(response[2], 2, 16, QChar('0'));
        return false;
    }

    return true;
}

bool ModbusRTU::writeMultipleRegisters(uint8_t address, uint16_t startReg,
                                        const std::vector<uint16_t>& values) {
    if (!m_port || !m_port->isOpen()) {
        m_lastError = "Port not open";
        return false;
    }

    uint16_t count = static_cast<uint16_t>(values.size());
    uint8_t byteCount = count * 2;

    // Build request
    std::vector<uint8_t> request = {
        address,
        FUNC_WRITE_MULTIPLE,
        static_cast<uint8_t>(startReg >> 8),
        static_cast<uint8_t>(startReg & 0xFF),
        static_cast<uint8_t>(count >> 8),
        static_cast<uint8_t>(count & 0xFF),
        byteCount
    };

    for (uint16_t val : values) {
        request.push_back(static_cast<uint8_t>(val >> 8));
        request.push_back(static_cast<uint8_t>(val & 0xFF));
    }
    CRC16::append(request);

    if (!sendRequest(request)) {
        return false;
    }

    // Response: [addr][func][startHi][startLo][countHi][countLo][crcLo][crcHi]
    std::vector<uint8_t> response;
    if (!readResponse(response, 8)) {
        return false;
    }

    if (!CRC16::verify(response.data(), response.size())) {
        m_lastError = "CRC error in response";
        return false;
    }

    if (response[1] & 0x80) {
        m_lastError = QString("Modbus error: 0x%1").arg(response[2], 2, 16, QChar('0'));
        return false;
    }

    return true;
}

bool ModbusRTU::sendRequest(const std::vector<uint8_t>& request) {
    m_port->clear();

    qint64 written = m_port->write(reinterpret_cast<const char*>(request.data()),
                                    static_cast<qint64>(request.size()));

    if (written != static_cast<qint64>(request.size())) {
        m_lastError = "Failed to write request";
        return false;
    }

    m_port->flush();

    // Inter-frame delay (3.5 char times at 9600 baud ≈ 4ms)
    QThread::msleep(5);

    return true;
}

bool ModbusRTU::readResponse(std::vector<uint8_t>& response, int expectedLen) {
    response.clear();

    if (!m_port->waitForReadyRead(m_timeout)) {
        m_lastError = "Response timeout";
        Logger::warn("Modbus response timeout");
        return false;
    }

    QByteArray data = m_port->readAll();

    // Wait for more data if needed
    while (data.size() < expectedLen && m_port->waitForReadyRead(100)) {
        data.append(m_port->readAll());
    }

    response.assign(data.begin(), data.end());

    if (static_cast<int>(response.size()) < expectedLen) {
        m_lastError = QString("Incomplete response: got %1 bytes, expected %2")
                          .arg(response.size()).arg(expectedLen);
        return false;
    }

    return true;
}

} // namespace rcms
