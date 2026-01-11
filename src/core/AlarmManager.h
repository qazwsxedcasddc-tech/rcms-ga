#pragma once

#include <QObject>
#include <QVector>
#include <QDateTime>
#include "protocol/IRadioDevice.h"

namespace rcms {

/**
 * @brief Alarm event with metadata
 */
struct AlarmEvent {
    QDateTime timestamp;
    QString deviceName;
    uint8_t deviceAddress;
    AlarmInfo alarm;
    bool acknowledged = false;
};

/**
 * @brief Manages alarm events and notifications
 */
class AlarmManager : public QObject {
    Q_OBJECT

public:
    explicit AlarmManager(QObject* parent = nullptr);

    /**
     * @brief Add new alarm event
     */
    void addAlarm(const QString& deviceName, uint8_t address, const AlarmInfo& alarm);

    /**
     * @brief Get all alarm events
     */
    const QVector<AlarmEvent>& alarms() const { return m_alarms; }

    /**
     * @brief Get unacknowledged alarms count
     */
    int unacknowledgedCount() const;

    /**
     * @brief Acknowledge alarm by index
     */
    void acknowledge(int index);

    /**
     * @brief Acknowledge all alarms
     */
    void acknowledgeAll();

    /**
     * @brief Clear all alarms
     */
    void clear();

    /**
     * @brief Enable/disable sound notifications
     */
    void setSoundEnabled(bool enabled) { m_soundEnabled = enabled; }
    bool isSoundEnabled() const { return m_soundEnabled; }

signals:
    /**
     * @brief Emitted when new alarm is added
     */
    void alarmAdded(const AlarmEvent& event);

    /**
     * @brief Emitted when alarm is acknowledged
     */
    void alarmAcknowledged(int index);

    /**
     * @brief Emitted when alarms are cleared
     */
    void alarmsCleared();

private:
    void playAlarmSound();

    QVector<AlarmEvent> m_alarms;
    bool m_soundEnabled = true;
    static constexpr int MAX_ALARMS = 1000;
};

} // namespace rcms
