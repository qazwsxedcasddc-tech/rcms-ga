#pragma once

#include <QString>
#include <cmath>

namespace rcms {

/**
 * @brief Frequency correction policy
 */
enum class FrequencyCorrectionPolicy {
    Clamp,      // Silently clamp to valid range
    WarnClamp,  // Clamp with warning notification
    WarnBlock   // Show error and block the command
};

/**
 * @brief Channel spacing for frequency rounding
 */
enum class ChannelSpacing {
    Spacing25kHz,   // 25 kHz grid (legacy)
    Spacing8_33kHz  // 8.33 kHz grid (ICAO standard)
};

/**
 * @brief Frequency validation and correction result
 */
struct FrequencyValidationResult {
    bool valid = false;
    double originalFreqMHz = 0.0;
    double correctedFreqMHz = 0.0;
    bool wasClamped = false;
    bool wasRounded = false;
    QString message;
};

/**
 * @brief Frequency policy handler
 *
 * Validates and corrects frequencies according to Fazan-19 specs
 */
class FrequencyPolicy {
public:
    // Fazan-19 frequency range (per РЭ documentation)
    // Full range: 100 - 149.975 MHz (25 kHz) or 100 - 149.9916 MHz (8.33 kHz)
    static constexpr double MIN_FREQ_MHZ = 100.000;
    static constexpr double MAX_FREQ_MHZ = 149.975;

    FrequencyPolicy() = default;

    void setCorrectionPolicy(FrequencyCorrectionPolicy policy) {
        m_correctionPolicy = policy;
    }

    FrequencyCorrectionPolicy correctionPolicy() const {
        return m_correctionPolicy;
    }

    void setChannelSpacing(ChannelSpacing spacing) {
        m_channelSpacing = spacing;
    }

    ChannelSpacing channelSpacing() const {
        return m_channelSpacing;
    }

    /**
     * @brief Validate and optionally correct frequency
     * @param freqMHz Input frequency in MHz
     * @return Validation result with corrected value
     */
    FrequencyValidationResult validate(double freqMHz) const {
        FrequencyValidationResult result;
        result.originalFreqMHz = freqMHz;
        result.correctedFreqMHz = freqMHz;

        // Step 1: Range check and clamp
        if (freqMHz < MIN_FREQ_MHZ) {
            result.wasClamped = true;
            result.correctedFreqMHz = MIN_FREQ_MHZ;
            result.message = QString::fromUtf8("Частота ниже минимума, исправлено: %1 -> %2 МГц")
                .arg(freqMHz, 0, 'f', 3)
                .arg(MIN_FREQ_MHZ, 0, 'f', 3);
        } else if (freqMHz > MAX_FREQ_MHZ) {
            result.wasClamped = true;
            result.correctedFreqMHz = MAX_FREQ_MHZ;
            result.message = QString::fromUtf8("Частота выше максимума, исправлено: %1 -> %2 МГц")
                .arg(freqMHz, 0, 'f', 3)
                .arg(MAX_FREQ_MHZ, 0, 'f', 3);
        }

        // Step 2: Round to channel grid
        double gridFreq = roundToGrid(result.correctedFreqMHz);
        if (std::abs(gridFreq - result.correctedFreqMHz) > 0.0001) {
            result.wasRounded = true;
            result.correctedFreqMHz = gridFreq;
            if (result.message.isEmpty()) {
                result.message = QString::fromUtf8("Частота округлена до сетки: %1 -> %2 МГц")
                    .arg(freqMHz, 0, 'f', 3)
                    .arg(gridFreq, 0, 'f', 3);
            }
        }

        // Step 3: Determine validity based on policy
        if (m_correctionPolicy == FrequencyCorrectionPolicy::WarnBlock &&
            (result.wasClamped || result.wasRounded)) {
            result.valid = false;
        } else {
            result.valid = true;
        }

        return result;
    }

    /**
     * @brief Round frequency to channel grid
     */
    double roundToGrid(double freqMHz) const {
        double stepKHz = (m_channelSpacing == ChannelSpacing::Spacing8_33kHz)
            ? 8.33333333
            : 25.0;

        double stepMHz = stepKHz / 1000.0;
        double baseFreq = MIN_FREQ_MHZ;

        double channels = std::round((freqMHz - baseFreq) / stepMHz);
        double result = baseFreq + channels * stepMHz;

        // Clamp to range
        if (result < MIN_FREQ_MHZ) result = MIN_FREQ_MHZ;
        if (result > MAX_FREQ_MHZ) result = MAX_FREQ_MHZ;

        return result;
    }

    /**
     * @brief Get valid frequencies list for combobox (main channels)
     */
    static QVector<double> getCommonFrequencies() {
        QVector<double> freqs;
        // Emergency and common frequencies
        freqs << 121.500  // Emergency
              << 121.600  // Ground
              << 122.800  // UNICOM
              << 123.450  // Air-to-air
              << 126.700  // Approach
              << 118.100  // Tower
              << 119.100  // Tower alt
              << 127.850; // ATIS
        return freqs;
    }

private:
    FrequencyCorrectionPolicy m_correctionPolicy = FrequencyCorrectionPolicy::WarnClamp;
    ChannelSpacing m_channelSpacing = ChannelSpacing::Spacing8_33kHz;
};

} // namespace rcms
