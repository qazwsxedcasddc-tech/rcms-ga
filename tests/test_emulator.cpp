/**
 * @file test_emulator.cpp
 * @brief Unit tests for Fazan-19 Modbus emulator
 *
 * Tests the emulator itself to ensure it correctly simulates
 * the real device behavior.
 */

#include <gtest/gtest.h>
#include "emulator/Fazan19Emulator.h"
#include <cmath>

using namespace rcms::test;

class EmulatorTest : public ::testing::Test {
protected:
    Fazan19Emulator emulator{1};  // Address 1

    // Helper to build a read request
    std::vector<uint8_t> makeReadRequest(uint8_t addr, uint16_t startReg, uint16_t count) {
        std::vector<uint8_t> request = {
            addr,
            0x03,  // Read holding registers
            static_cast<uint8_t>(startReg >> 8),
            static_cast<uint8_t>(startReg & 0xFF),
            static_cast<uint8_t>(count >> 8),
            static_cast<uint8_t>(count & 0xFF)
        };
        appendCRC(request);
        return request;
    }

    // Helper to build a write single request
    std::vector<uint8_t> makeWriteSingleRequest(uint8_t addr, uint16_t reg, uint16_t value) {
        std::vector<uint8_t> request = {
            addr,
            0x06,  // Write single register
            static_cast<uint8_t>(reg >> 8),
            static_cast<uint8_t>(reg & 0xFF),
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value & 0xFF)
        };
        appendCRC(request);
        return request;
    }

    // CRC helper (same as emulator)
    static void appendCRC(std::vector<uint8_t>& data) {
        uint16_t crc = 0xFFFF;
        for (uint8_t byte : data) {
            crc ^= byte;
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        data.push_back(static_cast<uint8_t>(crc & 0xFF));
        data.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
    }
};

// Test reading registers
TEST_F(EmulatorTest, ReadHoldingRegisters) {
    auto request = makeReadRequest(1, 0x00, 2);  // Read 2 registers from 0x00
    auto response = emulator.processRequest(request);

    ASSERT_GE(response.size(), 7);  // addr + func + count + 4 bytes + CRC
    EXPECT_EQ(response[0], 1);      // Address
    EXPECT_EQ(response[1], 0x03);   // Function code
    EXPECT_EQ(response[2], 4);      // Byte count (2 registers * 2 bytes)
}

// Test wrong address gets no response
TEST_F(EmulatorTest, WrongAddressNoResponse) {
    auto request = makeReadRequest(2, 0x00, 1);  // Wrong address
    auto response = emulator.processRequest(request);

    EXPECT_TRUE(response.empty());  // No response for wrong address
}

// Test frequency encoding/decoding
TEST_F(EmulatorTest, FrequencyRoundTrip) {
    double testFreq = 121.5;  // Emergency frequency
    emulator.setFrequency(testFreq);

    double readFreq = emulator.getFrequency();
    EXPECT_NEAR(readFreq, testFreq, 0.001);  // Within 1 kHz tolerance
}

// Test reading frequency register
TEST_F(EmulatorTest, ReadFrequencyRegister) {
    emulator.setFrequency(118.0);

    auto request = makeReadRequest(1, 0x05, 1);  // FrRS register
    auto response = emulator.processRequest(request);

    ASSERT_GE(response.size(), 7);
    EXPECT_EQ(response[1], 0x03);  // Success

    // Extract value
    uint16_t frrs = (static_cast<uint16_t>(response[3]) << 8) | response[4];
    uint16_t f12 = frrs & 0x1FFF;

    // 118.0 MHz -> f12 = (118 - 100) * 1e6 / 8333.33 = 2160
    EXPECT_NEAR(f12, 2160, 1);
}

// Test writing frequency register
TEST_F(EmulatorTest, WriteFrequencyRegister) {
    // Encode 121.5 MHz
    // f12 = (121.5 - 100) * 1e6 / 8333.33 = 2580
    uint16_t frrs = 2580;

    auto request = makeWriteSingleRequest(1, 0x05, frrs);
    auto response = emulator.processRequest(request);

    ASSERT_GE(response.size(), 8);
    EXPECT_EQ(response[1], 0x06);  // Echo function code

    // Verify frequency was set
    double freq = emulator.getFrequency();
    EXPECT_NEAR(freq, 121.5, 0.001);
}

// Test operating hours
TEST_F(EmulatorTest, OperatingHours) {
    emulator.setOperatingHours(12345);

    uint16_t cw1 = emulator.getRegister(0x00);
    uint16_t cw2 = emulator.getRegister(0x01);

    uint32_t hours = (static_cast<uint32_t>(cw1) << 16) | cw2;
    EXPECT_EQ(hours, 12345);
}

// Test error registers
TEST_F(EmulatorTest, ErrorRegisters) {
    emulator.setError(0x0001, 0x0002, 0x0004, 0x0008);

    EXPECT_EQ(emulator.getRegister(0x18), 0x0001);  // DV1
    EXPECT_EQ(emulator.getRegister(0x19), 0x0002);  // DV2
    EXPECT_EQ(emulator.getRegister(0x1A), 0x0004);  // DV3
    EXPECT_EQ(emulator.getRegister(0x1B), 0x0008);  // DV4

    emulator.clearErrors();
    EXPECT_EQ(emulator.getRegister(0x18), 0);
}

// Test mode register bits
TEST_F(EmulatorTest, ModeRegisterBits) {
    emulator.setRemoteMode(true);
    emulator.setTransmitting(true);

    uint16_t mr1 = emulator.getRegister(0x03);
    EXPECT_TRUE(mr1 & 0x0100);  // Remote mode bit
    EXPECT_TRUE(mr1 & 0x0001);  // TX bit

    emulator.setTransmitting(false);
    mr1 = emulator.getRegister(0x03);
    EXPECT_FALSE(mr1 & 0x0001);  // TX bit cleared
}

// Test offline device
TEST_F(EmulatorTest, OfflineDevice) {
    emulator.setOnline(false);

    auto request = makeReadRequest(1, 0x00, 1);
    auto response = emulator.processRequest(request);

    EXPECT_TRUE(response.empty());  // No response when offline
}

// Test invalid register address
TEST_F(EmulatorTest, InvalidRegisterAddress) {
    auto request = makeReadRequest(1, 0xFF, 1);  // Invalid address
    auto response = emulator.processRequest(request);

    ASSERT_GE(response.size(), 5);
    EXPECT_EQ(response[1], 0x83);  // Error response (0x03 | 0x80)
    EXPECT_EQ(response[2], 0x02);  // Illegal address error
}

// Test bad CRC
TEST_F(EmulatorTest, BadCRC) {
    auto request = makeReadRequest(1, 0x00, 1);
    request[request.size() - 1] ^= 0xFF;  // Corrupt CRC

    auto response = emulator.processRequest(request);
    EXPECT_TRUE(response.empty());  // No response for bad CRC
}

// Test all aviation frequencies
TEST_F(EmulatorTest, AviationFrequencyRange) {
    std::vector<double> frequencies = {
        118.000, 118.025, 118.050,  // Start of band
        121.500,                     // Emergency
        122.800,                     // UNICOM
        127.400,
        131.900,
        133.500,
        136.975                      // End of band
    };

    for (double freq : frequencies) {
        emulator.setFrequency(freq);
        double readBack = emulator.getFrequency();
        EXPECT_NEAR(readBack, freq, 0.005)
            << "Failed for frequency: " << freq << " MHz";
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
