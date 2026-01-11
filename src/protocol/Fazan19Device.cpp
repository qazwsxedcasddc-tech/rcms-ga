#include "Fazan19Device.h"
#include "core/Logger.h"
#include <cmath>

namespace rcms {

using namespace fazan19;

Fazan19Device::Fazan19Device(uint8_t address)
    : m_address(address)
    , m_modbus(std::make_unique<ModbusRTU>())
{
    m_deviceId = QString("Fazan19_%1").arg(address);
}

Fazan19Device::~Fazan19Device() {
    close();
}

bool Fazan19Device::open(const QString& portName, int baudRate) {
    close();

    m_port = std::make_unique<QSerialPort>();
    m_port->setPortName(portName);
    m_port->setBaudRate(baudRate);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_port->open(QIODevice::ReadWrite)) {
        Logger::error("Failed to open port {}: {}",
                      portName.toStdString(),
                      m_port->errorString().toStdString());
        return false;
    }

    m_modbus->setPort(m_port.get());
    m_modbus->setTimeout(timing::RESPONSE_TIMEOUT_MS);

    Logger::info("Opened port {} for Fazan-19 (addr: {})",
                 portName.toStdString(), m_address);

    return true;
}

void Fazan19Device::close() {
    if (m_port && m_port->isOpen()) {
        m_port->close();
        Logger::info("Closed port for Fazan-19 (addr: {})", m_address);
    }
}

bool Fazan19Device::isOpen() const {
    return m_port && m_port->isOpen();
}

bool Fazan19Device::readStatus(DeviceStatus& status) {
    uint16_t regs[registers::TOTAL_REGISTERS];
    if (!readAllRegisters(regs)) {
        status.online = false;
        return false;
    }

    status.online = true;

    // Operating hours (32-bit from CW1 + CW2)
    m_operatingHours = (static_cast<uint32_t>(regs[registers::CW1]) << 16) |
                        regs[registers::CW2];
    status.operatingHours = m_operatingHours;

    // Frequency
    m_currentFrequency = decodeFrequency(regs[registers::FRRS]);
    status.frequencyMHz = m_currentFrequency;

    // Mode register
    parseModeRegister(regs[registers::MR1], status);

    // ADC values (raw, need calibration)
    // AD0-AD7 contain voltage, temperature, signal level etc.
    // TODO: Apply calibration from documentation
    status.voltage24V = regs[registers::AD0] * 0.1;  // Placeholder
    status.temperature = regs[registers::AD1] * 0.1; // Placeholder
    status.signalLevel = regs[registers::AD2];

    status.lastUpdate = QDateTime::currentDateTime();

    return true;
}

bool Fazan19Device::readAlarms(QVector<AlarmInfo>& alarms) {
    std::vector<uint16_t> values;
    if (!m_modbus->readHoldingRegisters(m_address, registers::DV1, 4, values)) {
        return false;
    }

    if (values.size() >= 4) {
        parseErrors(values[0], values[1], values[2], values[3], alarms);
    }

    return true;
}

bool Fazan19Device::getFrequency(double& freqMHz) {
    freqMHz = m_currentFrequency;
    return true;
}

bool Fazan19Device::setFrequency(double freqMHz) {
    // Validate frequency range (118.000 - 136.975 MHz)
    if (freqMHz < frequency::MIN_MHZ || freqMHz > frequency::MAX_MHZ) {
        Logger::error("Frequency {} MHz out of range", freqMHz);
        return false;
    }

    uint16_t frrs = encodeFrequency(freqMHz);

    if (!m_modbus->writeSingleRegister(m_address, registers::FRRS, frrs)) {
        Logger::error("Failed to set frequency: {}", m_modbus->lastError().toStdString());
        return false;
    }

    m_currentFrequency = freqMHz;
    Logger::info("Set frequency to {} MHz (reg: 0x{:04X})", freqMHz, frrs);

    return true;
}

bool Fazan19Device::setSquelch(bool enabled, int level) {
    // Read current MR1 register
    std::vector<uint16_t> values;
    if (!m_modbus->readHoldingRegisters(m_address, registers::MR1, 1, values)) {
        return false;
    }

    uint16_t mr1 = values[0];

    // Set/clear squelch bit (bit 7)
    if (enabled) {
        mr1 |= modes::MR1_SQUELCH;
    } else {
        mr1 &= ~modes::MR1_SQUELCH;
    }

    if (!m_modbus->writeSingleRegister(m_address, registers::MR1, mr1)) {
        Logger::error("Failed to set squelch: {}", m_modbus->lastError().toStdString());
        return false;
    }

    m_squelchEnabled = enabled;
    m_squelchLevel = level;

    Logger::info("Set squelch: {} (level: {})", enabled ? "ON" : "OFF", level);

    return true;
}

