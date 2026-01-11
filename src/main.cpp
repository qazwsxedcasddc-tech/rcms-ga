/**
 * @file main.cpp
 * @brief Entry point for RCMS-GA application
 *
 * RCMS-GA - Radio Control and Monitoring System for General Aviation
 * Universal software for controlling Russian civil aviation radio stations
 *
 * @author RCMS-GA Team
 * @date 2026
 * @license GPL-3.0
 */

#include <QApplication>
#include "gui/MainWindow.h"
#include "core/Logger.h"
#include "core/ConfigManager.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("RCMS-GA");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("RCMS-GA");

    // Initialize logger
    rcms::Logger::init();
    rcms::Logger::info("RCMS-GA starting...");

    // Load configuration
    rcms::ConfigManager config;
    if (!config.load("config/default.json")) {
        rcms::Logger::warn("Could not load config, using defaults");
    }

    // Create and show main window
    rcms::MainWindow mainWindow;
    mainWindow.show();

    rcms::Logger::info("RCMS-GA initialized successfully");

    int result = app.exec();

    rcms::Logger::info("RCMS-GA shutting down");
    return result;
}
