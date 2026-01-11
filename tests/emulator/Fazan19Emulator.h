#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <functional>

namespace rcms {
namespace test {

/**
 * @brief Fazan-19 Modbus RTU emulator for testing
 *
 * Emulates a Fazan-19 P5 radio device responding to Modbus RTU commands.
 * Can be used with virtual serial ports (socat) or for unit testing.
 */
class Fazan19Emulator {
public:
    // Register count matching real device
    static constexpr size_t REGISTER_COUNT = 28;

    // Modbus function codes
    static constexpr uint8_t FUNC_READ_HOLDING = 0x03;
    static constexpr uint8_t FUNC_WRITE_SINGLE = 0x06;
    static constexpr uint8_t FUNC_WRITE_MULTIPLE = 0x10;
    static constexpr uint8_t FUNC_DEVICE_ID = 0x11;

    Fazan19Emulator(uint8_t address = 1);

    /**
     * @brief Set device Modbus address
     */
    void setAddress(uint8_t address) { m_address = address; }
    uint8_t address() const { return m_address; }

    /**
     * @brief Process incoming Modbus request
     * @param request Raw request bytes
     * @return Response bytes (empty if request not for us)
     */
    std::vector<uint8_t> processRequest(const std::vector<uint8_t>& request);

    /**
     * @brief Direct access to registers for test setup
     */
    uint16_t getRegister(uint16_t addr) const;
    void setRegister(uint16_t addr, uint16_t value);

    /**
     * @brief Set frequency (updates FrRS register)
     */
    void setFrequency(double freqMHz);
    double getFrequency() const;

    /**
     * @brief Set operating hours
     */
    void setOperatingHours(uint32_t hours);

    /**
     * @brief Set error flags
     */
    void setError(uint16_t dv1, uint16_t dv2 = 0, uint16_t dv3 = 0, uint16_t dv4 = 0);
    void clearErrors();

    /**
     * @brief Set mode register bits
     */
    void setRemoteMode(bool remote);
    void setTransmitting(bool tx);
    void setSquelchOpen(bool open);

    /**
     * @brief Simulate device going offline (won't respond)
     */
    void setOnline(bool online) { m_online = online; }
    bool isOnline() const { return m_online; }

    /**
     * @brief Set response delay (for timeout testing)
     */
    void setResponseDelayMs(int ms) { m_responseDelayMs = ms; }

    /**
     * @brief Callback for request logging
     */
    using RequestCallback = std::function<void(const std::vector<uint8_t>&, const std::vector<uint8_t>&)>;
    void setRequestCallback(RequestCallback cb) { m_requestCallback = cb; }

private:
    std::vector<uint8_t> handleReadHolding(const std::vector<uint8_t>& request);
    std::vector<uint8_t> handleWriteSingle(const std::vector<uint8_t>& request);
    std::vector<uint8_t> handleWriteMultiple(const std::vector<uint8_t>& request);
    std::vector<uint8_t> handleDeviceId(const std::vector<uint8_t>& request);
    std::vector<uint8_t> makeErrorResponse(uint8_t funcCode, uint8_t errorCode);

    static uint16_t calculateCRC(const uint8_t* data, size_t length);
    static void appendCRC(std::vector<uint8_t>& data);
    static bool verifyCRC(const std::vector<uint8_t>& data);

    // Frequency encoding
    static uint16_t encodeFrequency(double freqMHz, uint8_t kf = 0);
    static double decodeFrequency(uint16_t frrs);

    uint8_t m_address;
    bool m_online = true;
    int m_responseDelayMs = 0;
    std::array<uint16_t, REGISTER_COUNT> m_registers{};
    RequestCallback m_requestCallback;
};

} // namespace test
} // namespace rcms
