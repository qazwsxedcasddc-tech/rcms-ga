#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace rcms {

/**
 * @brief Device configuration
 */
struct DeviceConfig {
    std::string name;
    std::string type;           // "fazan19", "rsp1000", etc.
    uint8_t modbusAddress = 1;
    std::string portName;
    int baudRate = 9600;
    int pollingInterval = 1000; // ms
};

/**
 * @brief Application configuration manager
 *
 * Handles loading/saving JSON configuration files
 */
class ConfigManager {
public:
    ConfigManager() = default;

    /**
     * @brief Load configuration from JSON file
     * @param filename Path to config file
     * @return true if successful
     */
    bool load(const std::string& filename);

    /**
     * @brief Save configuration to JSON file
     * @param filename Path to config file
     * @return true if successful
     */
    bool save(const std::string& filename) const;

    /**
     * @brief Get configured devices
     */
    const std::vector<DeviceConfig>& devices() const { return m_devices; }

    /**
     * @brief Add device configuration
     */
    void addDevice(const DeviceConfig& device);

    /**
     * @brief Remove device configuration
     */
    void removeDevice(size_t index);

    /**
     * @brief Get global polling interval (ms)
     */
    int pollingInterval() const { return m_pollingInterval; }

    /**
     * @brief Set global polling interval (ms)
     */
    void setPollingInterval(int ms) { m_pollingInterval = ms; }

private:
    std::vector<DeviceConfig> m_devices;
    int m_pollingInterval = 1000;
};

} // namespace rcms
