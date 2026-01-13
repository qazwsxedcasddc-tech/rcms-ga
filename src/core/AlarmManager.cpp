#include "AlarmManager.h"
#include "Logger.h"
#include <QUuid>

namespace rcms {

AlarmManager::AlarmManager(QObject* parent)
    : QObject(parent)
    , m_repeatTimer(new QTimer(this))
{
    connect(m_repeatTimer, &QTimer::timeout, this, &AlarmManager::onRepeatTimer);
}

AlarmManager::~AlarmManager() {
    stopSound();
}

QString AlarmManager::generateAlarmId() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void AlarmManager::addAlarm(const QString& deviceName, uint8_t address,
                            const AlarmInfo& alarm, const QString& groupId) {
    AlarmEvent event;
    event.id = generateAlarmId();
    event.timestamp = QDateTime::currentDateTime();
    event.deviceName = deviceName;
    event.deviceAddress = address;
    event.groupId = groupId;
    event.alarm = alarm;
    event.acknowledged = false;
    event.isActive = true;

    // Limit stored alarms
    if (m_alarms.size() >= MAX_ALARMS) {
        m_alarms.removeFirst();
    }

    m_alarms.append(event);

    Logger::warn("Alarm from {} [{}]: {} (code: {})",
                 deviceName.toStdString(),
                 address,
                 alarm.message.toStdString(),
                 alarm.code);

    if (m_soundEnabled && alarm.severity >= AlarmSeverity::Error) {
        playAlarmSound();
        updateRepeatTimer();
    }

    emit alarmAdded(event);
    emit unacknowledgedCountChanged(unacknowledgedCount());
}

void AlarmManager::clearAlarm(const QString& alarmId) {
    for (auto& alarm : m_alarms) {
        if (alarm.id == alarmId) {
            alarm.isActive = false;
            emit alarmCleared(alarmId);
            updateRepeatTimer();
            break;
        }
    }
}

void AlarmManager::clearDeviceAlarms(uint8_t deviceAddress) {
    for (auto& alarm : m_alarms) {
        if (alarm.deviceAddress == deviceAddress && alarm.isActive) {
            alarm.isActive = false;
            emit alarmCleared(alarm.id);
        }
    }
    updateRepeatTimer();
}

QVector<AlarmEvent> AlarmManager::activeAlarms() const {
    QVector<AlarmEvent> result;
    for (const auto& alarm : m_alarms) {
        if (alarm.isActive) {
            result.append(alarm);
        }
    }
    return result;
}

int AlarmManager::unacknowledgedCount() const {
    int count = 0;
    for (const auto& alarm : m_alarms) {
        if (!alarm.acknowledged) {
            ++count;
        }
    }
    return count;
}

int AlarmManager::activeUnacknowledgedCount() const {
    int count = 0;
    for (const auto& alarm : m_alarms) {
        if (alarm.isActive && !alarm.acknowledged) {
            ++count;
        }
    }
    return count;
}

void AlarmManager::acknowledge(int index) {
    if (index >= 0 && index < m_alarms.size()) {
        m_alarms[index].acknowledged = true;
        m_alarms[index].ackTimestamp = QDateTime::currentDateTime();
        emit alarmAcknowledged(index);
        emit unacknowledgedCountChanged(unacknowledgedCount());
        updateRepeatTimer();
    }
}

void AlarmManager::acknowledgeById(const QString& alarmId) {
    for (int i = 0; i < m_alarms.size(); ++i) {
        if (m_alarms[i].id == alarmId) {
            acknowledge(i);
            break;
        }
    }
}

void AlarmManager::acknowledgeDevice(uint8_t deviceAddress) {
    for (int i = 0; i < m_alarms.size(); ++i) {
        if (m_alarms[i].deviceAddress == deviceAddress && !m_alarms[i].acknowledged) {
            m_alarms[i].acknowledged = true;
            m_alarms[i].ackTimestamp = QDateTime::currentDateTime();
            emit alarmAcknowledged(i);
        }
    }
    emit unacknowledgedCountChanged(unacknowledgedCount());
    updateRepeatTimer();
}

void AlarmManager::acknowledgeGroup(const QString& groupId) {
    for (int i = 0; i < m_alarms.size(); ++i) {
        if (m_alarms[i].groupId == groupId && !m_alarms[i].acknowledged) {
            m_alarms[i].acknowledged = true;
            m_alarms[i].ackTimestamp = QDateTime::currentDateTime();
            emit alarmAcknowledged(i);
        }
    }
    emit unacknowledgedCountChanged(unacknowledgedCount());
    updateRepeatTimer();
}

void AlarmManager::acknowledgeAll() {
    for (int i = 0; i < m_alarms.size(); ++i) {
        if (!m_alarms[i].acknowledged) {
            m_alarms[i].acknowledged = true;
            m_alarms[i].ackTimestamp = QDateTime::currentDateTime();
            emit alarmAcknowledged(i);
        }
    }
    emit unacknowledgedCountChanged(0);
    updateRepeatTimer();
}

void AlarmManager::clear() {
    m_alarms.clear();
    stopSound();
    emit alarmsCleared();
    emit unacknowledgedCountChanged(0);
}

void AlarmManager::setSoundEnabled(bool enabled) {
    m_soundEnabled = enabled;
    if (!enabled) {
        stopSound();
    }
}

void AlarmManager::testSound() {
    playAlarmSound();
}

void AlarmManager::stopSound() {
    m_repeatTimer->stop();
    // TODO: Stop actual audio playback when implemented
}

void AlarmManager::onRepeatTimer() {
    if (activeUnacknowledgedCount() > 0) {
        playAlarmSound();
    } else {
        m_repeatTimer->stop();
    }
}

void AlarmManager::playAlarmSound() {
    // TODO: Implement actual sound playback
    // Options:
    // 1. QSound (requires Qt5::Multimedia)
    // 2. QMediaPlayer (Qt5::Multimedia)
    // 3. Platform-specific API
    Logger::debug("Playing alarm sound: {}", m_soundFile.toStdString());
}

void AlarmManager::updateRepeatTimer() {
    if (!m_soundEnabled) {
        m_repeatTimer->stop();
        return;
    }

    bool hasActiveUnacked = activeUnacknowledgedCount() > 0;

    if (hasActiveUnacked && m_soundMode == AlarmSoundMode::RepeatInterval) {
        if (!m_repeatTimer->isActive()) {
            m_repeatTimer->start(m_repeatIntervalSec * 1000);
        }
    } else {
        m_repeatTimer->stop();
    }
}

} // namespace rcms
