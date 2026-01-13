#include "ConnectionProfile.h"
#include "comm/ComTransport.h"
#include "comm/TcpSerialTransport.h"

namespace rcms {

std::unique_ptr<ITransport> ConnectionProfile::createTransport() const {
    if (type == ConnectionType::COM) {
        QSerialPort::Parity qParity = QSerialPort::NoParity;
        if (parity == 'E') qParity = QSerialPort::EvenParity;
        else if (parity == 'O') qParity = QSerialPort::OddParity;

        QSerialPort::StopBits qStopBits = QSerialPort::OneStop;
        if (stopBits == 2) qStopBits = QSerialPort::TwoStop;

        return std::make_unique<ComTransport>(
            comPort,
            baudRate,
            QSerialPort::Data8,
            qParity,
            qStopBits
        );
    } else {
        return std::make_unique<TcpSerialTransport>(tcpHost, tcpPort);
    }
}

} // namespace rcms
