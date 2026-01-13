/**
 * @file test_protocol.cpp
 * @brief Integration tests: Fazan19Device + Emulator
 *
 * Tests the actual protocol implementation against the emulator
 * without needing real hardware.
 */

#include <gtest/gtest.h>
#include "emulator/Fazan19Emulator.h"
#include "protocol/Fazan19Registers.h"
#include "comm/CRC16.h"
#include <cmath>

using namespace rcms;
using namespace rcms::test;

class ProtocolTest : public ::testing::Test {
protected:
    Fazan19Emulator emulator{1};

    // Build read request like Fazan19Device would
    std::vector<uint8_t> buildReadRequest(uint8_t addr, uint16_t startReg, uint16_t count) {
        std::vector<uint8_t> request = {
            addr,
            0x03,
            static_cast<uint8_t>(startReg >> 8),
            static_cast<uint8_t>(startReg & 0xFF),
            static_cast<uint8_t>(count >> 8),
            static_cast<uint8_t>(count & 0xFF)
        };
        CRC16::append(request);
        return request;
    }

    // Build write request
    std::vector<uint8_t> buildWriteRequest(uint8_t addr, uint16_t reg, uint16_t value) {
        std::vector<uint8_t> request = {
            addr,
            0x06,
            static_cast<uint8_t>(reg >> 8),
            static_cast<uint8_t>(reg & 0xFF),
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value & 0xFF)
        };
        CRC16::append(request);
        return request;
    }

    // Parse response
    bool parseReadResponse(const std::vector<uint8_t>& response,
                           std::vector<uint16_t>& values) {
        if (response.size() < 5) return false;
        if (response[1] & 0x80) return false;  // Error

        if (!CRC16::verify(response.data(), response.size())) {
            return false;
        }

        uint8_t byteCount = response[2];
        values.clear();
        for (int i = 0; i < byteCount / 2; ++i) {
            uint16_t val = (static_cast<uint16_t>(response[3 + i * 2]) << 8) |
                            response[4 + i * 2];
            values.push_back(val);
        }
        return true;
    }
};

// Test full read cycle with CRC verification
TEST_F(ProtocolTest, ReadAllRegisters) {
    auto request = buildReadRequest(1, 0, fazan19::registers::TOTAL_REGISTERS);
    auto response = emulator.processRequest(request);

    std::vector<uint16_t> values;
    ASSERT_TRUE(parseReadResponse(response, values));
    EXPECT_EQ(values.size(), fazan19::registers::TOTAL_REGISTERS);
}

// Test frequency command cycle
TEST_F(ProtocolTest, SetFrequencyCommand) {
    // Encode frequency like the real driver
    double targetFreq = 127.4;
    double diffHz = (targetFreq - fazan19::frequency::BASE_MHZ) * 1000000.0;
    uint16_t f12 = static_cast<uint16_t>(std::round(diffHz / fazan19::frequency::STEP_HZ));
    uint16_t frrs = f12 & 0x1FFF;

    // Send write command
    auto request = buildWriteRequest(1, fazan19::registers::FRRS, frrs);
    auto response = emulator.processRequest(request);

    // Verify response
    ASSERT_GE(response.size(), 8);
    ASSERT_TRUE(CRC16::verify(response.data(), response.size()));
    EXPECT_EQ(response[1], 0x06);  // Write single response

    // Read back frequency
    auto readReq = buildReadRequest(1, fazan19::registers::FRRS, 1);
    auto readResp = emulator.processRequest(readReq);

    std::vector<uint16_t> values;
    ASSERT_TRUE(parseReadResponse(readResp, values));
    ASSERT_EQ(values.size(), 1);

    // Decode and compare
    uint16_t readF12 = values[0] & 0x1FFF;
    double readFreq = fazan19::frequency::BASE_MHZ +
                      (readF12 * fazan19::frequency::STEP_HZ) / 1000000.0;

    EXPECT_NEAR(readFreq, targetFreq, 0.005);
}

