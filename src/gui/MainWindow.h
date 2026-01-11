#pragma once

#include <QMainWindow>
#include <memory>
#include "core/DeviceManager.h"
#include "core/AlarmManager.h"
#include "core/ConfigManager.h"

namespace Ui {
class MainWindow;
}

namespace rcms {

class DeviceTreeWidget;
class StatusPanel;
class ControlPanel;
class EventLogWidget;

/**
 * @brief Main application window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onDeviceSelected(int index);
    void onDeviceStatusChanged(size_t index, const DeviceStatus& status);
    void onAlarmDetected(size_t index, const AlarmInfo& alarm);

    void onAddDevice();
    void onRemoveDevice();
    void onSettings();
    void onAbout();

    void onStartPolling();
    void onStopPolling();

private:
    void setupUI();
    void setupMenus();
    void setupToolbar();
    void setupConnections();
    void loadConfiguration();
    void saveConfiguration();

    Ui::MainWindow* ui;

    // Managers
    std::unique_ptr<DeviceManager> m_deviceManager;
    std::unique_ptr<AlarmManager> m_alarmManager;
    std::unique_ptr<ConfigManager> m_configManager;

    // Widgets
    DeviceTreeWidget* m_deviceTree;
    StatusPanel* m_statusPanel;
    ControlPanel* m_controlPanel;
    EventLogWidget* m_eventLog;

    int m_selectedDevice = -1;
};

} // namespace rcms
