#pragma once

#include <cstdint>

namespace rcms {
namespace fazan19 {

/**
 * @brief Modbus register addresses for Fazan-19 radio stations
 *
 * Based on official operating manual (РЭ) ЦВИЯ.464514.008 РЭ
 * Register map per Table 2.12 (Карта регистров внутреннего управления)
 */
namespace registers {

// Operating hours counter (Наработка)
constexpr uint16_t CountWork = 0x00;    // Наработка - часы работы

// Request counter (Запросы)
constexpr uint16_t Cntr = 0x01;         // Счётчик запросов

// Mode/Control register (Режим передатчика/приёмника)
constexpr uint16_t ModTR = 0x02;        // Режим: TX/RX, мощность, управление

// Frequency register (Частота)
constexpr uint16_t FrRS = 0x03;         // Частота: f12...f0 + kf (коэффициент)

// Power register (Мощность)
constexpr uint16_t PKm = 0x04;          // Задаваемая мощность

// Reserved registers 0x05-0x0F

// ADC channels (analog readings) - per РЭ at 0x10-0x17
constexpr uint16_t AD0 = 0x10;          // АЦП канал 0
constexpr uint16_t AD1 = 0x11;          // АЦП канал 1
constexpr uint16_t AD2 = 0x12;          // АЦП канал 2
constexpr uint16_t AD3 = 0x13;          // АЦП канал 3
constexpr uint16_t AD4 = 0x14;          // АЦП канал 4
constexpr uint16_t AD5 = 0x15;          // АЦП канал 5
constexpr uint16_t AD6 = 0x16;          // АЦП канал 6
constexpr uint16_t AD7 = 0x17;          // АЦП канал 7

// Diagnostic register (DiagVUU) - 8 bytes of error flags
constexpr uint16_t DiagVUU = 0x18;      // Диагностика ВУУ (8 байт)

// Legacy aliases for backwards compatibility
constexpr uint16_t CW1 = CountWork;
constexpr uint16_t CNTR = Cntr;
constexpr uint16_t MR1 = ModTR;
constexpr uint16_t FRRS = FrRS;
constexpr uint16_t DV1 = DiagVUU;

// Total number of registers to read for full status
constexpr uint16_t TOTAL_REGISTERS = 0x1C; // 28 registers

} // namespace registers

/**
 * @brief ModTR register bit definitions (per РЭ)
 *
 * ModTR format:
 * Bit 0: TX/RX mode (0=RX, 1=TX)
 * Bits 1-2: Power level
 * Other bits per documentation
 */
namespace modes {
constexpr uint16_t ModTR_TX = 0x0001;            // Bit 0: TX mode active
constexpr uint16_t ModTR_RX = 0x0000;            // Bit 0: RX mode (default)
constexpr uint16_t ModTR_POWER_MASK = 0x0006;    // Bits 1-2: Power level mask
constexpr uint16_t ModTR_POWER_SHIFT = 1;        // Power level bit position

// Legacy aliases
constexpr uint16_t MR1_TX = ModTR_TX;
constexpr uint16_t MR1_SQUELCH = 0x0080;         // Bit 7: Squelch enabled
constexpr uint16_t MR1_REMOTE = 0x0100;          // Bit 8: Remote mode
constexpr uint16_t MR1_DATA_MODE = 0x0200;       // Bit 9: Data mode (vs voice)
constexpr uint16_t MR1_4WIRE = 0x0400;           // Bit 10: 4-wire mode
}

/**
 * @brief DiagVUU (Diagnostic) register bit definitions
 *
 * Per РЭ Table 2.12: DiagVUU contains 8 bytes of diagnostic information
 * Each byte contains specific error/status flags for different subsystems
 */
namespace diag {

// Byte 0 - General status
constexpr uint8_t DIAG0_POWER_OK = 0x01;         // Питание в норме
constexpr uint8_t DIAG0_PLL_LOCK = 0x02;         // ФАПЧ захвачена
constexpr uint8_t DIAG0_PA_OK = 0x04;            // УМ исправен
constexpr uint8_t DIAG0_ANTENNA_OK = 0x08;       // Антенна подключена

// Byte 1 - Temperature and power
constexpr uint8_t DIAG1_TEMP_NORMAL = 0x01;      // Температура в норме
constexpr uint8_t DIAG1_VSWR_OK = 0x02;          // КСВ в норме
constexpr uint8_t DIAG1_FWD_POWER_OK = 0x04;     // Прямая мощность в норме

// Byte 2 - Receiver status
constexpr uint8_t DIAG2_RX_OK = 0x01;            // Приёмник исправен
constexpr uint8_t DIAG2_SQUELCH_OPEN = 0x02;     // Шумоподавитель открыт
constexpr uint8_t DIAG2_SIGNAL_DETECT = 0x04;   // Сигнал обнаружен

// Byte 3 - Transmitter status
constexpr uint8_t DIAG3_TX_OK = 0x01;            // Передатчик исправен
constexpr uint8_t DIAG3_TX_ACTIVE = 0x02;        // Передача активна
constexpr uint8_t DIAG3_MODULATION_OK = 0x04;    // Модуляция в норме

// Bytes 4-7 - Reserved/extended diagnostics
}

/**
 * @brief Error bit definitions (derived from DiagVUU)
 */
namespace errors {
// Critical errors (inverted from DIAG - 0 means error)
constexpr uint16_t ERR_POWER_FAIL = 0x0001;     // Power supply failure
constexpr uint16_t ERR_PLL_UNLOCK = 0x0002;     // PLL unlocked
constexpr uint16_t ERR_PA_FAIL = 0x0004;        // Power amplifier failure
constexpr uint16_t ERR_VSWR_HIGH = 0x0008;      // VSWR too high
constexpr uint16_t ERR_TEMP_HIGH = 0x0010;      // Overtemperature
constexpr uint16_t ERR_ANTENNA = 0x0020;        // Antenna problem

// Secondary errors
constexpr uint16_t ERR_RX_FAIL = 0x0100;        // Receiver failure
constexpr uint16_t ERR_TX_FAIL = 0x0200;        // Transmitter failure

// Legacy aliases
constexpr uint16_t DV1_POWER_FAIL = ERR_POWER_FAIL;
constexpr uint16_t DV1_PLL_UNLOCK = ERR_PLL_UNLOCK;
constexpr uint16_t DV1_PA_FAIL = ERR_PA_FAIL;
constexpr uint16_t DV1_VSWR_HIGH = ERR_VSWR_HIGH;
constexpr uint16_t DV1_TEMP_HIGH = ERR_TEMP_HIGH;
constexpr uint16_t DV2_RX_FAIL = ERR_RX_FAIL;
constexpr uint16_t DV2_BATTERY_LOW = 0x0002;
}

/**
 * @brief Frequency calculation constants (per РЭ)
 *
 * FrRS register format:
 * - Bits 0-12 (f12...f0): Frequency offset from 100 MHz in 8.33 kHz steps
 * - Bits 13-14 (kf): Frequency coefficient
 *   - 00: 8.33 kHz step mode
 *   - 01: 25 kHz step mode
 *   - 10: Offset down mode
 *   - 11: Offset up mode
 *
 * Frequency range per РЭ: 100.000 - 149.975 MHz
 */
namespace frequency {
constexpr double BASE_MHZ = 100.0;              // Base frequency: 100 MHz
constexpr double STEP_KHZ = 8.33333;            // Minimum step: 8.33 kHz
constexpr double STEP_HZ = 8333.33333;          // Step in Hz
constexpr double MIN_MHZ = 100.0;               // Minimum frequency per РЭ
constexpr double MAX_MHZ = 149.975;             // Maximum frequency per РЭ
constexpr uint16_t F_MASK = 0x1FFF;             // 13-bit frequency value mask (f12...f0)
constexpr uint8_t KF_SHIFT = 13;                // KF coefficient bit position
constexpr uint8_t KF_MASK = 0x03;               // 2-bit KF mask

// Legacy alias
constexpr uint16_t F12_MASK = F_MASK;
}

/**
 * @brief KF coefficient values (per РЭ)
 *
 * КФ (коэффициент частоты):
 * 00 - шаг 8.33 кГц
 * 01 - шаг 25 кГц
 * 10 - смещение вниз
 * 11 - смещение вверх
 */
enum class KFCoefficient : uint8_t {
    Step_8_33_kHz = 0b00,   // 8.33 kHz step mode
    Step_25_kHz = 0b01,     // 25 kHz step mode
    Offset_Down = 0b10,     // Offset down
    Offset_Up = 0b11,       // Offset up

