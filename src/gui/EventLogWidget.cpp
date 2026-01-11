#include "EventLogWidget.h"
#include <QHeaderView>

namespace rcms {

EventLogWidget::EventLogWidget(QWidget* parent)
    : QTableWidget(parent)
{
    setupUI();
}

void EventLogWidget::setupUI() {
    setColumnCount(5);
    setHorizontalHeaderLabels({"Время", "Устройство", "Тип", "Код", "Сообщение"});

    horizontalHeader()->setStretchLastSection(true);
    setColumnWidth(0, 100);
    setColumnWidth(1, 120);
    setColumnWidth(2, 80);
    setColumnWidth(3, 60);

    setAlternatingRowColors(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    verticalHeader()->setVisible(false);
}

void EventLogWidget::addEvent(const AlarmEvent& event) {
    int row = 0;
    insertRow(row);

    // Time
    auto* timeItem = new QTableWidgetItem(event.timestamp.toString("hh:mm:ss"));
    setItem(row, 0, timeItem);

    // Device
    auto* deviceItem = new QTableWidgetItem(
        QString("%1 [%2]").arg(event.deviceName).arg(event.deviceAddress));
    setItem(row, 1, deviceItem);

    // Severity
    auto* severityItem = new QTableWidgetItem(severityToString(event.alarm.severity));
    severityItem->setBackground(severityToColor(event.alarm.severity));
    setItem(row, 2, severityItem);

    // Code
    auto* codeItem = new QTableWidgetItem(
        QString("0x%1").arg(event.alarm.code, 4, 16, QChar('0')).toUpper());
    setItem(row, 3, codeItem);

    // Message
    auto* messageItem = new QTableWidgetItem(event.alarm.message);
    setItem(row, 4, messageItem);

    // Limit rows
    while (rowCount() > 1000) {
        removeRow(rowCount() - 1);
    }
}

void EventLogWidget::clearEvents() {
    setRowCount(0);
}

QString EventLogWidget::severityToString(AlarmSeverity severity) {
    switch (severity) {
        case AlarmSeverity::Info: return "Инфо";
        case AlarmSeverity::Warning: return "Внимание";
        case AlarmSeverity::Error: return "Ошибка";
        case AlarmSeverity::Critical: return "Авария";
        default: return "?";
    }
}

QColor EventLogWidget::severityToColor(AlarmSeverity severity) {
    switch (severity) {
        case AlarmSeverity::Info: return QColor(200, 200, 255);
        case AlarmSeverity::Warning: return QColor(255, 255, 150);
        case AlarmSeverity::Error: return QColor(255, 200, 150);
        case AlarmSeverity::Critical: return QColor(255, 150, 150);
        default: return QColor(255, 255, 255);
    }
}

} // namespace rcms
