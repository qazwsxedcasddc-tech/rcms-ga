#pragma once

#include <QTableWidget>
#include "core/AlarmManager.h"

namespace rcms {

/**
 * @brief Widget displaying event log table
 */
class EventLogWidget : public QTableWidget {
    Q_OBJECT

public:
    explicit EventLogWidget(QWidget* parent = nullptr);

    /**
     * @brief Add event to log
     */
    void addEvent(const AlarmEvent& event);

    /**
     * @brief Clear all events
     */
    void clearEvents();

private:
    void setupUI();
    QString severityToString(AlarmSeverity severity);
    QColor severityToColor(AlarmSeverity severity);
};

} // namespace rcms
