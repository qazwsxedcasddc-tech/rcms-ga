#pragma once

#include <QTreeWidget>
#include "protocol/IRadioDevice.h"

namespace rcms {

/**
 * @brief Widget displaying device tree with status icons
 */
class DeviceTreeWidget : public QTreeWidget {
    Q_OBJECT

public:
    explicit DeviceTreeWidget(QWidget* parent = nullptr);

    /**
     * @brief Add device to tree
     */
    void addDevice(const QString& name, const QString& type, uint8_t address);

    /**
     * @brief Remove device by index
     */
    void removeDevice(int index);

    /**
     * @brief Update device status display
     */
    void updateDeviceStatus(size_t index, const DeviceStatus& status);

    /**
     * @brief Clear all devices
     */
    void clear();

signals:
    /**
     * @brief Emitted when device is selected
     */
    void deviceSelected(int index);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    void updateStatusIcon(QTreeWidgetItem* item, bool online, bool hasAlarm);
};

} // namespace rcms
