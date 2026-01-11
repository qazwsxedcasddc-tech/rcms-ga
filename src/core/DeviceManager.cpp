#include "DeviceManager.h"
#include "Logger.h"

namespace rcms {

DeviceManager::DeviceManager(QObject* parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
{
    connect(m_pollTimer, &QTimer::timeout, this, &DeviceManager::pollDevices);
}

DeviceManager::~DeviceManager() {
    stopPolling();
}

void DeviceManager::addDevice(std::shared_ptr<IRadioDevice> device) {
    m_devices.push_back(device);
    Logger::info("Added device: {} (addr: {})",
                 device->deviceId().toStdString(),
                 device->modbusAddress());
}

void DeviceManager::removeDevice(size_t index) {
    if (index < m_devices.size()) {
        auto& dev = m_devices[index];
        Logger::info("Removing device: {}", dev->deviceId().toStdString());
        dev->close();
        m_devices.erase(m_devices.begin() + index);
    }
}

std::shared_ptr<IRadioDevice> DeviceManager::device(size_t index) const {
    if (index < m_devices.size()) {
        return m_devices[index];
    }
    return nullptr;
}

void DeviceManager::startPolling(int intervalMs) {
    if (!m_polling) {
        m_pollTimer->start(intervalMs);
        m_polling = true;
        Logger::info("Started polling with {}ms interval", intervalMs);
    }
}

void DeviceManager::stopPolling() {
    if (m_polling) {
        m_pollTimer->stop();
        m_polling = false;
        Logger::info("Stopped polling");
    }
}

void DeviceManager::pollDevices() {
    for (size_t i = 0; i < m_devices.size(); ++i) {
        auto& dev = m_devices[i];

        if (!dev->isOpen()) {
            continue;
        }

        DeviceStatus status;
        bool wasOnline = status.online;

        if (dev->readStatus(status)) {
            if (!wasOnline && status.online) {
                emit deviceOnlineChanged(i, true);
            }
            emit deviceStatusChanged(i, status);

            // Check for alarms
            QVector<AlarmInfo> alarms;
            if (dev->readAlarms(alarms)) {
                for (const auto& alarm : alarms) {
                    emit alarmDetected(i, alarm);
                }
            }
        } else {
            if (wasOnline) {
                emit deviceOnlineChanged(i, false);
            }
        }
    }
}

} // namespace rcms