// Test CRC compatibility
TEST_F(ProtocolTest, CRCCompatibility) {
    // Verify our CRC16 class produces correct values
    std::vector<uint8_t> testData = {0x01, 0x03, 0x00, 0x00, 0x00, 0x1C};
    uint16_t crc = CRC16::calculate(testData);

    CRC16::append(testData);

    // Emulator should accept our CRC
    auto response = emulator.processRequest(testData);
    EXPECT_FALSE(response.empty());
    EXPECT_NE(response[1], 0x83);  // Not an error
}

// Test mode register bits
TEST_F(ProtocolTest, ModeRegisterBits) {
    emulator.setRemoteMode(true);
    emulator.setTransmitting(false);
    emulator.setSquelchOpen(true);

    // Use ModTR register (0x02 per РЭ)
    auto request = buildReadRequest(1, fazan19::registers::ModTR, 1);
    auto response = emulator.processRequest(request);

    std::vector<uint16_t> values;
    ASSERT_TRUE(parseReadResponse(response, values));

    uint16_t modtr = values[0];
    EXPECT_TRUE(modtr & fazan19::modes::MR1_REMOTE);
    EXPECT_FALSE(modtr & fazan19::modes::MR1_TX);
    EXPECT_TRUE(modtr & fazan19::modes::MR1_SQUELCH);
}

// Test operating hours register
TEST_F(ProtocolTest, OperatingHoursRegister) {
    // Per РЭ, operating hours is a single 16-bit register (max 65535)
    uint16_t testHours = 12345;
    emulator.setOperatingHours(testHours);

    auto request = buildReadRequest(1, fazan19::registers::CountWork, 1);
    auto response = emulator.processRequest(request);

    std::vector<uint16_t> values;
    ASSERT_TRUE(parseReadResponse(response, values));
    ASSERT_EQ(values.size(), 1);

    EXPECT_EQ(values[0], testHours);
}

// Test error detection
TEST_F(ProtocolTest, ErrorRegisters) {
    emulator.setError(
        fazan19::errors::DV1_POWER_FAIL | fazan19::errors::DV1_TEMP_HIGH,
        fazan19::errors::DV2_BATTERY_LOW,
        0, 0
    );

    auto request = buildReadRequest(1, fazan19::registers::DV1, 4);
    auto response = emulator.processRequest(request);

    std::vector<uint16_t> values;
    ASSERT_TRUE(parseReadResponse(response, values));
    ASSERT_EQ(values.size(), 4);

    EXPECT_TRUE(values[0] & fazan19::errors::DV1_POWER_FAIL);
    EXPECT_TRUE(values[0] & fazan19::errors::DV1_TEMP_HIGH);
    EXPECT_TRUE(values[1] & fazan19::errors::DV2_BATTERY_LOW);
}

// Test all 8.33 kHz channels
TEST_F(ProtocolTest, Channel833Spacing) {
    // Test that adjacent channels differ by exactly 1 in register value
    // Using frequency well within the range (100-149.975 MHz per РЭ)
    double freq1 = 121.500;  // Emergency frequency
    double freq2 = 121.500 + (8.33333 / 1000.0);  // +8.33 kHz

    emulator.setFrequency(freq1);
    uint16_t reg1 = emulator.getRegister(fazan19::registers::FrRS) & 0x1FFF;

    emulator.setFrequency(freq2);
    uint16_t reg2 = emulator.getRegister(fazan19::registers::FrRS) & 0x1FFF;

    EXPECT_EQ(reg2 - reg1, 1);
}

// Test timeout behavior (emulator goes offline)
TEST_F(ProtocolTest, TimeoutBehavior) {
    emulator.setOnline(false);

    auto request = buildReadRequest(1, 0, 1);
    auto response = emulator.processRequest(request);

    EXPECT_TRUE(response.empty());  // Simulates timeout
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
