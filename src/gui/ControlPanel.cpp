#include "ControlPanel.h"
#include "core/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QDoubleValidator>

namespace rcms {

ControlPanel::ControlPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    updateEnabled();
}

void ControlPanel::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Frequency control group
    auto* freqGroup = new QGroupBox("Управление частотой", this);
    auto* freqLayout = new QHBoxLayout(freqGroup);

    freqLayout->addWidget(new QLabel("Частота (МГц):"));

    m_edtFrequency = new QLineEdit(this);
    m_edtFrequency->setPlaceholderText("118.000 - 136.975");
    m_edtFrequency->setValidator(new QDoubleValidator(118.0, 137.0, 3, this));
    freqLayout->addWidget(m_edtFrequency);

    m_btnSetFreq = new QPushButton("Установить", this);
    connect(m_btnSetFreq, &QPushButton::clicked, this, &ControlPanel::onSetFrequency);
    freqLayout->addWidget(m_btnSetFreq);

    mainLayout->addWidget(freqGroup);

    // Squelch control group
    auto* squelchGroup = new QGroupBox("Подавление шума (ПШ)", this);
    auto* squelchLayout = new QHBoxLayout(squelchGroup);

    m_chkSquelch = new QCheckBox("Включить ПШ", this);
    connect(m_chkSquelch, &QCheckBox::stateChanged, this, &ControlPanel::onSquelchChanged);
    squelchLayout->addWidget(m_chkSquelch);

    squelchLayout->addWidget(new QLabel("Уровень:"));

    m_spnSquelchLevel = new QSpinBox(this);
    m_spnSquelchLevel->setRange(1, 10);
    m_spnSquelchLevel->setValue(5);
    connect(m_spnSquelchLevel, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ControlPanel::onSquelchLevelChanged);
    squelchLayout->addWidget(m_spnSquelchLevel);

    squelchLayout->addStretch();

    mainLayout->addWidget(squelchGroup);

    // PTT control group
    auto* pttGroup = new QGroupBox("Передача (PTT)", this);
    auto* pttLayout = new QHBoxLayout(pttGroup);

    m_btnPTT = new QPushButton("PTT (удерживать)", this);
    m_btnPTT->setStyleSheet("QPushButton { background-color: #cc0000; color: white; "
                            "font-weight: bold; padding: 10px; }"
                            "QPushButton:pressed { background-color: #ff0000; }");
    connect(m_btnPTT, &QPushButton::pressed, this, &ControlPanel::onPTTPressed);
    connect(m_btnPTT, &QPushButton::released, this, &ControlPanel::onPTTReleased);
    pttLayout->addWidget(m_btnPTT);

    mainLayout->addWidget(pttGroup);

    mainLayout->addStretch();
}

void ControlPanel::setDevice(std::shared_ptr<IRadioDevice> device) {
    m_device = device;
    updateEnabled();
}

void ControlPanel::updateEnabled() {
    bool enabled = m_device && m_device->isOpen();

    m_edtFrequency->setEnabled(enabled);
    m_btnSetFreq->setEnabled(enabled);
    m_chkSquelch->setEnabled(enabled);
    m_spnSquelchLevel->setEnabled(enabled && m_chkSquelch->isChecked());
    m_btnPTT->setEnabled(enabled);
}

void ControlPanel::onSetFrequency() {
    if (!m_device) return;

    bool ok;
    double freq = m_edtFrequency->text().toDouble(&ok);

    if (!ok || freq < 118.0 || freq > 136.975) {
        QMessageBox::warning(this, "Ошибка",
                             "Некорректная частота. Допустимый диапазон: 118.000 - 136.975 МГц");
        return;
    }

    if (m_device->setFrequency(freq)) {
        Logger::info("Frequency set to {} MHz", freq);
    } else {
        QMessageBox::warning(this, "Ошибка",
                             "Не удалось установить частоту");
    }
}

void ControlPanel::onSquelchChanged(int state) {
    if (!m_device) return;

    bool enabled = (state == Qt::Checked);
    m_spnSquelchLevel->setEnabled(enabled);

    if (m_device->setSquelch(enabled, m_spnSquelchLevel->value())) {
        Logger::info("Squelch {} (level: {})",
                     enabled ? "enabled" : "disabled",
                     m_spnSquelchLevel->value());
    }
}

void ControlPanel::onSquelchLevelChanged(int value) {
    if (!m_device || !m_chkSquelch->isChecked()) return;

    m_device->setSquelch(true, value);
}

void ControlPanel::onPTTPressed() {
    if (!m_device) return;

    if (m_device->setPTT(true)) {
        Logger::info("PTT activated");
        m_btnPTT->setText(">>> ПЕРЕДАЧА <<<");
    }
}

void ControlPanel::onPTTReleased() {
    if (!m_device) return;

    if (m_device->setPTT(false)) {
        Logger::info("PTT deactivated");
        m_btnPTT->setText("PTT (удерживать)");
    }
}

} // namespace rcms
