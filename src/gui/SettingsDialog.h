#pragma once

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

namespace rcms {

/**
 * @brief Application settings dialog
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

private slots:
    void onRefreshPorts();
    void onAccept();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();

    Ui::SettingsDialog* ui;
};

} // namespace rcms
