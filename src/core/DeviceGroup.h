#pragma once

#include <QString>
#include <QVector>
#include <QColor>
#include <QDateTime>

namespace rcms {

/**
 * @brief Device group for logical organization
 *
 * Groups allow organizing devices by location, function, etc.
 */
struct DeviceGroup {
    QString id;                     // Unique group ID
    QString name;                   // Display name (e.g., "Tower-1", "Sector-A")
    QString description;            // Optional description
    QColor color;                   // Color for visual distinction
    int sortOrder = 0;              // Sort order in UI
    bool expanded = true;           // UI state: expanded/collapsed

    /**
     * @brief Generate unique group ID
     */
    static QString generateId() {
        return QString("grp_%1").arg(
            QString::number(QDateTime::currentMSecsSinceEpoch(), 16)
        );
    }
};

/**
 * @brief Special group IDs
 */
namespace groups {
    const QString UNGROUPED = "__ungrouped__";
    const QString ALL_DEVICES = "__all__";
}

} // namespace rcms
