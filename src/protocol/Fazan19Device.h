#pragma once

#include "IRadioDevice.h"
#include "ModbusRTU.h"
#include "Fazan19Registers.h"
#include <QSerialPort>
#include <memory>

namespace rcms {

/**
 * @brief Fazan-19 P5 radio device implementation
 *
 * Implements IRadioDevice interface for Fazan-19 P5 radio transmitter/receiver
 * using Modbus RTU protocol over RS-485
 */
class Fazan19Device : public IRadioDevice {
public:
    Fazan19Device(uint8_t address = 1);
    ~Fazan19Device() override;

    // IRadioDevice interface implementation
    QString deviceType() const override { return "Фазан-19 П5"; }
    QString deviceId() const override { return m_deviceId; }
    uint8_t modbusAddress() const override { return m_address; }
    void setModbusAddress(uint8_t address) override { m_address = address; }

    bool open(const QString& portName, int baudRate = 9600) override;
    void close() override;
    bool isOpen() const override;

    bool readStatus(DeviceStatus& status) override;
    bool readAlarms(QVector<AlarmInfo>& alarms) override;

    bool setFrequency(double freqMHz) override;
    bool getFrequency(double& freqMHz) override;
    bool setSquelch(bool enabled, int level = 5) override;
    bool setPTT(bool enabled) override;

    bool runSelfTest() override;
    QString lastError() const override { return m_lastError; }

    /**
     * @brief Read all registers from device
     * @param registers Output array for register values
     * @return true on success
     */
    bool readAllRegisters(uint16_t* registers);

    /**
     * @brief Get operating hours
     */
    uint32_t getOperatingHours() const { return m_operatingHours; }

    /**
     * @brief Get current frequency in MHz
     */
    double getCurrentFrequency() const { return m_currentFrequency; }

private:
    // Frequency encoding/decoding
    static uint16_t encodeFrequency(double freqMHz, uint8_t kf = 0);
    static double decodeFrequency(uint16_t frrs);
    static uint8_t extractKF(uint16_t frrs);

    // Error code parsing
    void parseErrors(uint16_t dv1, uint16_t dv2, uint16_t dv3, uint16_t dv4,
                     QVector<AlarmInfo>& alarms);

    // Parse mode registers
    void parseModeRegister(uint16_t mr1, DeviceStatus& status);

    uint8_t m_address;
    QString m_deviceId;
    QString m_lastError;
    std::unique_ptr<QSerialPort> m_port;
    std::unique_ptr<ModbusRTU> m_modbus;

    // Cached state
    double m_currentFrequency = 0.0;
    uint32_t m_operatingHours = 0;
    bool m_squelchEnabled = false;
    int m_squelchLevel = 5;
};

} // namespace rcms
