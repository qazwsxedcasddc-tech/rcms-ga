#pragma once

#include <QByteArray>
#include <QString>
#include <cstdint>

namespace rcms {

/**
 * @brief Abstract transport interface for RS-485 communication
 *
 * Supports both COM port and TCP-Serial connections
 */
class ITransport {
public:
    virtual ~ITransport() = default;

    /**
     * @brief Open the transport connection
     * @return true if successful
     */
    virtual bool open() = 0;

    /**
     * @brief Close the connection
     */
    virtual void close() = 0;

    /**
     * @brief Check if connection is open
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Write data to transport
     * @param data Data to write
     * @return Number of bytes written, or -1 on error
     */
    virtual qint64 write(const QByteArray& data) = 0;

    /**
     * @brief Read data from transport
     * @param maxSize Maximum bytes to read
     * @param timeoutMs Timeout in milliseconds
     * @return Data read (empty on timeout/error)
     */
    virtual QByteArray read(int maxSize, int timeoutMs = 1000) = 0;

    /**
     * @brief Flush any pending data
     */
    virtual void flush() = 0;

    /**
     * @brief Get last error message
     */
    virtual QString lastError() const = 0;

    /**
     * @brief Get transport type name
     */
    virtual QString transportType() const = 0;

    /**
     * @brief Get connection description (port name or host:port)
     */
    virtual QString connectionString() const = 0;
};

} // namespace rcms
