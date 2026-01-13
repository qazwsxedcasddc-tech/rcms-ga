#include "Fazan19Emulator.h"
#include <cmath>
#include <cstring>
#include <thread>
#include <chrono>

namespace rcms {
namespace test {

// Register addresses (matching Fazan19Registers.h per РЭ)
namespace reg {
    constexpr uint16_t CountWork = 0x00;  // Operating hours (single 16-bit)
    constexpr uint16_t Cntr = 0x01;       // Request counter
    constexpr uint16_t ModTR = 0x02;      // Mode register (TX/RX, power)
    constexpr uint16_t FrRS = 0x03;       // Frequency + KF coefficient
    constexpr uint16_t PKm = 0x04;        // Power level
    constexpr uint16_t AD0 = 0x10;        // ADC channel 0 (voltage)
    constexpr uint16_t AD1 = 0x11;        // ADC channel 1 (temperature)
    constexpr uint16_t AD2 = 0x12;        // ADC channel 2 (signal level)
    constexpr uint16_t DiagVUU = 0x18;    // Diagnostic register (8 bytes)

    // Legacy aliases for backward compatibility
    constexpr uint16_t CW1 = CountWork;
    constexpr uint16_t MR1 = ModTR;
    constexpr uint16_t FRRS = FrRS;
    constexpr uint16_t DV1 = DiagVUU;
    constexpr uint16_t DV2 = DiagVUU + 1;
    constexpr uint16_t DV3 = DiagVUU + 2;
    constexpr uint16_t DV4 = DiagVUU + 3;
}

// Mode register bits
namespace mode {
    constexpr uint16_t TX = 0x0001;         // Transmitting
    constexpr uint16_t SQUELCH = 0x0080;    // Squelch enabled
    constexpr uint16_t REMOTE = 0x0100;     // Remote control mode
    constexpr uint16_t DATA_MODE = 0x0200;  // Data mode (vs voice)
    constexpr uint16_t WIRE_4 = 0x0400;     // 4-wire mode
}

// Frequency constants
constexpr double FREQ_STEP_HZ = 8333.33333;
constexpr double FREQ_BASE_MHZ = 100.0;

Fazan19Emulator::Fazan19Emulator(uint8_t address)
    : m_address(address)
{
    // Initialize with default values
    m_registers.fill(0);

    // Set default frequency 121.5 MHz (emergency)
    setFrequency(121.5);

    // Set some operating hours
    setOperatingHours(1234);

    // Set default ADC values
    m_registers[reg::AD0] = 240;  // ~24.0V
    m_registers[reg::AD1] = 250;  // ~25.0°C
    m_registers[reg::AD2] = 50;   // Signal level

    // Remote mode enabled
    setRemoteMode(true);
}

std::vector<uint8_t> Fazan19Emulator::processRequest(const std::vector<uint8_t>& request) {
    // Check if we're online
    if (!m_online) {
        return {};  // No response
    }

    // Minimum request size: addr(1) + func(1) + data(2+) + crc(2)
    if (request.size() < 6) {
        return {};
    }

    // Check address
    if (request[0] != m_address) {
        return {};  // Not for us
    }

    // Verify CRC
    if (!verifyCRC(request)) {
        return {};  // Bad CRC, ignore
    }

    // Simulate response delay
    if (m_responseDelayMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_responseDelayMs));
    }

    std::vector<uint8_t> response;

    uint8_t funcCode = request[1];
    switch (funcCode) {
        case FUNC_READ_HOLDING:
            response = handleReadHolding(request);
            break;
        case FUNC_WRITE_SINGLE:
            response = handleWriteSingle(request);
            break;
        case FUNC_WRITE_MULTIPLE:
            response = handleWriteMultiple(request);
            break;
        case FUNC_DEVICE_ID:
            response = handleDeviceId(request);
            break;
        default:
            response = makeErrorResponse(funcCode, 0x01);  // Illegal function
            break;
    }

    // Log request/response
    if (m_requestCallback) {
        m_requestCallback(request, response);
    }

    return response;
}

