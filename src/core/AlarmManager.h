#pragma once

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QTimer>
#include "protocol/IRadioDevice.h"

namespace rcms {

/**
 * @brief Sound playback mode for alarms
 */
enum class AlarmSoundMode {
    Once,           // Play sound once on alarm
    RepeatInterval, // Repeat every N seconds until ACK
    Continuous      // Continuous until ACK
};

/**
 * @brief Alarm event with metadata
 */
struct AlarmEvent {
    QString id;                     // Unique alarm ID
    QDateTime timestamp;
    QString deviceName;
    uint8_t deviceAddress;
    QString groupId;                // Device group
    AlarmInfo alarm;
    bool acknowledged = false;
    QDateTime ackTimestamp;
    QString ackUser;                // Who acknowledged (for future)

    /**
     * @brief Check if alarm is currently active (not cleared by device)
     */
    bool isActive = true;
};

/**
 * @brief Manages alarm events and notifications
 */
class AlarmManager : public QObject {
    Q_OBJECT

public:
    explicit AlarmManager(QObject* parent = nullptr);
    ~AlarmManager();

    /**
     * @brief Add new alarm event
     */
    void addAlarm(const QString& deviceName, uint8_t address,
                  const AlarmInfo& alarm, const QString& groupId = QString());

    /**
     * @brief Mark alarm as cleared (device recovered)
     */
    void clearAlarm(const QString& alarmId);

    /**
     * @brief Mark alarm as cleared by device address
     */
    void clearDeviceAlarms(uint8_t deviceAddress);

    /**
     * @brief Get all alarm events
     */
    const QVector<AlarmEvent>& alarms() const { return m_alarms; }

    /**
     * @brief Get active (non-cleared) alarms
     */
    QVector<AlarmEvent> activeAlarms() const;

    /**
     * @brief Get unacknowledged alarms count
     */
    int unacknowledgedCount() const;

    /**
     * @brief Get active unacknowledged alarms count
     */
    int activeUnacknowledgedCount() const;

    /**
     * @brief Acknowledge alarm by index
     */
    void acknowledge(int index);

    /**
     * @brief Acknowledge alarm by ID
     */
    void acknowledgeById(const QString& alarmId);

    /**
     * @brief Acknowledge all alarms for a device
     */
    void acknowledgeDevice(uint8_t deviceAddress);

    /**
     * @brief Acknowledge all alarms for a group
     */
    void acknowledgeGroup(const QString& groupId);

    /**
     * @brief Acknowledge all alarms
     */
    void acknowledgeAll();

    /**
     * @brief Clear all alarms from history
     */
    void clear();

    // Sound settings
    void setSoundEnabled(bool enabled);
    bool isSoundEnabled() const { return m_soundEnabled; }

    void setSoundMode(AlarmSoundMode mode) { m_soundMode = mode; }
    AlarmSoundMode soundMode() const { return m_soundMode; }

    void setRepeatIntervalSec(int seconds) { m_repeatIntervalSec = seconds; }
    int repeatIntervalSec() const { return m_repeatIntervalSec; }

    void setVolume(int percent) { m_volume = qBound(0, percent, 100); }
    int volume() const { return m_volume; }

    void setSoundFile(const QString& path) { m_soundFile = path; }
    QString soundFile() const { return m_soundFile; }

    /**
     * @brief Test sound playback
     */
    void testSound();

    /**
     * @brief Stop any playing sound
     */
    void stopSound();

signals:
    void alarmAdded(const AlarmEvent& event);
    void alarmCleared(const QString& alarmId);
    void alarmAcknowledged(int index);
    void alarmsCleared();
    void unacknowledgedCountChanged(int count);

private slots:
    void onRepeatTimer();

private:
    void playAlarmSound();
    void updateRepeatTimer();
    QString generateAlarmId() const;

    QVector<AlarmEvent> m_alarms;
    bool m_soundEnabled = true;
    AlarmSoundMode m_soundMode = AlarmSoundMode::Once;
    int m_repeatIntervalSec = 30;
    int m_volume = 80;
    QString m_soundFile = ":/sounds/alarm.wav";
    QTimer* m_repeatTimer = nullptr;

    static constexpr int MAX_ALARMS = 1000;
};

} // namespace rcms
