#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace rcms {

/**
 * @brief CRC-16 Modbus calculation
 *
 * Polynomial: 0x8005 (reflected: 0xA001)
 * Initial value: 0xFFFF
 * Result: Little Endian in Modbus packets
 */
class CRC16 {
public:
    /**
     * @brief Calculate CRC-16 Modbus checksum
     * @param data Pointer to data buffer
     * @param length Length of data
     * @return CRC-16 value
     */
    static uint16_t calculate(const uint8_t* data, size_t length);

    /**
     * @brief Calculate CRC-16 Modbus checksum for vector
     * @param data Vector of bytes
     * @return CRC-16 value
     */
    static uint16_t calculate(const std::vector<uint8_t>& data);

    /**
     * @brief Verify CRC-16 in Modbus packet
     * @param data Pointer to data buffer (including CRC at end)
     * @param length Total length including 2-byte CRC
     * @return true if CRC is valid
     */
    static bool verify(const uint8_t* data, size_t length);

    /**
     * @brief Append CRC-16 to data vector
     * @param data Vector to append CRC to (modified in place)
     */
    static void append(std::vector<uint8_t>& data);

private:
    static constexpr uint16_t POLYNOMIAL = 0xA001;
    static constexpr uint16_t INITIAL_VALUE = 0xFFFF;
};

} // namespace rcms
