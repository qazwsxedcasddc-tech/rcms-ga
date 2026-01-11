#include <gtest/gtest.h>
#include "comm/CRC16.h"

using namespace rcms;

/**
 * @brief Test CRC-16 Modbus calculation
 *
 * Test vectors from known Modbus packets
 */
class CRC16Test : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test basic CRC calculation
TEST_F(CRC16Test, BasicCalculation) {
    // Example: Modbus request to read holding registers
    // Address=1, Function=03, Start=0000, Count=001C
    std::vector<uint8_t> data = {0x01, 0x03, 0x00, 0x00, 0x00, 0x1C};

    uint16_t crc = CRC16::calculate(data);

    // CRC should be non-zero
    EXPECT_NE(crc, 0);
}

// Test known Modbus packet CRC
TEST_F(CRC16Test, KnownPacket) {
    // Standard Modbus query: Read 1 register from address 0
    // Slave=01, Func=03, Addr=0000, Count=0001
    std::vector<uint8_t> data = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};

    uint16_t crc = CRC16::calculate(data);

    // Known CRC for this packet: 0x840A
    EXPECT_EQ(crc, 0x840A);
}

// Test CRC verification
TEST_F(CRC16Test, Verification) {
    // Packet with valid CRC appended
    std::vector<uint8_t> packet = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};

    EXPECT_TRUE(CRC16::verify(packet.data(), packet.size()));
}

// Test CRC verification failure
TEST_F(CRC16Test, VerificationFailure) {
    // Packet with invalid CRC
    std::vector<uint8_t> packet = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};

    EXPECT_FALSE(CRC16::verify(packet.data(), packet.size()));
}

// Test CRC append
TEST_F(CRC16Test, AppendCRC) {
    std::vector<uint8_t> data = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};
    size_t originalSize = data.size();

    CRC16::append(data);

    // Size should increase by 2
    EXPECT_EQ(data.size(), originalSize + 2);

    // Packet should now verify
    EXPECT_TRUE(CRC16::verify(data.data(), data.size()));
}

// Test empty data
TEST_F(CRC16Test, EmptyData) {
    std::vector<uint8_t> empty;

    uint16_t crc = CRC16::calculate(empty);

    // CRC of empty data is initial value
    EXPECT_EQ(crc, 0xFFFF);
}

// Test single byte
TEST_F(CRC16Test, SingleByte) {
    std::vector<uint8_t> data = {0x00};

    uint16_t crc = CRC16::calculate(data);

    // Known CRC for single 0x00 byte
    EXPECT_EQ(crc, 0x40BF);
}

// Test Fazan-19 specific: Read 28 registers command
TEST_F(CRC16Test, Fazan19ReadRegisters) {
    // Command to read 28 registers from Fazan-19
    // Address=1, Function=03, Start=0000, Count=001C
    std::vector<uint8_t> cmd = {0x01, 0x03, 0x00, 0x00, 0x00, 0x1C};

    CRC16::append(cmd);

    // Verify the complete packet
    EXPECT_EQ(cmd.size(), 8);
    EXPECT_TRUE(CRC16::verify(cmd.data(), cmd.size()));
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
