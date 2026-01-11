#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include "comm/SerialPort.h"

namespace rcms {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowTitle("Настройки");

    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::setupUI() {
    connect(ui->btnRefreshPorts, &QPushButton::clicked,
            this, &SettingsDialog::onRefreshPorts);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &SettingsDialog::onAccept);

    // Populate baud rates
    for (int baud : SerialPort::standardBaudRates()) {
        ui->cmbBaudRate->addItem(QString::number(baud), baud);
    }
    ui->cmbBaudRate->setCurrentText("9600");

    onRefreshPorts();
}

void SettingsDialog::onRefreshPorts() {
    ui->cmbPort->clear();

    for (const auto& info : SerialPort::availablePortsInfo()) {
        ui->cmbPort->addItem(SerialPort::formatPortInfo(info), info.portName());
    }
}

void SettingsDialog::loadSettings() {
    // TODO: Load from QSettings
    ui->spnPollingInterval->setValue(1000);
    ui->chkSoundEnabled->setChecked(true);
}

void SettingsDialog::saveSettings() {
    // TODO: Save to QSettings
}

void SettingsDialog::onAccept() {
    saveSettings();
    accept();
}

} // namespace rcms
