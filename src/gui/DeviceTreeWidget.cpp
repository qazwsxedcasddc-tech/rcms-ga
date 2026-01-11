#include "DeviceTreeWidget.h"
#include <QHeaderView>

namespace rcms {

DeviceTreeWidget::DeviceTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    setHeaderLabels({"Устройство", "Адрес", "Статус"});
    setColumnCount(3);

    header()->setStretchLastSection(true);
    setColumnWidth(0, 150);
    setColumnWidth(1, 50);

    setRootIsDecorated(false);
    setAlternatingRowColors(true);

    connect(this, &QTreeWidget::itemClicked,
            this, &DeviceTreeWidget::onItemClicked);
}

void DeviceTreeWidget::addDevice(const QString& name, const QString& type, uint8_t address) {
    auto* item = new QTreeWidgetItem(this);
    item->setText(0, name);
    item->setText(1, QString::number(address));
    item->setText(2, "Offline");
    item->setData(0, Qt::UserRole, topLevelItemCount() - 1);

    // Set offline icon
    updateStatusIcon(item, false, false);
}

void DeviceTreeWidget::removeDevice(int index) {
    if (index >= 0 && index < topLevelItemCount()) {
        delete takeTopLevelItem(index);

        // Update indices
        for (int i = index; i < topLevelItemCount(); ++i) {
            topLevelItem(i)->setData(0, Qt::UserRole, i);
        }
    }
}

void DeviceTreeWidget::updateDeviceStatus(size_t index, const DeviceStatus& status) {
    if (static_cast<int>(index) < topLevelItemCount()) {
        auto* item = topLevelItem(static_cast<int>(index));

        if (status.online) {
            QString statusText = QString("%1 МГц").arg(status.frequencyMHz, 0, 'f', 3);
            if (status.isTransmitting) {
                statusText += " [TX]";
            }
            item->setText(2, statusText);
        } else {
            item->setText(2, "Offline");
        }

        bool hasAlarm = !status.errorCodes.isEmpty();
        updateStatusIcon(item, status.online, hasAlarm);
    }
}

void DeviceTreeWidget::clear() {
    QTreeWidget::clear();
}

void DeviceTreeWidget::onItemClicked(QTreeWidgetItem* item, int /*column*/) {
    int index = item->data(0, Qt::UserRole).toInt();
    emit deviceSelected(index);
}

void DeviceTreeWidget::updateStatusIcon(QTreeWidgetItem* item, bool online, bool hasAlarm) {
    QIcon icon;

    if (!online) {
        icon = QIcon(":/icons/device_offline.png");
    } else if (hasAlarm) {
        icon = QIcon(":/icons/device_error.png");
    } else {
        icon = QIcon(":/icons/device_ok.png");
    }

    item->setIcon(0, icon);
}

} // namespace rcms
