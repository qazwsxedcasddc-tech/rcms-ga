#include "ComTransport.h"

namespace rcms {

ComTransport::ComTransport(const QString& portName,
                           int baudRate,
                           QSerialPort::DataBits dataBits,
                           QSerialPort::Parity parity,
                           QSerialPort::StopBits stopBits)
    : m_portName(portName)
    , m_baudRate(baudRate)
    , m_dataBits(dataBits)
    , m_parity(parity)
    , m_stopBits(stopBits)
    , m_port(std::make_unique<QSerialPort>())
{
}

ComTransport::~ComTransport() {
    close();
}

bool ComTransport::open() {
    if (m_port->isOpen()) {
        return true;
    }

    m_port->setPortName(m_portName);
    m_port->setBaudRate(m_baudRate);
    m_port->setDataBits(m_dataBits);
    m_port->setParity(m_parity);
    m_port->setStopBits(m_stopBits);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_port->open(QIODevice::ReadWrite)) {
        m_lastError = m_port->errorString();
        return false;
    }

    m_port->clear();
    return true;
}

void ComTransport::close() {
    if (m_port->isOpen()) {
        m_port->close();
    }
}

bool ComTransport::isOpen() const {
    return m_port->isOpen();
}

qint64 ComTransport::write(const QByteArray& data) {
    if (!m_port->isOpen()) {
        m_lastError = "Port not open";
        return -1;
    }

    qint64 written = m_port->write(data);
    if (written < 0) {
        m_lastError = m_port->errorString();
    }
    return written;
}

QByteArray ComTransport::read(int maxSize, int timeoutMs) {
    if (!m_port->isOpen()) {
        m_lastError = "Port not open";
        return QByteArray();
    }

    QByteArray result;
    int elapsed = 0;
    const int pollInterval = 10;

    while (result.size() < maxSize && elapsed < timeoutMs) {
        if (m_port->waitForReadyRead(pollInterval)) {
            result.append(m_port->read(maxSize - result.size()));
        }
        elapsed += pollInterval;

        // Early exit if we got expected data
        if (result.size() >= maxSize) {
            break;
        }
    }

    return result;
}

void ComTransport::flush() {
    if (m_port->isOpen()) {
        m_port->flush();
        m_port->clear();
    }
}

void ComTransport::setBaudRate(int baudRate) {
    m_baudRate = baudRate;
    if (m_port->isOpen()) {
        m_port->setBaudRate(baudRate);
    }
}

} // namespace rcms
