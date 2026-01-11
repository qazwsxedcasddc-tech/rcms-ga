#include "Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace rcms {

void Logger::init(const std::string& logFile) {
    try {
        // Create console sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);

        // Create file sink (5 MB max, 3 rotated files)
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFile, 5 * 1024 * 1024, 3);
        file_sink->set_level(spdlog::level::debug);

        // Create logger with both sinks
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        auto logger = std::make_shared<spdlog::logger>("rcms", sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::debug);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        spdlog::set_default_logger(logger);
        spdlog::flush_every(std::chrono::seconds(3));

    } catch (const spdlog::spdlog_ex& ex) {
        // Fallback to console only
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    }
}

void Logger::shutdown() {
    spdlog::shutdown();
}

} // namespace rcms
