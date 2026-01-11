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
constexpr uint16_t AD0 = 0x0C;      // ADC channel 0 (voltage)
constexpr uint16_t AD1 = 0x0D;      // ADC channel 1 (temperature)
constexpr uint16_t AD2 = 0x0E;      // ADC channel 2 (signal level)
constexpr uint16_t AD3 = 0x0F;      // ADC channel 3
constexpr uint16_t AD4 = 0x10;      // ADC channel 4
constexpr uint16_t AD5 = 0x11;      // ADC channel 5
constexpr uint16_t AD6 = 0x12;      // ADC channel 6
constexpr uint16_t AD7 = 0x13;      // ADC channel 7

// Error/diagnostic registers
constexpr uint16_t DV1 = 0x18;      // Error register 1
constexpr uint16_t DV2 = 0x19;      // Error register 2
constexpr uint16_t DV3 = 0x1A;      // Error register 3
constexpr uint16_t DV4 = 0x1B;      // Error register 4

// Total number of registers to read
constexpr uint16_t TOTAL_REGISTERS = 0x1C; // 28 registers

} // namespace registers

/**
 * @brief MR1 register bit definitions
 */
namespace modes {
constexpr uint16_t MR1_TX = 0x0001;              // Bit 0: Transmitting
constexpr uint16_t MR1_SQUELCH = 0x0080;         // Bit 7: Squelch enabled
constexpr uint16_t MR1_REMOTE = 0x0100;          // Bit 8: Remote mode
constexpr uint16_t MR1_DATA_MODE = 0x0200;       // Bit 9: Data mode (vs voice)
constexpr uint16_t MR1_4WIRE = 0x0400;           // Bit 10: 4-wire mode
}

/**
 * @brief Error bit definitions for DV1-DV4 registers
 */
namespace errors {
// DV1 - Critical errors
constexpr uint16_t DV1_POWER_FAIL = 0x0001;     // Power supply failure
constexpr uint16_t DV1_PLL_UNLOCK = 0x0002;     // PLL unlocked
constexpr uint16_t DV1_PA_FAIL = 0x0004;        // Power amplifier failure
constexpr uint16_t DV1_VSWR_HIGH = 0x0008;      // VSWR too high
constexpr uint16_t DV1_TEMP_HIGH = 0x0010;      // Overtemperature

// DV2 - Secondary errors
constexpr uint16_t DV2_RX_FAIL = 0x0001;        // Receiver failure
constexpr uint16_t DV2_BATTERY_LOW = 0x0002;    // Low battery
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
 * @brief Timing constants
 */
namespace timing {
constexpr int RESPONSE_TIMEOUT_MS = 2000;       // Response timeout
constexpr int RETRY_COUNT = 3;                  // Retry count
constexpr int POLL_INTERVAL_MS = 1000;          // Default poll interval
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
}

} // namespace fazan19
} // namespace rcms
