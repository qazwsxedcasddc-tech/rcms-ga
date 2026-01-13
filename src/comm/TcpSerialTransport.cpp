#include "TcpSerialTransport.h"
#include <QAbstractSocket>

namespace rcms {

TcpSerialTransport::TcpSerialTransport(const QString& host, uint16_t port, int connectTimeoutMs)
    : m_host(host)
    , m_port(port)
    , m_connectTimeoutMs(connectTimeoutMs)
    , m_socket(std::make_unique<QTcpSocket>())
{
}

TcpSerialTransport::~TcpSerialTransport() {
    close();
}

bool TcpSerialTransport::open() {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    m_socket->connectToHost(m_host, m_port);

    if (!m_socket->waitForConnected(m_connectTimeoutMs)) {
        m_lastError = m_socket->errorString();
        return false;
    }

    return true;
}

void TcpSerialTransport::close() {
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }
    }
}

bool TcpSerialTransport::isOpen() const {
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

qint64 TcpSerialTransport::write(const QByteArray& data) {
    if (!isOpen()) {
        m_lastError = "Socket not connected";
        return -1;
    }

    qint64 written = m_socket->write(data);
    if (written < 0) {
        m_lastError = m_socket->errorString();
        return -1;
    }

    if (!m_socket->waitForBytesWritten(1000)) {
        m_lastError = "Write timeout";
        return -1;
    }

    return written;
}

QByteArray TcpSerialTransport::read(int maxSize, int timeoutMs) {
    if (!isOpen()) {
        m_lastError = "Socket not connected";
        return QByteArray();
    }

    QByteArray result;
    int elapsed = 0;
    const int pollInterval = 10;

    while (result.size() < maxSize && elapsed < timeoutMs) {
        if (m_socket->waitForReadyRead(pollInterval)) {
            result.append(m_socket->read(maxSize - result.size()));
        }
        elapsed += pollInterval;

        if (result.size() >= maxSize) {
            break;
        }
    }

    return result;
}

void TcpSerialTransport::flush() {
    if (isOpen()) {
        m_socket->flush();
    }
}

} // namespace rcms
