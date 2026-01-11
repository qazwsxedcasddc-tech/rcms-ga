#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <memory>
#include "protocol/IRadioDevice.h"

namespace rcms {

/**
 * @brief Panel for device control (frequency, squelch, PTT)
 */
class ControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit ControlPanel(QWidget* parent = nullptr);

    /**
     * @brief Set the device to control
     */
    void setDevice(std::shared_ptr<IRadioDevice> device);

private slots:
    void onSetFrequency();
    void onSquelchChanged(int state);
    void onSquelchLevelChanged(int value);
    void onPTTPressed();
    void onPTTReleased();

private:
    void setupUI();
    void updateEnabled();

    std::shared_ptr<IRadioDevice> m_device;

    QLineEdit* m_edtFrequency;
    QPushButton* m_btnSetFreq;
    QCheckBox* m_chkSquelch;
    QSpinBox* m_spnSquelchLevel;
    QPushButton* m_btnPTT;
};

} // namespace rcms