std::vector<uint8_t> Fazan19Emulator::handleReadHolding(const std::vector<uint8_t>& request) {
    if (request.size() < 8) {
        return makeErrorResponse(FUNC_READ_HOLDING, 0x03);
    }

    uint16_t startAddr = (static_cast<uint16_t>(request[2]) << 8) | request[3];
    uint16_t count = (static_cast<uint16_t>(request[4]) << 8) | request[5];

    // Validate range
    if (startAddr + count > REGISTER_COUNT) {
        return makeErrorResponse(FUNC_READ_HOLDING, 0x02);  // Illegal address
    }

    if (count == 0 || count > 125) {
        return makeErrorResponse(FUNC_READ_HOLDING, 0x03);  // Illegal value
    }

    // Build response
    std::vector<uint8_t> response;
    response.push_back(m_address);
    response.push_back(FUNC_READ_HOLDING);
    response.push_back(static_cast<uint8_t>(count * 2));  // Byte count

    for (uint16_t i = 0; i < count; ++i) {
        uint16_t val = m_registers[startAddr + i];
        response.push_back(static_cast<uint8_t>(val >> 8));
        response.push_back(static_cast<uint8_t>(val & 0xFF));
    }

    appendCRC(response);
    return response;
}

std::vector<uint8_t> Fazan19Emulator::handleWriteSingle(const std::vector<uint8_t>& request) {
    if (request.size() < 8) {
        return makeErrorResponse(FUNC_WRITE_SINGLE, 0x03);
    }

    uint16_t regAddr = (static_cast<uint16_t>(request[2]) << 8) | request[3];
    uint16_t value = (static_cast<uint16_t>(request[4]) << 8) | request[5];

    if (regAddr >= REGISTER_COUNT) {
        return makeErrorResponse(FUNC_WRITE_SINGLE, 0x02);
    }

    m_registers[regAddr] = value;

    // Echo back the request (standard Modbus response)
    std::vector<uint8_t> response(request.begin(), request.begin() + 6);
    response[0] = m_address;
    appendCRC(response);
    return response;
}

std::vector<uint8_t> Fazan19Emulator::handleWriteMultiple(const std::vector<uint8_t>& request) {
    if (request.size() < 9) {
        return makeErrorResponse(FUNC_WRITE_MULTIPLE, 0x03);
    }

    uint16_t startAddr = (static_cast<uint16_t>(request[2]) << 8) | request[3];
    uint16_t count = (static_cast<uint16_t>(request[4]) << 8) | request[5];
    uint8_t byteCount = request[6];

    if (startAddr + count > REGISTER_COUNT) {
        return makeErrorResponse(FUNC_WRITE_MULTIPLE, 0x02);
    }

    if (byteCount != count * 2 || request.size() < static_cast<size_t>(9 + byteCount)) {
        return makeErrorResponse(FUNC_WRITE_MULTIPLE, 0x03);
    }

    // Write registers
    for (uint16_t i = 0; i < count; ++i) {
        uint16_t val = (static_cast<uint16_t>(request[7 + i * 2]) << 8) |
                        request[8 + i * 2];
        m_registers[startAddr + i] = val;
    }

    // Response: addr + func + startAddr + count
    std::vector<uint8_t> response;
    response.push_back(m_address);
    response.push_back(FUNC_WRITE_MULTIPLE);
    response.push_back(request[2]);
    response.push_back(request[3]);
    response.push_back(request[4]);
    response.push_back(request[5]);
    appendCRC(response);
    return response;
}

std::vector<uint8_t> Fazan19Emulator::handleDeviceId(const std::vector<uint8_t>& /*request*/) {
    // Return device identification
    std::vector<uint8_t> response;
    response.push_back(m_address);
    response.push_back(FUNC_DEVICE_ID);

    // Device ID string: "Fazan-19 P5 Emulator"
    const char* deviceId = "Fazan-19 P5 EMU";
    size_t len = strlen(deviceId);
    response.push_back(static_cast<uint8_t>(len));
    for (size_t i = 0; i < len; ++i) {
        response.push_back(static_cast<uint8_t>(deviceId[i]));
    }

    appendCRC(response);
    return response;
}

