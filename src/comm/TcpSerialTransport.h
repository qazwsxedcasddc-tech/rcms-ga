#pragma once

#include "ITransport.h"
#include <QTcpSocket>
#include <memory>

namespace rcms {

/**
 * @brief TCP-Serial transport for RS-485 over network
 *
 * Connects to a serial-server that bridges TCP to RS-485
 */
class TcpSerialTransport : public ITransport {
public:
    /**
     * @brief Constructor
     * @param host Server hostname or IP
     * @param port TCP port number
     * @param connectTimeoutMs Connection timeout in milliseconds
     */
    TcpSerialTransport(const QString& host, uint16_t port, int connectTimeoutMs = 5000);

    ~TcpSerialTransport() override;

    bool open() override;
    void close() override;
    bool isOpen() const override;

    qint64 write(const QByteArray& data) override;
    QByteArray read(int maxSize, int timeoutMs = 1000) override;
    void flush() override;

    QString lastError() const override { return m_lastError; }
    QString transportType() const override { return "TCP-Serial"; }
    QString connectionString() const override { return QString("%1:%2").arg(m_host).arg(m_port); }

    // TCP-specific settings
    QString host() const { return m_host; }
    uint16_t port() const { return m_port; }
    void setHost(const QString& host) { m_host = host; }
    void setPort(uint16_t port) { m_port = port; }

private:
    QString m_host;
    uint16_t m_port;
    int m_connectTimeoutMs;
    std::unique_ptr<QTcpSocket> m_socket;
    QString m_lastError;
};

} // namespace rcms
