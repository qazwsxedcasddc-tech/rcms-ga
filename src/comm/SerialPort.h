#pragma once

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QStringList>

namespace rcms {

/**
 * @brief Serial port utilities and wrapper
 */
class SerialPort {
public:
    /**
     * @brief Get list of available serial ports
     * @return List of port names
     */
    static QStringList availablePorts();

    /**
     * @brief Get detailed info about available ports
     * @return List of QSerialPortInfo objects
     */
    static QList<QSerialPortInfo> availablePortsInfo();

    /**
     * @brief Check if port exists
     * @param portName Port name to check
     * @return true if port exists
     */
    static bool portExists(const QString& portName);

    /**
     * @brief Get default baud rates for radio equipment
     * @return List of common baud rates
     */
    static QList<int> standardBaudRates();

    /**
     * @brief Format port info for display
     * @param info Port info
     * @return Formatted string
     */
    static QString formatPortInfo(const QSerialPortInfo& info);
};

} // namespace rcms
