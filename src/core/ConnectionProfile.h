#pragma once

#include <QString>
#include <memory>
#include "comm/ITransport.h"

namespace rcms {

/**
 * @brief Connection profile type
 */
enum class ConnectionType {
    COM,        // Direct COM/USB-RS485
    TcpSerial   // TCP to Serial bridge
};

/**
 * @brief Connection profile for device communication
 *
 * Encapsulates COM or TCP-Serial connection parameters
 */
struct ConnectionProfile {
    QString id;                         // Unique profile ID
    QString name;                       // Display name
    ConnectionType type = ConnectionType::COM;

    // COM settings
    QString comPort;                    // e.g., "COM3", "/dev/ttyUSB0"
    int baudRate = 9600;
    int dataBits = 8;
    int stopBits = 1;
    char parity = 'N';                  // N/E/O

    // TCP-Serial settings
    QString tcpHost;                    // e.g., "192.168.1.100"
    uint16_t tcpPort = 4001;

    // Common settings
    int responseTimeoutMs = 500;        // Response timeout
    int retryCount = 3;                 // Number of retries
    int pollingIntervalMs = 1000;       // Polling interval

    // Runtime state (not serialized)
    bool connected = false;
    QString lastError;
    int successCount = 0;
    int errorCount = 0;

    /**
     * @brief Create transport instance based on profile type
     */
    std::unique_ptr<ITransport> createTransport() const;

    /**
     * @brief Get connection string for display
     */
    QString connectionString() const {
        if (type == ConnectionType::COM) {
            return QString("%1 @ %2 baud").arg(comPort).arg(baudRate);
        } else {
            return QString("%1:%2").arg(tcpHost).arg(tcpPort);
        }
    }
};

} // namespace rcms