bool Fazan19Device::setPTT(bool enabled) {
    // PTT control via MR1 register
    std::vector<uint16_t> values;
    if (!m_modbus->readHoldingRegisters(m_address, registers::MR1, 1, values)) {
        return false;
    }

    uint16_t mr1 = values[0];

    if (enabled) {
        mr1 |= modes::MR1_TX;
    } else {
        mr1 &= ~modes::MR1_TX;
    }

    if (!m_modbus->writeSingleRegister(m_address, registers::MR1, mr1)) {
        Logger::error("Failed to set PTT: {}", m_modbus->lastError().toStdString());
        return false;
    }

    Logger::info("Set PTT: {}", enabled ? "ON" : "OFF");

    return true;
}

bool Fazan19Device::runSelfTest() {
    // TODO: Implement self-test command if supported
    Logger::info("Running self-test for Fazan-19 (addr: {})", m_address);
    return true;
}

bool Fazan19Device::readAllRegisters(uint16_t* registers) {
    std::vector<uint16_t> values;
    if (!m_modbus->readHoldingRegisters(m_address, 0, registers::TOTAL_REGISTERS, values)) {
        return false;
    }

    for (size_t i = 0; i < values.size() && i < registers::TOTAL_REGISTERS; ++i) {
        registers[i] = values[i];
    }

    return true;
}

uint16_t Fazan19Device::encodeFrequency(double freqMHz, uint8_t kf) {
    double diffHz = (freqMHz - frequency::BASE_MHZ) * 1000000.0;
    uint16_t f12 = static_cast<uint16_t>(std::round(diffHz / frequency::STEP_HZ));
    return (static_cast<uint16_t>(kf & 0x03) << 13) | (f12 & 0x1FFF);
}

double Fazan19Device::decodeFrequency(uint16_t frrs) {
    uint16_t f12 = frrs & 0x1FFF;
    return frequency::BASE_MHZ + (f12 * frequency::STEP_HZ) / 1000000.0;
}

uint8_t Fazan19Device::extractKF(uint16_t frrs) {
    return static_cast<uint8_t>((frrs >> 13) & 0x03);
}

void Fazan19Device::parseModeRegister(uint16_t mr1, DeviceStatus& status) {
    status.isTransmitting = (mr1 & modes::MR1_TX) != 0;
    status.squelchEnabled = (mr1 & modes::MR1_SQUELCH) != 0;

    // Control mode
    if (mr1 & modes::MR1_REMOTE) {
        status.mode = "ДУ";  // Remote control
    } else {
        status.mode = "МУ";  // Local control
    }

    // Work mode
    if (mr1 & modes::MR1_DATA_MODE) {
        status.workMode = "ДАН";  // Data mode
    } else {
        status.workMode = "ТЛФ";  // Telephone mode
    }

    // Line type
    if (mr1 & modes::MR1_4WIRE) {
        status.lineType = "4-х";  // 4-wire
    } else {
        status.lineType = "2-х";  // 2-wire
    }
}

void Fazan19Device::parseErrors(uint16_t dv1, uint16_t dv2, uint16_t dv3, uint16_t dv4,
                                 QVector<AlarmInfo>& alarms) {
    // Parse error codes from DV1-DV4 registers
    // Each bit represents a specific error condition

    auto addAlarm = [&alarms](uint16_t code, const QString& message, AlarmSeverity severity) {
        AlarmInfo info;
        info.code = code;
        info.message = message;
        info.severity = severity;
        info.timestamp = QDateTime::currentDateTime();
        alarms.append(info);
    };

    // DV1 - Critical errors
    if (dv1 & errors::DV1_POWER_FAIL) {
        addAlarm(0x0101, "Отказ питания 24В", AlarmSeverity::Critical);
    }
    if (dv1 & errors::DV1_PLL_UNLOCK) {
        addAlarm(0x0102, "Срыв ФАПЧ синтезатора", AlarmSeverity::Critical);
    }
    if (dv1 & errors::DV1_PA_FAIL) {
        addAlarm(0x0103, "Отказ усилителя мощности", AlarmSeverity::Critical);
    }
    if (dv1 & errors::DV1_VSWR_HIGH) {
        addAlarm(0x0104, "КСВ антенны превышен", AlarmSeverity::Error);
    }
    if (dv1 & errors::DV1_TEMP_HIGH) {
        addAlarm(0x0105, "Перегрев устройства", AlarmSeverity::Warning);
    }

    // DV2 - Secondary errors
    if (dv2 & errors::DV2_RX_FAIL) {
        addAlarm(0x0201, "Отказ приёмника", AlarmSeverity::Error);
    }
    if (dv2 & errors::DV2_BATTERY_LOW) {
        addAlarm(0x0202, "Низкий заряд АКБ", AlarmSeverity::Warning);
    }

    // Additional error parsing can be added based on documentation
}

} // namespace rcms
