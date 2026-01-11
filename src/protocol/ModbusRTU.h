#pragma once

#include <cstdint>
#include <vector>
#include <QSerialPort>

namespace rcms {

/**
 * @brief Modbus RTU protocol implementation
 *
 * Implements Modbus RTU protocol for RS-485 communication
 * with radio devices like Fazan-19
 */
class ModbusRTU {
public:
    // Modbus function codes
    static constexpr uint8_t FUNC_READ_HOLDING = 0x03;
    static constexpr uint8_t FUNC_WRITE_SINGLE = 0x06;
    static constexpr uint8_t FUNC_WRITE_MULTIPLE = 0x10;
    static constexpr uint8_t FUNC_DEVICE_ID = 0x11;

    // Error codes
    static constexpr uint8_t ERR_ILLEGAL_FUNCTION = 0x01;
    static constexpr uint8_t ERR_ILLEGAL_ADDRESS = 0x02;
    static constexpr uint8_t ERR_ILLEGAL_VALUE = 0x03;
    static constexpr uint8_t ERR_DEVICE_FAILURE = 0x04;

    ModbusRTU();
    ~ModbusRTU();

    /**
     * @brief Set serial port for communication
     */
    void setPort(QSerialPort* port) { m_port = port; }

    /**
     * @brief Set response timeout in milliseconds
     */
    void setTimeout(int ms) { m_timeout = ms; }

    /**
     * @brief Read holding registers (function 0x03)
     * @param address Device address
     * @param startReg Starting register address
     * @param count Number of registers to read
     * @param values Output vector for register values
     * @return true on success
     */
    bool readHoldingRegisters(uint8_t address, uint16_t startReg,
                              uint16_t count, std::vector<uint16_t>& values);

    /**
     * @brief Write single register (function 0x06)
     * @param address Device address
     * @param reg Register address
     * @param value Value to write
     * @return true on success
     */
    bool writeSingleRegister(uint8_t address, uint16_t reg, uint16_t value);

    /**
     * @brief Write multiple registers (function 0x10)
     * @param address Device address
     * @param startReg Starting register address
     * @param values Values to write
     * @return true on success
     */
    bool writeMultipleRegisters(uint8_t address, uint16_t startReg,
                                const std::vector<uint16_t>& values);

    /**
     * @brief Get last error message
     */
    const QString& lastError() const { return m_lastError; }

private:
    bool sendRequest(const std::vector<uint8_t>& request);
    bool readResponse(std::vector<uint8_t>& response, int expectedLen);

    QSerialPort* m_port = nullptr;
    int m_timeout = 2000; // Default 2 seconds
    QString m_lastError;
};

} // namespace rcms
