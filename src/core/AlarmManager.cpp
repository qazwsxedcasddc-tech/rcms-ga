#include "AlarmManager.h"
#include "Logger.h"
// #include <QSound> // TODO: Qt5::Multimedia

namespace rcms {

AlarmManager::AlarmManager(QObject* parent)
    : QObject(parent)
{
}

void AlarmManager::addAlarm(const QString& deviceName, uint8_t address, const AlarmInfo& alarm) {
    AlarmEvent event;
    event.timestamp = QDateTime::currentDateTime();
    event.deviceName = deviceName;
    event.deviceAddress = address;
    event.alarm = alarm;
    event.acknowledged = false;

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
    }

    emit alarmAdded(event);
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

void AlarmManager::acknowledge(int index) {
    if (index >= 0 && index < m_alarms.size()) {
        m_alarms[index].acknowledged = true;
        emit alarmAcknowledged(index);
    }
}

void AlarmManager::acknowledgeAll() {
    for (int i = 0; i < m_alarms.size(); ++i) {
        if (!m_alarms[i].acknowledged) {
            m_alarms[i].acknowledged = true;
            emit alarmAcknowledged(i);
        }
    }
}

void AlarmManager::clear() {
    m_alarms.clear();
    emit alarmsCleared();
}

void AlarmManager::playAlarmSound() {
    // TODO: Implement sound playback
    // QSound::play(":/sounds/alarm.wav");
}

} // namespace rcms
