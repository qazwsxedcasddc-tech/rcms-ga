#pragma once

#include <QString>
#include <QVector>
#include <QDateTime>
#include <cstdint>

namespace rcms {
/**
 * @brief Alarm severity levels
 */
enum class AlarmSeverity {
    Info,
    Warning,
    Error,
    Critical
};


/**
 * @brief Device status structure
 */
struct DeviceStatus {
    bool online = false;                    // Communication OK
    double frequencyMHz = 0.0;              // Current frequency in MHz
    bool isTransmitting = false;            // PTT active
    bool isReceiving = false;               // Squelch open
    bool squelchEnabled = false;            // Noise suppressor ON
    int squelchLevel = 0;                   // Squelch level (0-15)
    int signalLevel = 0;                    // Receiver signal level (ADC)
    double voltage24V = 0.0;                // Power supply voltage
    double batteryVoltage = 0.0;            // Battery voltage
    double temperature = 0.0;               // Temperature in Celsius
    uint32_t operatingHours = 0;            // Total operating hours
    QString mode;                           // "МУ" (local) or "ДУ" (remote)
    QString workMode;                       // "ТЛФ" (phone) or "ДАН" (data)
    QString lineType;                       // "2-х" or "4-х" wire
    QDateTime lastUpdate;                   // Last successful read time
    QVector<uint16_t> errorCodes;           // Active error codes
};

/**
 * @brief Alarm information structure
 */
struct AlarmInfo {
    QDateTime timestamp;
    uint8_t deviceAddress;
    QString deviceName;
    uint16_t code;
    AlarmSeverity severity = AlarmSeverity::Info;       // "INFO", "WARN", "ERROR", "CRITICAL"
    QString message;
    bool acknowledged = false;
};

/**
 * @brief Abstract interface for radio devices
 *
 * All radio device drivers must implement this interface.
 * This allows adding support for new devices without changing core logic.
 */
class IRadioDevice {
public:
    virtual ~IRadioDevice() = default;

    // ========== Identification ==========

    /**
     * @brief Get device type name
     * @return Device type string (e.g., "Фазан-19 Р5")
     */
    virtual QString deviceType() const = 0;

    /**
     * @brief Get device unique identifier
     * @return Device serial number or ID
     */
    virtual QString deviceId() const = 0;

    /**
     * @brief Get Modbus slave address
     * @return Address (1-247)
     */
    virtual uint8_t modbusAddress() const = 0;

    /**
     * @brief Set Modbus slave address
     * @param address New address (1-247)
     */
    virtual void setModbusAddress(uint8_t address) = 0;

    // ========== Connection ==========

    /**
     * @brief Open connection to device
     * @param portName Serial port name (e.g., "/dev/ttyUSB0")
     * @param baudRate Baud rate (default 9600)
     * @return true if successful
     */
    virtual bool open(const QString& portName, int baudRate = 9600) = 0;

    /**
     * @brief Close connection
     */
    virtual void close() = 0;

    /**
     * @brief Check if connection is open
     * @return true if connected
     */
    virtual bool isOpen() const = 0;

    // ========== Status Reading ==========

    /**
     * @brief Read device status
     * @param status Output status structure
     * @return true if successful
     */
    virtual bool readStatus(DeviceStatus& status) = 0;

    /**
     * @brief Read active alarms
     * @param alarms Output vector of alarms
     * @return true if successful
     */
    virtual bool readAlarms(QVector<AlarmInfo>& alarms) = 0;

    // ========== Control ==========

    /**
     * @brief Set operating frequency
     * @param freqMHz Frequency in MHz (118.0 - 136.975)
     * @return true if successful
     */
    virtual bool setFrequency(double freqMHz) = 0;

    /**
     * @brief Get current frequency
     * @param freqMHz Output frequency in MHz
     * @return true if successful
     */
    virtual bool getFrequency(double& freqMHz) = 0;

    /**
     * @brief Set noise suppressor (squelch)
     * @param enabled Enable/disable
     * @param level Squelch level (0-15)
     * @return true if successful
     */
    virtual bool setSquelch(bool enabled, int level = 5) = 0;

    /**
     * @brief Set PTT (Push-To-Talk)
     * @param enabled Enable transmission
     * @return true if successful
     */
    virtual bool setPTT(bool enabled) = 0;

    // ========== Diagnostics ==========

    /**
     * @brief Run self-test
     * @return true if test passed
     */
    virtual bool runSelfTest() = 0;

    /**
     * @brief Get last error message
     * @return Error description
     */
    virtual QString lastError() const = 0;
};

} // namespace rcms
