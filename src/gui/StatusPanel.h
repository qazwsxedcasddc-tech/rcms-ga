#pragma once

#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include "protocol/IRadioDevice.h"

namespace rcms {

/**
 * @brief Panel displaying detailed device status
 */
class StatusPanel : public QWidget {
    Q_OBJECT

public:
    explicit StatusPanel(QWidget* parent = nullptr);

    /**
     * @brief Update display with new status
     */
    void updateStatus(const DeviceStatus& status);

    /**
     * @brief Clear all displayed values
     */
    void clear();

private:
    void setupUI();
    QString formatFrequency(double freqMHz);

    // Labels for status values
    QLabel* m_lblOnline;
    QLabel* m_lblFrequency;
    QLabel* m_lblMode;
    QLabel* m_lblWorkMode;
    QLabel* m_lblLineType;
    QLabel* m_lblTransmitting;
    QLabel* m_lblSquelch;
    QLabel* m_lblSignalLevel;
    QLabel* m_lblVoltage;
    QLabel* m_lblTemperature;
    QLabel* m_lblOperatingHours;
    QLabel* m_lblLastUpdate;
};

} // namespace rcms
