# План разработки: RCMS-GA

## Обзор проекта

**Название**: RCMS-GA (Radio Control and Monitoring System for General Aviation)
**Цель**: Универсальное ПО для дистанционного управления и мониторинга радиопередатчиков/радиоприемников гражданской авиации РФ по интерфейсу RS-485
**Язык**: C++17
**GUI**: Qt 5.15 / Qt 6
**Платформа**: Linux (Debian, РЕД ОС)
**Приоритетное оборудование**: Фазан-19 П5 (Электроприбор), позже РС П1000

---

## Архитектура системы

```
┌──────────────────────────────────────────────────────────────┐
│                        GUI Layer (Qt)                         │
│  ┌────────────┐ ┌────────────┐ ┌────────────┐ ┌────────────┐ │
│  │ DeviceTree │ │ StatusPanel│ │ ControlPanel│ │ EventLog   │ │
│  └────────────┘ └────────────┘ └────────────┘ └────────────┘ │
├──────────────────────────────────────────────────────────────┤
│                    Business Logic Layer                       │
│  ┌───────────────┐ ┌───────────────┐ ┌───────────────────┐   │
│  │ RadioManager  │ │ AlarmManager  │ │ ConfigManager     │   │
│  │ (управление   │ │ (события,     │ │ (JSON конфиг,     │   │
│  │  устройствами)│ │  аларм)       │ │  профили)         │   │
│  └───────────────┘ └───────────────┘ └───────────────────┘   │
├──────────────────────────────────────────────────────────────┤
│               Protocol Abstraction Layer (PAL)                │
│  ┌──────────────────────────────────────────────────────┐    │
│  │              IRadioProtocol (абстрактный класс)      │    │
│  ├──────────────────────────────────────────────────────┤    │
│  │  ┌────────────────┐  ┌────────────────┐  ┌────────┐  │    │
│  │  │ Fazan19Protocol│  │ RSP1000Protocol│  │ ...    │  │    │
│  │  │ (драйвер Фазан)│  │ (драйвер П1000)│  │        │  │    │
│  │  └────────────────┘  └────────────────┘  └────────┘  │    │
│  └──────────────────────────────────────────────────────┘    │
├──────────────────────────────────────────────────────────────┤
│                   Communication Layer                         │
│  ┌──────────────────────────────────────────────────────┐    │
│  │         ModbusRTU (libmodbus / custom)               │    │
│  │  - CRC-16 Modbus                                     │    │
│  │  - Таймаут 2000 мс                                   │    │
│  └──────────────────────────────────────────────────────┘    │
│  ┌──────────────────────────────────────────────────────┐    │
│  │            RS485Port (libserialport / termios)       │    │
│  │  - 9600 бод, 8N1                                     │    │
│  │  - TX Enable управление (RTS/DTR)                    │    │
│  └──────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────┘
```

---

## Структура проекта

```
rcms-ga/
├── CMakeLists.txt
├── README.md
├── LICENSE (GPL-3.0)
├── config/
│   └── default_config.json
│
├── src/
│   ├── main.cpp
│   │
│   ├── core/
│   │   ├── RadioManager.h/.cpp
│   │   ├── AlarmManager.h/.cpp
│   │   ├── ConfigManager.h/.cpp
│   │   └── EventLogger.h/.cpp
│   │
│   ├── protocol/
│   │   ├── IRadioProtocol.h
│   │   ├── RadioTypes.h
│   │   ├── ModbusRTU.h/.cpp
│   │   ├── fazan19/
│   │   │   ├── Fazan19Protocol.h/.cpp
│   │   │   ├── Fazan19Registers.h
│   │   │   └── Fazan19Parser.h/.cpp
│   │   └── rsp1000/ (в будущем)
│   │
│   ├── comm/
│   │   ├── RS485Port.h/.cpp
│   │   └── CRC16Modbus.h/.cpp
│   │
│   └── gui/
│       ├── MainWindow.h/.cpp/.ui
│       ├── DeviceTreeWidget.h/.cpp
│       ├── StatusPanel.h/.cpp
│       ├── ControlPanel.h/.cpp
│       └── EventLogWidget.h/.cpp
│
├── tests/
│   ├── test_fazan19_protocol.cpp
│   ├── test_crc16.cpp
│   └── test_frequency_calc.cpp
│
└── docs/
    ├── PLAN.md
    ├── PROTOCOL_FAZAN19.md
    └── MARKET_RESEARCH.md
```

