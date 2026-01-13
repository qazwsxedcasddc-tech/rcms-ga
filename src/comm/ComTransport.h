#pragma once

#include "ITransport.h"
#include <QSerialPort>
#include <memory>

namespace rcms {

/**
 * @brief COM port transport for RS-485 communication
 */
class ComTransport : public ITransport {
public:
    /**
     * @brief Constructor
     * @param portName COM port name (e.g., "COM3", "/dev/ttyUSB0")
     * @param baudRate Baud rate (default 9600)
     * @param dataBits Data bits (default 8)
     * @param parity Parity (default None)
     * @param stopBits Stop bits (default 1)
     */
    ComTransport(const QString& portName,
                 int baudRate = 9600,
                 QSerialPort::DataBits dataBits = QSerialPort::Data8,
                 QSerialPort::Parity parity = QSerialPort::NoParity,
                 QSerialPort::StopBits stopBits = QSerialPort::OneStop);

    ~ComTransport() override;

    bool open() override;
    void close() override;
    bool isOpen() const override;

    qint64 write(const QByteArray& data) override;
    QByteArray read(int maxSize, int timeoutMs = 1000) override;
    void flush() override;

    QString lastError() const override { return m_lastError; }
    QString transportType() const override { return "COM"; }
    QString connectionString() const override { return m_portName; }

    // COM-specific settings
    void setBaudRate(int baudRate);
    int baudRate() const { return m_baudRate; }

private:
    QString m_portName;
    int m_baudRate;
    QSerialPort::DataBits m_dataBits;
    QSerialPort::Parity m_parity;
    QSerialPort::StopBits m_stopBits;
    std::unique_ptr<QSerialPort> m_port;
    QString m_lastError;
};

} // namespace rcms
