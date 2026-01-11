#pragma once

#include <string>
#include <spdlog/spdlog.h>

namespace rcms {

/**
 * @brief Application logger wrapper
 *
 * Provides unified logging interface using spdlog
 */
class Logger {
public:
    /**
     * @brief Initialize logging system
     * @param logFile Optional log file path
     */
    static void init(const std::string& logFile = "rcms-ga.log");

    /**
     * @brief Shutdown logging system
     */
    static void shutdown();

    /**
     * @brief Log debug message
     */
    template<typename... Args>
    static void debug(const std::string& fmt, Args&&... args) {
        spdlog::debug(fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log info message
     */
    template<typename... Args>
    static void info(const std::string& fmt, Args&&... args) {
        spdlog::info(fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log warning message
     */
    template<typename... Args>
    static void warn(const std::string& fmt, Args&&... args) {
        spdlog::warn(fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log error message
     */
    template<typename... Args>
    static void error(const std::string& fmt, Args&&... args) {
        spdlog::error(fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log critical message
     */
    template<typename... Args>
    static void critical(const std::string& fmt, Args&&... args) {
        spdlog::critical(fmt, std::forward<Args>(args)...);
    }
};

} // namespace rcms
