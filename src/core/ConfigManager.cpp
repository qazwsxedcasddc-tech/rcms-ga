#include "ConfigManager.h"
#include "Logger.h"
#include <fstream>

namespace rcms {

bool ConfigManager::load(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Logger::error("Cannot open config file: {}", filename);
            return false;
        }

        nlohmann::json config;
        file >> config;

        m_pollingInterval = config.value("pollingInterval", 1000);

        m_devices.clear();
        if (config.contains("devices")) {
            for (const auto& dev : config["devices"]) {
                DeviceConfig dc;
                dc.name = dev.value("name", "Unknown");
                dc.type = dev.value("type", "fazan19");
                dc.modbusAddress = dev.value("address", 1);
                dc.portName = dev.value("port", "");
                dc.baudRate = dev.value("baudRate", 9600);
                dc.pollingInterval = dev.value("pollingInterval", m_pollingInterval);
                m_devices.push_back(dc);
            }
        }

        Logger::info("Loaded config with {} devices", m_devices.size());
        return true;

    } catch (const std::exception& e) {
        Logger::error("Failed to load config: {}", e.what());
        return false;
    }
}

bool ConfigManager::save(const std::string& filename) const {
    try {
        nlohmann::json config;
        config["pollingInterval"] = m_pollingInterval;

        nlohmann::json devices = nlohmann::json::array();
        for (const auto& dev : m_devices) {
            nlohmann::json d;
            d["name"] = dev.name;
            d["type"] = dev.type;
            d["address"] = dev.modbusAddress;
            d["port"] = dev.portName;
            d["baudRate"] = dev.baudRate;
            d["pollingInterval"] = dev.pollingInterval;
            devices.push_back(d);
        }
        config["devices"] = devices;

        std::ofstream file(filename);
        if (!file.is_open()) {
            Logger::error("Cannot write config file: {}", filename);
            return false;
        }

        file << config.dump(4);
        Logger::info("Saved config to {}", filename);
        return true;

    } catch (const std::exception& e) {
        Logger::error("Failed to save config: {}", e.what());
        return false;
    }
}

void ConfigManager::addDevice(const DeviceConfig& device) {
    m_devices.push_back(device);
}

void ConfigManager::removeDevice(size_t index) {
    if (index < m_devices.size()) {
        m_devices.erase(m_devices.begin() + index);
    }
}

} // namespace rcms
