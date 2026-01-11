#pragma once

#include <QObject>
#include <QTimer>
#include <memory>
#include <vector>
#include "protocol/IRadioDevice.h"

namespace rcms {

/**
 * @brief Manages all radio devices
 *
 * Handles device polling, status aggregation, and device lifecycle
 */
class DeviceManager : public QObject {
    Q_OBJECT

public:
    explicit DeviceManager(QObject* parent = nullptr);
    ~DeviceManager();

    /**
     * @brief Add a device to management
     */
    void addDevice(std::shared_ptr<IRadioDevice> device);

    /**
     * @brief Remove device by index
     */
    void removeDevice(size_t index);

    /**
     * @brief Get all managed devices
     */
    const std::vector<std::shared_ptr<IRadioDevice>>& devices() const { return m_devices; }

    /**
     * @brief Get device by index
     */
    std::shared_ptr<IRadioDevice> device(size_t index) const;

    /**
     * @brief Start polling all devices
     */
    void startPolling(int intervalMs = 1000);

    /**
     * @brief Stop polling
     */
    void stopPolling();

    /**
     * @brief Check if polling is active
     */
    bool isPolling() const { return m_polling; }

signals:
    /**
     * @brief Emitted when device status changes
     */
    void deviceStatusChanged(size_t index, const DeviceStatus& status);

    /**
     * @brief Emitted when device goes online/offline
     */
    void deviceOnlineChanged(size_t index, bool online);

    /**
     * @brief Emitted when alarm is detected
     */
    void alarmDetected(size_t index, const AlarmInfo& alarm);

private slots:
    void pollDevices();

private:
    std::vector<std::shared_ptr<IRadioDevice>> m_devices;
    QTimer* m_pollTimer;
    bool m_polling = false;
};

} // namespace rcms
