# План разработки RCMS-GA

**Версия:** 2.0
**Дата:** 2026-01-11
**Язык:** C++17
**GUI:** Qt 5.15+
**Платформа:** Linux (Debian, РЕД ОС, Astra Linux)

---

## Обзор

RCMS-GA — универсальная система управления радиостанциями гражданской авиации РФ.

**Ключевое решение:** C++ выбран для:
- Минимальных требований к железу (работает на старых ПК)
- Один бинарник без зависимостей
- Перспектива сертификации
- Серьёзность для госзаказчика

---

## Архитектура

```
┌─────────────────────────────────────────────────────────────────┐
│                    GUI Layer (Qt Widgets)                        │
│  ┌───────────┐ ┌───────────┐ ┌─────────────┐ ┌───────────────┐  │
│  │DeviceTree │ │StatusPanel│ │ControlPanel │ │ EventLogWidget│  │
│  └───────────┘ └───────────┘ └─────────────┘ └───────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                      Core Layer                                  │
│  ┌─────────────┐ ┌────────────┐ ┌─────────────┐ ┌────────────┐  │
│  │DeviceManager│ │AlarmManager│ │ConfigManager│ │   Logger   │  │
│  └─────────────┘ └────────────┘ └─────────────┘ └────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                    Protocol Layer                                │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │  IRadioDevice   │  │ Fazan19Device   │  │ [RSP1000Device] │  │
│  │   (interface)   │  │ (implementation)│  │    (future)     │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                  Communication Layer                             │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │   ModbusRTU     │  │   SerialPort    │  │     CRC16       │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Структура проекта

```
rcms-ga/
├── CMakeLists.txt
├── README.md
├── LICENSE                      # GPL-3.0
│
├── docs/
│   ├── SRS.md                   # Техническое задание
│   ├── PLAN.md                  # Этот файл
│   ├── PROTOCOL_FAZAN19.md      # Спецификация протокола
│   └── MARKET_RESEARCH.md       # Анализ рынка
│
├── src/
│   ├── main.cpp                 # Точка входа
│   │
│   ├── core/
│   │   ├── DeviceManager.h
│   │   ├── DeviceManager.cpp
│   │   ├── AlarmManager.h
│   │   ├── AlarmManager.cpp
│   │   ├── ConfigManager.h
│   │   ├── ConfigManager.cpp
│   │   ├── Logger.h
│   │   └── Logger.cpp
│   │
│   ├── protocol/
│   │   ├── IRadioDevice.h       # Абстрактный интерфейс
│   │   ├── Fazan19Device.h
│   │   ├── Fazan19Device.cpp
│   │   ├── Fazan19Registers.h   # Константы регистров
│   │   ├── ModbusRTU.h
│   │   └── ModbusRTU.cpp
│   │
│   ├── comm/
│   │   ├── SerialPort.h
│   │   ├── SerialPort.cpp
│   │   ├── CRC16.h
│   │   └── CRC16.cpp
│   │
│   ├── gui/
│   │   ├── MainWindow.h
│   │   ├── MainWindow.cpp
│   │   ├── MainWindow.ui
│   │   ├── DeviceTreeWidget.h
│   │   ├── DeviceTreeWidget.cpp
│   │   ├── StatusPanel.h
│   │   ├── StatusPanel.cpp
│   │   ├── ControlPanel.h
│   │   ├── ControlPanel.cpp
│   │   ├── EventLogWidget.h
│   │   ├── EventLogWidget.cpp
│   │   ├── SettingsDialog.h
│   │   ├── SettingsDialog.cpp
│   │   └── SettingsDialog.ui
│   │
│   └── resources/
│       ├── resources.qrc
│       ├── icons/
│       │   ├── app.png
│       │   ├── device_ok.png
│       │   ├── device_error.png
│       │   └── device_offline.png
│       └── sounds/
│           └── alarm.wav
│
├── tests/
│   ├── CMakeLists.txt
│   ├── test_crc16.cpp
│   ├── test_modbus.cpp
│   ├── test_fazan19.cpp
│   └── test_frequency.cpp
│
└── config/
    └── default.json             # Пример конфигурации
