cmake_minimum_required(VERSION 3.16)
project(rcms-ga VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Опции сборки
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_STATIC "Build static binary" OFF)

# Поиск Qt
find_package(Qt5 REQUIRED COMPONENTS
    Widgets
    SerialPort
    Sql
)

# Поиск других зависимостей
find_package(spdlog REQUIRED)
find_package(nlohmann_json 3.9 REQUIRED)
find_package(SQLite3 REQUIRED)

# Исходники
set(SOURCES
    src/main.cpp

    # Core
    src/core/DeviceManager.cpp
    src/core/AlarmManager.cpp
    src/core/ConfigManager.cpp
    src/core/Logger.cpp

    # Protocol
    src/protocol/ModbusRTU.cpp
    src/protocol/Fazan19Device.cpp

    # Communication
    src/comm/SerialPort.cpp
    src/comm/CRC16.cpp

    # GUI
    src/gui/MainWindow.cpp
    src/gui/DeviceTreeWidget.cpp
    src/gui/StatusPanel.cpp
    src/gui/ControlPanel.cpp
    src/gui/EventLogWidget.cpp
    src/gui/SettingsDialog.cpp
)

set(HEADERS
    # Core
    src/core/DeviceManager.h
    src/core/AlarmManager.h
    src/core/ConfigManager.h
    src/core/Logger.h

    # Protocol
    src/protocol/IRadioDevice.h
    src/protocol/ModbusRTU.h
    src/protocol/Fazan19Device.h
    src/protocol/Fazan19Registers.h

    # Communication
    src/comm/SerialPort.h
    src/comm/CRC16.h

    # GUI
    src/gui/MainWindow.h
    src/gui/DeviceTreeWidget.h
    src/gui/StatusPanel.h
    src/gui/ControlPanel.h
    src/gui/EventLogWidget.h
    src/gui/SettingsDialog.h
)

set(UI_FILES
    src/gui/MainWindow.ui
    src/gui/SettingsDialog.ui
)

set(RESOURCES
    src/resources/resources.qrc
)

# Основной исполняемый файл
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::SerialPort
    Qt5::Sql
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
)

# Статическая сборка (опционально)
if(BUILD_STATIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "-static"
    )
endif()

# Unit-тесты
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    # Эмулятор как библиотека для тестов
    add_library(fazan19_emulator STATIC
        tests/emulator/Fazan19Emulator.cpp
        tests/emulator/Fazan19Emulator.h
    )
    target_include_directories(fazan19_emulator PUBLIC
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_SOURCE_DIR}/src
    )

    # CRC тесты
    add_executable(test_crc16 tests/test_crc16.cpp src/comm/CRC16.cpp)
    target_link_libraries(test_crc16 GTest::GTest GTest::Main)
    target_include_directories(test_crc16 PRIVATE ${CMAKE_SOURCE_DIR}/src)
    add_test(NAME test_crc16 COMMAND test_crc16)

    # Тесты частотного кодирования
    add_executable(test_frequency tests/test_frequency.cpp)
    target_link_libraries(test_frequency GTest::GTest GTest::Main)
    target_include_directories(test_frequency PRIVATE ${CMAKE_SOURCE_DIR}/src)
    add_test(NAME test_frequency COMMAND test_frequency)

    # Тесты эмулятора
    add_executable(test_emulator tests/test_emulator.cpp)
    target_link_libraries(test_emulator GTest::GTest GTest::Main fazan19_emulator)
    target_include_directories(test_emulator PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tests)
    add_test(NAME test_emulator COMMAND test_emulator)

    # Интеграционные тесты протокола
    add_executable(test_protocol tests/test_protocol.cpp src/comm/CRC16.cpp)
    target_link_libraries(test_protocol GTest::GTest GTest::Main fazan19_emulator)
    target_include_directories(test_protocol PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/tests)
    add_test(NAME test_protocol COMMAND test_protocol)
endif()

# Установка
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES config/default.json DESTINATION etc/rcms-ga)
