#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "DeviceTreeWidget.h"
#include "StatusPanel.h"
#include "ControlPanel.h"
#include "EventLogWidget.h"
#include "SettingsDialog.h"
#include "core/Logger.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QDockWidget>
#include <QMessageBox>
#include <QCloseEvent>

namespace rcms {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_deviceManager(std::make_unique<DeviceManager>(this))
    , m_alarmManager(std::make_unique<AlarmManager>(this))
    , m_configManager(std::make_unique<ConfigManager>())
{
    ui->setupUi(this);

    setWindowTitle("RCMS-GA - Система управления радиостанциями");
    setMinimumSize(1024, 768);

    setupUI();
    setupMenus();
    setupToolbar();
    setupConnections();
    loadConfiguration();

    statusBar()->showMessage("Готов к работе");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUI() {
    // Create main splitter
    auto* mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Left panel: Device tree
    m_deviceTree = new DeviceTreeWidget(this);
    m_deviceTree->setMinimumWidth(200);
    mainSplitter->addWidget(m_deviceTree);

    // Center: Status and Control
    auto* centerWidget = new QWidget(this);
    auto* centerLayout = new QVBoxLayout(centerWidget);

    m_statusPanel = new StatusPanel(this);
    m_controlPanel = new ControlPanel(this);

    centerLayout->addWidget(m_statusPanel, 2);
    centerLayout->addWidget(m_controlPanel, 1);

    mainSplitter->addWidget(centerWidget);

    // Set splitter sizes
    mainSplitter->setSizes({250, 600});

    setCentralWidget(mainSplitter);

    // Bottom dock: Event log
    auto* logDock = new QDockWidget("Журнал событий", this);
    m_eventLog = new EventLogWidget(this);
    logDock->setWidget(m_eventLog);
    addDockWidget(Qt::BottomDockWidgetArea, logDock);
}

void MainWindow::setupMenus() {
    // File menu
    auto* fileMenu = menuBar()->addMenu("&Файл");
    fileMenu->addAction("&Добавить устройство...", this, &MainWindow::onAddDevice);
    fileMenu->addAction("&Удалить устройство", this, &MainWindow::onRemoveDevice);
    fileMenu->addSeparator();
    fileMenu->addAction("&Настройки...", this, &MainWindow::onSettings);
    fileMenu->addSeparator();
    fileMenu->addAction("&Выход", this, &QMainWindow::close);

    // Control menu
    auto* controlMenu = menuBar()->addMenu("&Управление");
    controlMenu->addAction("&Начать опрос", this, &MainWindow::onStartPolling);
    controlMenu->addAction("&Остановить опрос", this, &MainWindow::onStopPolling);

    // Help menu
    auto* helpMenu = menuBar()->addMenu("&Справка");
    helpMenu->addAction("&О программе", this, &MainWindow::onAbout);
}

void MainWindow::setupToolbar() {
    auto* toolbar = addToolBar("Основные");

    toolbar->addAction(QIcon(":/icons/device_ok.png"), "Добавить", this, &MainWindow::onAddDevice);
    toolbar->addSeparator();
    toolbar->addAction("Старт", this, &MainWindow::onStartPolling);
    toolbar->addAction("Стоп", this, &MainWindow::onStopPolling);
}

void MainWindow::setupConnections() {
    connect(m_deviceTree, &DeviceTreeWidget::deviceSelected,
            this, &MainWindow::onDeviceSelected);

    connect(m_deviceManager.get(), &DeviceManager::deviceStatusChanged,
            this, &MainWindow::onDeviceStatusChanged);

    connect(m_deviceManager.get(), &DeviceManager::alarmDetected,
            this, &MainWindow::onAlarmDetected);

    connect(m_alarmManager.get(), &AlarmManager::alarmAdded,
            [this](const AlarmEvent& event) {
                m_eventLog->addEvent(event);
            });
}

void MainWindow::loadConfiguration() {
    if (!m_configManager->load("config/default.json")) {
        Logger::warn("Using default configuration");
    }

    // TODO: Create devices from configuration
}

void MainWindow::saveConfiguration() {
    m_configManager->save("config/default.json");
}

void MainWindow::closeEvent(QCloseEvent* event) {
    m_deviceManager->stopPolling();
    saveConfiguration();
    event->accept();
}

void MainWindow::onDeviceSelected(int index) {
    m_selectedDevice = index;

    if (index >= 0 && static_cast<size_t>(index) < m_deviceManager->devices().size()) {
        auto device = m_deviceManager->device(index);
        m_controlPanel->setDevice(device);
    } else {
        m_controlPanel->setDevice(nullptr);
    }
}

void MainWindow::onDeviceStatusChanged(size_t index, const DeviceStatus& status) {
    m_deviceTree->updateDeviceStatus(index, status);

    if (static_cast<int>(index) == m_selectedDevice) {
        m_statusPanel->updateStatus(status);
    }
}

void MainWindow::onAlarmDetected(size_t index, const AlarmInfo& alarm) {
    auto device = m_deviceManager->device(index);
    QString deviceName = device ? device->deviceId() : QString("Device %1").arg(index);
    uint8_t address = device ? device->modbusAddress() : 0;

    m_alarmManager->addAlarm(deviceName, address, alarm);
}

void MainWindow::onAddDevice() {
    // TODO: Show add device dialog
    QMessageBox::information(this, "Добавить устройство",
                             "Функция в разработке");
}

void MainWindow::onRemoveDevice() {
    if (m_selectedDevice >= 0) {
        m_deviceManager->removeDevice(m_selectedDevice);
        m_deviceTree->removeDevice(m_selectedDevice);
        m_selectedDevice = -1;
    }
}

void MainWindow::onSettings() {
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Apply settings
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "О программе RCMS-GA",
        "<h3>RCMS-GA v1.0.0</h3>"
        "<p>Универсальная система управления радиостанциями<br>"
        "гражданской авиации Российской Федерации</p>"
        "<p>Поддерживаемое оборудование:</p>"
        "<ul>"
        "<li>Фазан-19 П5 (АО \"Электроприбор\")</li>"
        "</ul>"
        "<p>&copy; 2026 RCMS-GA Team</p>"
        "<p>Лицензия: GPL-3.0</p>");
}

void MainWindow::onStartPolling() {
    m_deviceManager->startPolling(m_configManager->pollingInterval());
    statusBar()->showMessage("Опрос устройств запущен");
}

void MainWindow::onStopPolling() {
    m_deviceManager->stopPolling();
    statusBar()->showMessage("Опрос устройств остановлен");
}

} // namespace rcms