```

---

## Зависимости

| Библиотека | Версия | Назначение | Установка (Debian) |
|------------|--------|------------|-------------------|
| Qt | 5.15+ | GUI, Serial | `apt install qtbase5-dev libqt5serialport5-dev` |
| spdlog | 1.9+ | Логирование | `apt install libspdlog-dev` |
| nlohmann-json | 3.9+ | JSON конфиг | `apt install nlohmann-json3-dev` |
| SQLite3 | 3.31+ | БД журнала | `apt install libsqlite3-dev` |
| GTest | 1.10+ | Тесты | `apt install libgtest-dev` |

**Примечание:** Qt5 включает QSerialPort — отдельная libserialport не нужна!

---

## Этапы разработки

### Этап 1: Фундамент
**Срок: Неделя 1-2**

| Задача | Файлы | Статус |
|--------|-------|--------|
| CMake + Qt проект | CMakeLists.txt | ⬜ |
| CRC-16 Modbus | CRC16.h/cpp, test_crc16.cpp | ⬜ |
| SerialPort обёртка | SerialPort.h/cpp | ⬜ |
| ModbusRTU базовый | ModbusRTU.h/cpp, test_modbus.cpp | ⬜ |
| Логирование | Logger.h/cpp | ⬜ |

**Результат:** Можно отправить Modbus-запрос и получить ответ.

### Этап 2: Протокол Фазан-19
**Срок: Неделя 3-4**

| Задача | Файлы | Статус |
|--------|-------|--------|
| IRadioDevice интерфейс | IRadioDevice.h | ⬜ |
| Fazan19Registers константы | Fazan19Registers.h | ⬜ |
| Чтение регистров | Fazan19Device.cpp | ⬜ |
| Парсинг частоты | Fazan19Device.cpp | ⬜ |
| Запись частоты | Fazan19Device.cpp | ⬜ |
| Управление ПШ | Fazan19Device.cpp | ⬜ |
| Парсинг статуса | Fazan19Device.cpp | ⬜ |
| Парсинг ошибок | Fazan19Device.cpp | ⬜ |
| Unit-тесты | test_fazan19.cpp | ⬜ |

**Результат:** Полная работа с Фазан-19 через консоль.

### Этап 3: Core Logic
**Срок: Неделя 5**

| Задача | Файлы | Статус |
|--------|-------|--------|
| DeviceManager polling | DeviceManager.h/cpp | ⬜ |
| AlarmManager события | AlarmManager.h/cpp | ⬜ |
| ConfigManager JSON | ConfigManager.h/cpp | ⬜ |
| Logger SQLite | Logger.h/cpp | ⬜ |

**Результат:** Автоматический опрос, хранение событий.

### Этап 4: GUI
**Срок: Неделя 6-7**

| Задача | Файлы | Статус |
|--------|-------|--------|
| MainWindow каркас | MainWindow.* | ⬜ |
| DeviceTreeWidget | DeviceTreeWidget.* | ⬜ |
| StatusPanel | StatusPanel.* | ⬜ |
| ControlPanel | ControlPanel.* | ⬜ |
| EventLogWidget | EventLogWidget.* | ⬜ |
| SettingsDialog | SettingsDialog.* | ⬜ |
| Иконки и звуки | resources/ | ⬜ |

**Результат:** Полноценный GUI как на скриншоте NIK_371.

### Этап 5: Интеграция и тесты
**Срок: Неделя 8**

| Задача | Статус |
|--------|--------|
| Тест с реальным Фазан-19 | ⬜ |
| Тест на Debian 11 | ⬜ |
| Тест на РЕД ОС | ⬜ |
| Исправление багов | ⬜ |
| Оптимизация | ⬜ |

### Этап 6: Релиз MVP
**Срок: Неделя 9**

| Задача | Статус |
|--------|--------|
| Документация пользователя | ⬜ |
| Сборка статического бинарника | ⬜ |
| Сборка AppImage | ⬜ |
| README с инструкцией | ⬜ |
| GitHub Release v1.0.0 | ⬜ |

---

## Ключевые классы

### IRadioDevice (интерфейс)

```cpp
class IRadioDevice {
public:
    virtual ~IRadioDevice() = default;

