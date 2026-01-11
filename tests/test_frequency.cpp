#include <gtest/gtest.h>
#include <cmath>
#include <cstdint>

/**
 * @brief Test frequency encoding/decoding for Fazan-19
 *
 * Based on reference implementation:
 * https://github.com/drudrum/fazan_19P50
 */

namespace {

constexpr double BASE_MHZ = 100.0;
constexpr double STEP_HZ = 8333.33333;

/**
 * @brief Encode frequency to FrRS register value
 */
uint16_t encodeFrequency(double freqMHz, uint8_t kf = 0) {
    double diffHz = (freqMHz - BASE_MHZ) * 1000000.0;
    uint16_t f12 = static_cast<uint16_t>(std::round(diffHz / STEP_HZ));
    return (static_cast<uint16_t>(kf & 0x03) << 13) | (f12 & 0x1FFF);
}

/**
 * @brief Decode FrRS register value to frequency
 */
double decodeFrequency(uint16_t frrs) {
    uint16_t f12 = frrs & 0x1FFF;
    return BASE_MHZ + (f12 * STEP_HZ) / 1000000.0;
}

/**
 * @brief Extract KF coefficient from FrRS register
 */
uint8_t extractKF(uint16_t frrs) {
    return static_cast<uint8_t>((frrs >> 13) & 0x03);
}

} // anonymous namespace

class FrequencyTest : public ::testing::Test {
protected:
    // Tolerance for floating point comparison (0.001 MHz = 1 kHz)
    static constexpr double TOLERANCE = 0.001;
};

// Test encoding standard aviation frequencies
TEST_F(FrequencyTest, EncodeStandardFrequencies) {
    // 121.5 MHz - emergency frequency
    uint16_t reg = encodeFrequency(121.5);
    EXPECT_EQ(reg & 0x1FFF, 2580); // Expected f12 value

    // 127.4 MHz
    reg = encodeFrequency(127.4);
    EXPECT_GT(reg & 0x1FFF, 0);
}

// Test decoding
TEST_F(FrequencyTest, DecodeFrequencies) {
    // Test round-trip: encode then decode
    double original = 121.5;
    uint16_t encoded = encodeFrequency(original);
    double decoded = decodeFrequency(encoded);

    EXPECT_NEAR(decoded, original, TOLERANCE);
}

// Test frequency range
TEST_F(FrequencyTest, FrequencyRange) {
    // Minimum frequency: 118.0 MHz
    uint16_t regMin = encodeFrequency(118.0);
    double decodedMin = decodeFrequency(regMin);
    EXPECT_NEAR(decodedMin, 118.0, TOLERANCE);

    // Maximum frequency: 136.975 MHz
    uint16_t regMax = encodeFrequency(136.975);
    double decodedMax = decodeFrequency(regMax);
    EXPECT_NEAR(decodedMax, 136.975, TOLERANCE);
}

// Test KF coefficient
TEST_F(FrequencyTest, KFCoefficient) {
    double freq = 121.5;

    // KF = 0
    uint16_t reg0 = encodeFrequency(freq, 0);
    EXPECT_EQ(extractKF(reg0), 0);

    // KF = 1
    uint16_t reg1 = encodeFrequency(freq, 1);
    EXPECT_EQ(extractKF(reg1), 1);

    // KF = 2
    uint16_t reg2 = encodeFrequency(freq, 2);
    EXPECT_EQ(extractKF(reg2), 2);

    // KF = 3
    uint16_t reg3 = encodeFrequency(freq, 3);
    EXPECT_EQ(extractKF(reg3), 3);

    // Frequency part should be the same regardless of KF
    EXPECT_EQ(reg0 & 0x1FFF, reg1 & 0x1FFF);
    EXPECT_EQ(reg0 & 0x1FFF, reg2 & 0x1FFF);
    EXPECT_EQ(reg0 & 0x1FFF, reg3 & 0x1FFF);
}

// Test 8.33 kHz channel spacing
TEST_F(FrequencyTest, ChannelSpacing833) {
    // Adjacent 8.33 kHz channels
    double freq1 = 118.000;
    double freq2 = 118.00833;  // +8.33 kHz

    uint16_t reg1 = encodeFrequency(freq1);
    uint16_t reg2 = encodeFrequency(freq2);

    // Should differ by 1
    EXPECT_EQ((reg2 & 0x1FFF) - (reg1 & 0x1FFF), 1);
}

// Test 25 kHz channel spacing
TEST_F(FrequencyTest, ChannelSpacing25) {
    // Adjacent 25 kHz channels
    double freq1 = 118.000;
    double freq2 = 118.025;  // +25 kHz

    uint16_t reg1 = encodeFrequency(freq1);
    uint16_t reg2 = encodeFrequency(freq2);

    // Should differ by 3 (25 / 8.33 ≈ 3)
    EXPECT_EQ((reg2 & 0x1FFF) - (reg1 & 0x1FFF), 3);
}

// Test specific values from reference implementation
TEST_F(FrequencyTest, ReferenceValues) {
    // From forum discussion: 121.5 MHz -> register value 10772
    // Note: This may include KF bits, need to verify
    uint16_t reg = encodeFrequency(121.5, 0);
    double decoded = decodeFrequency(reg);

    EXPECT_NEAR(decoded, 121.5, TOLERANCE);

    // The f12 value for 121.5 MHz:
    // (121.5 - 100) * 1000000 / 8333.33333 = 2580
    EXPECT_EQ(reg & 0x1FFF, 2580);
}

// Test boundary conditions
TEST_F(FrequencyTest, BoundaryConditions) {
    // Just above base frequency
    double justAbove = 100.00833;
    uint16_t reg = encodeFrequency(justAbove);
    EXPECT_EQ(reg & 0x1FFF, 1);

    // At base frequency
    uint16_t regBase = encodeFrequency(100.0);
    EXPECT_EQ(regBase & 0x1FFF, 0);
}

// Test round-trip for all standard frequencies
TEST_F(FrequencyTest, RoundTripAllStandard) {
    // Test common aviation frequencies
    std::vector<double> frequencies = {
        118.0, 118.025, 118.05, 118.1,
        121.5,  // Emergency
        122.8,  // UNICOM
        127.4,
        131.9,
        133.5,
        136.975
    };

    for (double freq : frequencies) {
        uint16_t encoded = encodeFrequency(freq);
        double decoded = decodeFrequency(encoded);
        EXPECT_NEAR(decoded, freq, TOLERANCE)
            << "Failed for frequency: " << freq << " MHz";
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
