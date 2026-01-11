#include "CRC16.h"

namespace rcms {

uint16_t CRC16::calculate(const uint8_t* data, size_t length) {
    uint16_t crc = INITIAL_VALUE;

    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ POLYNOMIAL;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

uint16_t CRC16::calculate(const std::vector<uint8_t>& data) {
    return calculate(data.data(), data.size());
}

bool CRC16::verify(const uint8_t* data, size_t length) {
    if (length < 3) {
        return false; // Minimum: 1 byte data + 2 bytes CRC
    }

    // Calculate CRC of data without the last 2 bytes
    uint16_t calculated = calculate(data, length - 2);

    // Extract CRC from packet (Little Endian)
    uint16_t received = static_cast<uint16_t>(data[length - 2]) |
                        (static_cast<uint16_t>(data[length - 1]) << 8);

    return calculated == received;
}

void CRC16::append(std::vector<uint8_t>& data) {
    uint16_t crc = calculate(data);

    // Append in Little Endian order
    data.push_back(static_cast<uint8_t>(crc & 0xFF));        // Low byte
    data.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF)); // High byte
}

} // namespace rcms