    // Legacy aliases
    NO_OFFSET = Step_8_33_kHz,
    OFFSET_8_33 = Step_25_kHz,
    OFFSET_16_67 = Offset_Down,
    RESERVED = Offset_Up
};

/**
 * @brief Timing constants
 */
namespace timing {
constexpr int RESPONSE_TIMEOUT_MS = 2000;       // Response timeout
constexpr int RETRY_COUNT = 3;                  // Retry count
constexpr int POLL_INTERVAL_MS = 1000;          // Default poll interval
}

/**
 * @brief Modbus function codes (per Modbus RTU standard)
 */
namespace modbus_func {
constexpr uint8_t READ_HOLDING_REGISTERS = 0x03;
constexpr uint8_t WRITE_SINGLE_REGISTER = 0x06;
constexpr uint8_t WRITE_MULTIPLE_REGISTERS = 0x10;
constexpr uint8_t READ_DEVICE_ID = 0x11;
}

/**
 * @brief Communication parameters (per РЭ default settings)
 */
namespace comm {
constexpr int DEFAULT_BAUD_RATE = 9600;
constexpr int DATA_BITS = 8;
constexpr int STOP_BITS = 1;
constexpr char PARITY = 'N';
}

/**
 * @brief Power levels
 */
namespace power {
constexpr uint8_t POWER_MIN = 0;                // Minimum power
constexpr uint8_t POWER_MAX = 3;                // Maximum power (4 levels: 0-3)
}

} // namespace fazan19
} // namespace rcms
