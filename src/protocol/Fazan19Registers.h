#pragma once

#include <cstdint>

namespace rcms {
namespace fazan19 {

/**
 * @brief Modbus register addresses for Fazan-19 radio stations
 *
 * Based on technical documentation and reference implementation:
 * https://github.com/drudrum/fazan_19P50
 */
namespace registers {

// Operating hours counter
constexpr uint16_t CW1 = 0x00;      // Operating hours (high word)
constexpr uint16_t CW2 = 0x01;      // Operating hours (low word)

// Control registers
constexpr uint16_t CNTR = 0x02;     // Request register
constexpr uint16_t MR1 = 0x03;      // Mode register 1
constexpr uint16_t MR2 = 0x04;      // Mode register 2
constexpr uint16_t FRRS = 0x05;     // Frequency + KF coefficient

// ADC channels (analog readings)
constexpr uint16_t AD0 = 0x09;      // ADC channel 0
constexpr uint16_t AD1 = 0x0A;      // ADC channel 1
constexpr uint16_t AD2 = 0x0B;      // ADC channel 2
constexpr uint16_t AD3 = 0x0C;      // ADC channel 3
constexpr uint16_t AD4 = 0x0D;      // ADC channel 4
constexpr uint16_t AD5 = 0x0E;      // ADC channel 5
constexpr uint16_t AD6 = 0x0F;      // ADC channel 6
constexpr uint16_t AD7 = 0x10;      // ADC channel 7

// Error/diagnostic registers
constexpr uint16_t DV1 = 0x16;      // Error register 1
constexpr uint16_t DV2 = 0x17;      // Error register 2
constexpr uint16_t DV3 = 0x18;      // Error register 3
constexpr uint16_t DV4 = 0x19;      // Error register 4

// Total number of registers to read
constexpr uint16_t TOTAL_REGISTERS = 0x1C; // 28 registers

} // namespace registers

/**
 * @brief MR1 register bit definitions
 */
namespace mr1_bits {
constexpr uint16_t SQUELCH_ENABLED = (1 << 7);   // Bit 7: Noise suppressor ON/OFF
// TODO: Add other bits from documentation
}

/**
 * @brief Frequency calculation constants
 */
namespace frequency {
constexpr double BASE_MHZ = 100.0;              // Base frequency offset
constexpr double STEP_HZ = 8333.33333;          // Frequency step in Hz
constexpr double MIN_MHZ = 118.0;               // Minimum frequency
constexpr double MAX_MHZ = 136.975;             // Maximum frequency
constexpr uint16_t F12_MASK = 0x1FFF;           // 13-bit frequency value mask
constexpr uint8_t KF_SHIFT = 13;                // KF coefficient bit position
constexpr uint8_t KF_MASK = 0x03;               // 2-bit KF mask
}

/**
 * @brief KF coefficient values
 */
enum class KFCoefficient : uint8_t {
    NO_OFFSET = 0b00,       // 25 kHz step, no offset
    OFFSET_8_33 = 0b01,     // +8.33 kHz offset
    OFFSET_16_67 = 0b10,    // +16.67 kHz offset
    RESERVED = 0b11         // Reserved/check documentation
};

/**
 * @brief Modbus function codes
 */
namespace modbus_func {
constexpr uint8_t READ_HOLDING_REGISTERS = 0x03;
constexpr uint8_t WRITE_SINGLE_REGISTER = 0x06;
constexpr uint8_t WRITE_MULTIPLE_REGISTERS = 0x10;
constexpr uint8_t READ_DEVICE_ID = 0x11;
}

/**
 * @brief Communication parameters
 */
namespace comm {
constexpr int DEFAULT_BAUD_RATE = 9600;
constexpr int DATA_BITS = 8;
constexpr int STOP_BITS = 1;
constexpr char PARITY = 'N';
constexpr int TIMEOUT_MS = 2000;
constexpr int RETRY_COUNT = 3;
}

/**
 * @brief Error codes from DV1-DV4 registers
 */
namespace error_codes {
constexpr uint16_t NO_ERROR = 0x0000;
// TODO: Add specific error codes from documentation
}

} // namespace fazan19
} // namespace rcms