    // Идентификация
    virtual QString deviceType() const = 0;
    virtual QString deviceId() const = 0;
    virtual uint8_t modbusAddress() const = 0;

    // Подключение
    virtual bool open(const QString& portName, int baudRate = 9600) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    // Чтение состояния
    virtual bool readStatus(DeviceStatus& status) = 0;
    virtual bool readAlarms(QVector<AlarmInfo>& alarms) = 0;

    // Управление
    virtual bool setFrequency(double freqMHz) = 0;
    virtual bool setSquelch(bool enabled, int level = 5) = 0;
    virtual bool setPTT(bool enabled) = 0;

    // Диагностика
    virtual bool runSelfTest() = 0;
};
```

### DeviceStatus (структура)

```cpp
struct DeviceStatus {
    bool online = false;
    double frequencyMHz = 0.0;
    bool isTransmitting = false;
    bool squelchEnabled = false;
    int squelchLevel = 0;
    int signalLevel = 0;
    double voltage24V = 0.0;
    double batteryVoltage = 0.0;
    double temperature = 0.0;
    uint32_t operatingHours = 0;
    QString mode;           // "МУ" или "ДУ"
    QString workMode;       // "ТЛФ" или "ДАН"
    QString lineType;       // "2-х" или "4-х"
    QDateTime lastUpdate;
    QVector<uint16_t> errorCodes;
};
```

---

## Формулы Фазан-19

### Кодирование частоты

```cpp
// MHz -> регистр FrRS
uint16_t encodeFrequency(double freqMHz, uint8_t kf = 0) {
    int32_t diffHz = static_cast<int32_t>((freqMHz - 100.0) * 1000000.0);
    uint16_t f12 = static_cast<uint16_t>(std::round(diffHz / 8333.33333));
    return (static_cast<uint16_t>(kf) << 13) | (f12 & 0x1FFF);
}

// Регистр FrRS -> MHz
double decodeFrequency(uint16_t frrs) {
    uint16_t f12 = frrs & 0x1FFF;
    return 100.0 + (f12 * 8333.33333) / 1000000.0;
}
```

### CRC-16 Modbus

```cpp
uint16_t crc16Modbus(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}
```

---

## Сборка

### Debian / Ubuntu

```bash
# Установка зависимостей
sudo apt install build-essential cmake \
    qtbase5-dev libqt5serialport5-dev \
    libspdlog-dev nlohmann-json3-dev \
    libsqlite3-dev libgtest-dev

# Сборка
mkdir build && cd build
cmake ..
make -j$(nproc)

# Запуск
./rcms-ga
```

### РЕД ОС / Astra Linux

```bash
# Зависимости (могут отличаться названия пакетов)
sudo dnf install cmake gcc-c++ \
    qt5-qtbase-devel qt5-qtserialport-devel \
    spdlog-devel json-devel sqlite-devel gtest-devel

# Сборка аналогично
```

### AppImage (для универсальности)

```bash
# После сборки
linuxdeployqt rcms-ga -appimage
```

---

## Тестирование

### Unit-тесты

```bash
cd build
ctest --output-on-failure
```

### Тест с реальным оборудованием

1. Подключить Фазан-19 через USB-RS485
2. Определить порт: `dmesg | grep tty`
3. Запустить: `./rcms-ga --port /dev/ttyUSB0`
4. Проверить:
   - [ ] Устройство обнаружено
   - [ ] Параметры читаются корректно
   - [ ] Частота меняется
   - [ ] ПШ управляется

---

## Риски и митигация

| Риск | Вероятность | Митигация |
|------|-------------|-----------|
| Нет доступа к железу | Средняя | Эмулятор Modbus для тестов |
| Баги в протоколе | Низкая | Референс drudrum проверен |
| Qt не соберётся на РЕД ОС | Низкая | Статическая линковка |

---

## Контакты и ссылки

- **Проект:** https://github.com/qazwsxedcasddc-tech/rcms-ga
- **Референс:** https://github.com/drudrum/fazan_19P50
- **Форум ЭРТОС:** https://ertos.ru
- **Автор референса:** Aleksey Druzhinin (druzhinin@ans.aero)