---

## Этапы разработки

### Этап 1: Инфраструктура
- [ ] Настройка CMake проекта с Qt
- [ ] Базовая структура директорий
- [ ] CRC-16 Modbus (тесты с известными значениями)
- [ ] Класс RS485Port (libserialport)
- [ ] Базовые типы данных
- [ ] Логирование (spdlog)

### Этап 2: Протокол Фазан-19 (ГОТОВА СПЕЦИФИКАЦИЯ!)
- [ ] ModbusRTU базовый класс
- [ ] Fazan19Protocol:
  - [ ] Чтение 28 регистров (функция 0x03)
  - [ ] Запись регистра (функция 0x06/0x10)
  - [ ] Чтение ID (функция 0x11)
  - [ ] Расчёт частоты (формула f12)
  - [ ] Парсинг регистров MR1, MR2, FrRS
  - [ ] Парсинг АЦП (AD0-AD7)
  - [ ] Парсинг ошибок (DV1-DV4)

### Этап 3: Core Logic
- [ ] RadioManager (polling, состояния)
- [ ] AlarmManager (события, уведомления)
- [ ] ConfigManager (JSON)
- [ ] EventLogger (SQLite)

### Этап 4: GUI
- [ ] MainWindow
- [ ] DeviceTreeWidget
- [ ] StatusPanel (как на скриншоте NIK_371)
- [ ] ControlPanel (частота, ПШ, PTT)
- [ ] EventLogWidget

### Этап 5: Тестирование
- [ ] Unit-тесты
- [ ] Тест с реальным оборудованием
- [ ] Тест на РЕД ОС

### Этап 6: Расширение
- [ ] Драйвер РС П1000
- [ ] SNMP интеграция
- [ ] Zabbix плагин

---

## Ключевая информация протокола Фазан-19

### Modbus RTU параметры
- CRC: CRC-16 Modbus (LE)
- Таймаут: 2000 мс
- Шаг частоты: 8333.33333 Гц
- База: 100 МГц

### Карта регистров
| Адрес | Имя | Описание |
|-------|-----|----------|
| 0x00-0x01 | CW1/CW2 | Наработка |
| 0x02 | Cntr | Запросы |
| 0x03 | MR1 | Режимы (бит 7 = ПШ) |
| 0x04 | MR2 | Доп. режимы |
| 0x05 | FrRS | Частота + KF |
| 0x09-0x11 | AD0-AD7 | АЦП |
| 0x16-0x19 | DV1-DV4 | Ошибки |

### Формула частоты
```cpp
// Запись
uint16_t encodeFrequency(double freq_mhz, uint8_t kf) {
    int32_t diff = (freq_mhz - 100.0) * 1000000;
    uint16_t f12 = round(diff / 8333.33333);
    return (kf << 13) | (f12 & 0x1FFF);
}

// Чтение
double decodeFrequency(uint16_t frrs) {
    uint16_t f12 = frrs & 0x1FFF;
    return 100.0 + (f12 * 8333.33333) / 1000000.0;
}
```

---

## Зависимости

| Библиотека | Назначение | Debian |
|------------|------------|--------|
| Qt 5.15+ | GUI | `qtbase5-dev` |
| libserialport | Serial | `libserialport-dev` |
| spdlog | Логи | `libspdlog-dev` |
| nlohmann-json | JSON | `nlohmann-json3-dev` |
| SQLite3 | БД | `libsqlite3-dev` |
| GTest | Тесты | `libgtest-dev` |

---

## Референсы

- **Код**: https://github.com/drudrum/fazan_19P50 (JavaScript, GPL-3.0)
- **Форум**: https://ertos.ru/threads/vzaimodejstvie-po-styku-rs-485-s-radiosredstvami-serii-fazan-19.165/
- **Автор референса**: Aleksey Druzhinin (druzhinin@ans.aero)
