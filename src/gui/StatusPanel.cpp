#include "StatusPanel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>

namespace rcms {

StatusPanel::StatusPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    clear();
}

void StatusPanel::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Status group
    auto* statusGroup = new QGroupBox("Состояние устройства", this);
    auto* statusLayout = new QGridLayout(statusGroup);

    int row = 0;

    statusLayout->addWidget(new QLabel("Связь:"), row, 0);
    m_lblOnline = new QLabel();
    statusLayout->addWidget(m_lblOnline, row++, 1);

    statusLayout->addWidget(new QLabel("Частота:"), row, 0);
    m_lblFrequency = new QLabel();
    m_lblFrequency->setStyleSheet("font-size: 18px; font-weight: bold;");
    statusLayout->addWidget(m_lblFrequency, row++, 1);

    statusLayout->addWidget(new QLabel("Режим управления:"), row, 0);
    m_lblMode = new QLabel();
    statusLayout->addWidget(m_lblMode, row++, 1);

    statusLayout->addWidget(new QLabel("Режим работы:"), row, 0);
    m_lblWorkMode = new QLabel();
    statusLayout->addWidget(m_lblWorkMode, row++, 1);

    statusLayout->addWidget(new QLabel("Тип линии:"), row, 0);
    m_lblLineType = new QLabel();
    statusLayout->addWidget(m_lblLineType, row++, 1);

    statusLayout->addWidget(new QLabel("Передача:"), row, 0);
    m_lblTransmitting = new QLabel();
    statusLayout->addWidget(m_lblTransmitting, row++, 1);

    statusLayout->addWidget(new QLabel("ПШ:"), row, 0);
    m_lblSquelch = new QLabel();
    statusLayout->addWidget(m_lblSquelch, row++, 1);

    mainLayout->addWidget(statusGroup);

    // Parameters group
    auto* paramsGroup = new QGroupBox("Параметры", this);
    auto* paramsLayout = new QGridLayout(paramsGroup);

    row = 0;

    paramsLayout->addWidget(new QLabel("Уровень сигнала:"), row, 0);
    m_lblSignalLevel = new QLabel();
    paramsLayout->addWidget(m_lblSignalLevel, row++, 1);

    paramsLayout->addWidget(new QLabel("Напряжение 24В:"), row, 0);
    m_lblVoltage = new QLabel();
    paramsLayout->addWidget(m_lblVoltage, row++, 1);

    paramsLayout->addWidget(new QLabel("Температура:"), row, 0);
    m_lblTemperature = new QLabel();
    paramsLayout->addWidget(m_lblTemperature, row++, 1);

    paramsLayout->addWidget(new QLabel("Наработка:"), row, 0);
    m_lblOperatingHours = new QLabel();
    paramsLayout->addWidget(m_lblOperatingHours, row++, 1);

    paramsLayout->addWidget(new QLabel("Обновлено:"), row, 0);
    m_lblLastUpdate = new QLabel();
    paramsLayout->addWidget(m_lblLastUpdate, row++, 1);

    mainLayout->addWidget(paramsGroup);
    mainLayout->addStretch();
}

void StatusPanel::updateStatus(const DeviceStatus& status) {
    // Connection status
    if (status.online) {
        m_lblOnline->setText("<span style='color: green;'>Есть</span>");
    } else {
        m_lblOnline->setText("<span style='color: red;'>Нет</span>");
    }

    // Frequency
    m_lblFrequency->setText(formatFrequency(status.frequencyMHz));

    // Modes
    m_lblMode->setText(status.mode);
    m_lblWorkMode->setText(status.workMode);
    m_lblLineType->setText(status.lineType);

    // TX status
    if (status.isTransmitting) {
        m_lblTransmitting->setText("<span style='color: red; font-weight: bold;'>ВКЛ</span>");
    } else {
        m_lblTransmitting->setText("ВЫКЛ");
    }

    // Squelch
    m_lblSquelch->setText(status.squelchEnabled ?
                          QString("ВКЛ (ур. %1)").arg(status.squelchLevel) : "ВЫКЛ");

    // Parameters
    m_lblSignalLevel->setText(QString::number(status.signalLevel));
    m_lblVoltage->setText(QString("%1 В").arg(status.voltage24V, 0, 'f', 1));
    m_lblTemperature->setText(QString("%1 °C").arg(status.temperature, 0, 'f', 1));
    m_lblOperatingHours->setText(QString("%1 ч").arg(status.operatingHours));
    m_lblLastUpdate->setText(status.lastUpdate.toString("hh:mm:ss"));
}

void StatusPanel::clear() {
    m_lblOnline->setText("-");
    m_lblFrequency->setText("--- МГц");
    m_lblMode->setText("-");
    m_lblWorkMode->setText("-");
    m_lblLineType->setText("-");
    m_lblTransmitting->setText("-");
    m_lblSquelch->setText("-");
    m_lblSignalLevel->setText("-");
    m_lblVoltage->setText("-");
    m_lblTemperature->setText("-");
    m_lblOperatingHours->setText("-");
    m_lblLastUpdate->setText("-");
}

QString StatusPanel::formatFrequency(double freqMHz) {
    return QString("%1 МГц").arg(freqMHz, 0, 'f', 3);
}

} // namespace rcms