std::vector<uint8_t> Fazan19Emulator::makeErrorResponse(uint8_t funcCode, uint8_t errorCode) {
    std::vector<uint8_t> response;
    response.push_back(m_address);
    response.push_back(funcCode | 0x80);  // Error flag
    response.push_back(errorCode);
    appendCRC(response);
    return response;
}

uint16_t Fazan19Emulator::getRegister(uint16_t addr) const {
    if (addr < REGISTER_COUNT) {
        return m_registers[addr];
    }
    return 0;
}

void Fazan19Emulator::setRegister(uint16_t addr, uint16_t value) {
    if (addr < REGISTER_COUNT) {
        m_registers[addr] = value;
    }
}

void Fazan19Emulator::setFrequency(double freqMHz) {
    m_registers[reg::FRRS] = encodeFrequency(freqMHz);
}

double Fazan19Emulator::getFrequency() const {
    return decodeFrequency(m_registers[reg::FRRS]);
}

void Fazan19Emulator::setOperatingHours(uint32_t hours) {
    // Per РЭ, operating hours is stored in a single 16-bit register
    // If hours exceed 16-bit max, saturate at 65535
    m_registers[reg::CountWork] = static_cast<uint16_t>(
        hours > 65535 ? 65535 : hours
    );
}

void Fazan19Emulator::setError(uint16_t dv1, uint16_t dv2, uint16_t dv3, uint16_t dv4) {
    m_registers[reg::DV1] = dv1;
    m_registers[reg::DV2] = dv2;
    m_registers[reg::DV3] = dv3;
    m_registers[reg::DV4] = dv4;
}

void Fazan19Emulator::clearErrors() {
    setError(0, 0, 0, 0);
}

void Fazan19Emulator::setRemoteMode(bool remote) {
    if (remote) {
        m_registers[reg::MR1] |= mode::REMOTE;
    } else {
        m_registers[reg::MR1] &= ~mode::REMOTE;
    }
}

void Fazan19Emulator::setTransmitting(bool tx) {
    if (tx) {
        m_registers[reg::MR1] |= mode::TX;
    } else {
        m_registers[reg::MR1] &= ~mode::TX;
    }
}

void Fazan19Emulator::setSquelchOpen(bool open) {
    if (open) {
        m_registers[reg::MR1] |= mode::SQUELCH;
    } else {
        m_registers[reg::MR1] &= ~mode::SQUELCH;
    }
}

uint16_t Fazan19Emulator::encodeFrequency(double freqMHz, uint8_t kf) {
    double diffHz = (freqMHz - FREQ_BASE_MHZ) * 1000000.0;
    uint16_t f12 = static_cast<uint16_t>(std::round(diffHz / FREQ_STEP_HZ));
    return (static_cast<uint16_t>(kf & 0x03) << 13) | (f12 & 0x1FFF);
}

double Fazan19Emulator::decodeFrequency(uint16_t frrs) {
    uint16_t f12 = frrs & 0x1FFF;
    return FREQ_BASE_MHZ + (f12 * FREQ_STEP_HZ) / 1000000.0;
}

uint16_t Fazan19Emulator::calculateCRC(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void Fazan19Emulator::appendCRC(std::vector<uint8_t>& data) {
    uint16_t crc = calculateCRC(data.data(), data.size());
    data.push_back(static_cast<uint8_t>(crc & 0xFF));        // Low byte first
    data.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF)); // High byte
}

bool Fazan19Emulator::verifyCRC(const std::vector<uint8_t>& data) {
    if (data.size() < 3) return false;

    size_t dataLen = data.size() - 2;
    uint16_t calculated = calculateCRC(data.data(), dataLen);
    uint16_t received = static_cast<uint16_t>(data[dataLen]) |
                        (static_cast<uint16_t>(data[dataLen + 1]) << 8);

    return calculated == received;
}

} // namespace test
} // namespace rcms
