#include "SerialPort.h"

namespace rcms {

QStringList SerialPort::availablePorts() {
    QStringList ports;
    for (const auto& info : QSerialPortInfo::availablePorts()) {
        ports.append(info.portName());
    }
    return ports;
}

QList<QSerialPortInfo> SerialPort::availablePortsInfo() {
    return QSerialPortInfo::availablePorts();
}

bool SerialPort::portExists(const QString& portName) {
    for (const auto& info : QSerialPortInfo::availablePorts()) {
        if (info.portName() == portName) {
            return true;
        }
    }
    return false;
}

QList<int> SerialPort::standardBaudRates() {
    return {9600, 19200, 38400, 57600, 115200};
}

QString SerialPort::formatPortInfo(const QSerialPortInfo& info) {
    QString result = info.portName();

    if (!info.description().isEmpty()) {
        result += " - " + info.description();
    }

    if (!info.manufacturer().isEmpty()) {
        result += " (" + info.manufacturer() + ")";
    }

    return result;
}

} // namespace rcms
