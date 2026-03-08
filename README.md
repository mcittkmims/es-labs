# Embedded Systems Labs — `es-labs`

A collection of five embedded systems laboratory projects for the **Arduino Mega 2560** (ATmega2560), progressing from basic serial I/O through real-time operating system multitasking with FreeRTOS to dual-sensor monitoring with hysteresis-based alerting.

Each lab ships with **working firmware** (PlatformIO), a **Wokwi simulation**, and a **LaTeX report** (compiled PDF included).

---

## Labs at a Glance

| Lab | Title | Scheduling | Key Concepts | Hardware |
|-----|-------|------------|--------------|----------|
| **1.1** | Serial LED Control | `loop()` polling | UART, C STDIO redirection, command parsing | LED |
| **1.2** | LCD + Keypad Lock System | `loop()` event-driven | I2C LCD, 4×4 keypad, 10-state FSM, password management | LCD 1602, membrane keypad, 2 LEDs |
| **2.1** | Button Press Monitor — Bare-Metal | Non-preemptive cooperative scheduler | Deadline-driven task dispatch, button debounce FSM, shared globals | Push button, 3 LEDs |
| **2.2** | Button Press Monitor — FreeRTOS | Preemptive RTOS | Binary semaphore, mutex with priority inheritance, `vTaskDelay` | Push button, 3 LEDs |
| **3.1** | Dual-Sensor Temperature Monitor | FreeRTOS preemptive (3 tasks) | NTC thermistor ADC, DS18B20 OneWire, hysteresis FSM, debounce filtering, I2C LCD | NTC thermistor, DS18B20, LCD 1602, 3 LEDs |

Labs 2.1 and 2.2 implement **the same application** under two different scheduling paradigms, allowing a direct comparison between cooperative and preemptive multitasking. Lab 3.1 extends the FreeRTOS architecture to a production-style sensor pipeline with signal conditioning.

---

## Repository Structure

```
es-labs/
├── labs/                          # PlatformIO project (all firmware)
│   ├── platformio.ini             # 5 build environments (lab1_1 … lab3_1)
│   ├── src/
│   │   └── main.cpp               # Lab selector via #ifdef guards
│   ├── lab/                       # Per-lab entry points
│   │   ├── lab1_1/                #   Serial LED control
│   │   ├── lab1_2/                #   LCD + Keypad lock FSM
│   │   ├── lab2_1/                #   Cooperative scheduler monitor
│   │   ├── lab2_2/                #   FreeRTOS preemptive monitor
│   │   └── lab3_1/                #   Dual-sensor temperature monitoring
│   ├── lib/                       # Reusable libraries
│   │   ├── AnalogTempSensor/      #   NTC thermistor ADC driver (Steinhart-Hart)
│   │   ├── CommandParser/         #   Text → command enum parser
│   │   ├── DigitalTempSensor/     #   DS18B20 OneWire driver (non-blocking)
│   │   ├── KeypadInput/           #   4×4 matrix keypad driver
│   │   ├── LcdDisplay/            #   I2C 16×2 LCD driver
│   │   ├── Led/                   #   Single-pin LED driver
│   │   ├── LockFSM/               #   10-state electronic lock FSM
│   │   ├── StdioSerial/           #   printf/fgets → UART redirection
│   │   ├── TaskScheduler/         #   Bare-metal cooperative scheduler
│   │   └── ThresholdAlert/        #   Hysteresis + debounce threshold FSM
│   └── wokwi/                     # Wokwi simulation configs
│       ├── lab1.1/                #   diagram.json + wokwi.toml
│       ├── lab1.2/
│       ├── lab2.1/
│       ├── lab2.2/
│       └── lab3.1/
├── reports/                       # LaTeX lab reports (with compiled PDFs)
│   ├── lab1.1/
│   ├── lab1.2/
│   ├── lab2.1/
│   ├── lab2.2/
│   └── lab3.1/
└── template/                      # LaTeX report template
```

---

## Getting Started

### Prerequisites

| Tool | Purpose |
|------|---------|
| [VS Code](https://code.visualstudio.com/) | Editor |
| [PlatformIO IDE](https://platformio.org/install/ide?install=vscode) | Build, upload, serial monitor |
| [Wokwi for VS Code](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode) | Circuit simulation |
| [TeX Live](https://www.tug.org/texlive/) *(optional)* | Compile reports from source |

### Clone & Build

```bash
git clone https://github.com/mcittkmims/es-labs.git
cd es-labs/labs
```

Build a specific lab:

```bash
pio run -e lab1_1      # Lab 1.1
pio run -e lab1_2      # Lab 1.2
pio run -e lab2_1      # Lab 2.1
pio run -e lab2_2      # Lab 2.2
pio run -e lab3_1      # Lab 3.1
```

Build all labs at once:

```bash
pio run
```

### Upload to Hardware

```bash
pio run -e lab3_1 --target upload
pio device monitor -e lab3_1
```

### Run in Wokwi Simulator

1. Open the workspace in VS Code.
2. Navigate to the Wokwi config for the desired lab (e.g., `labs/wokwi/lab3.1/wokwi.toml`).
3. Press **F1** → **Wokwi: Start Simulator**.

> Each `wokwi.toml` points to the correct firmware hex: `.pio/build/<env>/firmware.hex`. Build the lab with PlatformIO first.

---

## Lab Details

### Lab 1.1 — Serial LED Control

Controls an LED via text commands (`led on` / `led off`) entered over the serial terminal. Demonstrates UART communication and AVR libc STDIO redirection (`printf` / `fgets` over hardware serial).

**Circuit:** LED + 220Ω resistor on **pin 7**.

**Libraries used:** `Led`, `StdioSerial`, `CommandParser`

---

### Lab 1.2 — LCD + Keypad Lock System

A complete electronic lock system with a **10-state FSM** managing lock, unlock, change-password, and status-query operations. User input comes from a 4×4 membrane keypad; output goes to a 16×2 I2C LCD and dual status LEDs (red = locked, green = unlocked).

**Circuit:** LCD 1602 (I2C `0x27`, SDA/SCL on pins 20/21), 4×4 keypad (rows 22–25, cols 26–29), red LED pin 7, green LED pin 6.

**Libraries used:** `Led`, `StdioSerial`, `LcdDisplay`, `KeypadInput`, `LockFSM`

**External dependencies:** `LiquidCrystal_I2C`, `Keypad`

---

### Lab 2.1 — Button Press Monitor (Cooperative Scheduler)

Three cooperatively-scheduled tasks monitor a button, classify presses as short (<500 ms) or long (≥500 ms), provide visual LED feedback, and print a statistics report every 10 seconds.

| Task | Period | Role |
|------|--------|------|
| Task 1 | 10 ms | Button debounce FSM, green/red LED indicator |
| Task 2 | 50 ms | Statistics accumulation, yellow LED blink sequence |
| Task 3 | 10 s | STDIO report (totals, averages), counter reset |

Shared state is safe without synchronization because the cooperative scheduler guarantees only one task runs at a time.

**Circuit:** Push button pin 7 (INPUT_PULLUP), green LED pin 8, red LED pin 9, yellow LED pin 10.

**Libraries used:** `StdioSerial`, `TaskScheduler`

---

### Lab 2.2 — Button Press Monitor (FreeRTOS Preemptive)

A direct FreeRTOS port of Lab 2.1 with the same functionality but using **preemptive priority-based scheduling**. The transition to preemptive multitasking introduces explicit synchronization:

| Mechanism | Type | Purpose |
|-----------|------|---------|
| `xNewPressSemaphore` | Binary semaphore | Event signal: Task 1 → Task 2 |
| `xSharedDataMutex` | Mutex (priority inheritance) | Protect `g_pressInfo` and `g_stats` |

| Task | Priority | Trigger | Role |
|------|----------|---------|------|
| Measure | 3 (highest) | `vTaskDelay` ~16 ms | Button FSM, LED indicator, semaphore give |
| Stats | 2 | Semaphore take (event-driven) | Update counters, blocking yellow LED blink |
| Report | 1 (lowest) | `vTaskDelay` 10 s | Mutex read/reset, `printf` statistics |

**Key design note:** The feilipu/FreeRTOS library uses the AVR Watchdog Timer (~62 Hz, 16 ms/tick). All `vTaskDelay` calls ensure a minimum of 1 tick to prevent priority starvation.

**Circuit:** Identical to Lab 2.1.

**Libraries used:** `StdioSerial`

**External dependency:** `feilipu/FreeRTOS`

---

### Lab 3.1 — Dual-Sensor Temperature Monitor (FreeRTOS)

A production-style sensor monitoring pipeline that reads two independent temperature sensors, applies hysteresis-based threshold detection with debounce filtering, drives three indicator LEDs, and outputs live data to both an I2C LCD and the serial terminal.

The architecture separates concerns across three FreeRTOS tasks:

| Task | Priority | Period | Role |
|------|----------|--------|------|
| Acquisition | 3 (highest) | 50 ms (`vTaskDelayUntil`) | Read NTC via ADC, DS18B20 via OneWire; write shared data; give semaphore |
| Conditioning | 2 | Event-driven (semaphore) | Run `ThresholdAlert` FSMs for both sensors; update LEDs |
| Display | 1 (lowest) | 500 ms | Update LCD (2-page alternation); print structured STDIO report every 2 s |

Synchronization uses a **binary semaphore** (Acquisition → Conditioning event signal) and a **mutex with priority inheritance** (protecting shared `SensorReadings_t` and `AlertStatus_t` structs).

The `ThresholdAlert` module implements a **4-state FSM** with configurable hysteresis band and debounce count:

```
NORMAL ──> DEBOUNCE_HIGH ──> ALERT_ACTIVE ──> DEBOUNCE_LOW ──> NORMAL
```

Default thresholds: HIGH = 30°C, LOW = 28°C, debounce = 5 consecutive readings (250 ms).

**LED mapping:** green = both normal, red = analog alert, yellow = digital alert. LEDs blink during debounce transitions.

**Circuit:** NTC thermistor (OUT → A0), DS18B20 (DQ → pin 2, 4.7 kΩ pull-up), LCD 1602 I2C (SDA pin 20, SCL pin 21), green LED pin 8, red LED pin 9, yellow LED pin 10.

**Libraries used:** `AnalogTempSensor`, `DigitalTempSensor`, `ThresholdAlert`, `LcdDisplay`, `Led`, `StdioSerial`

**External dependencies:** `feilipu/FreeRTOS`, `OneWire`, `DallasTemperature`, `LiquidCrystal_I2C`

---

## Library Reference

All reusable libraries live in `labs/lib/` and are auto-discovered by PlatformIO.

| Library | Description |
|---------|-------------|
| **AnalogTempSensor** | NTC thermistor ADC driver — Steinhart-Hart Beta equation conversion, single-read API (`readTemperatureC`, `getLastResistance`) |
| **CommandParser** | Parses trimmed serial input into command enums — `parseCommand(input)` |
| **DigitalTempSensor** | DS18B20 OneWire driver — non-blocking conversion (`requestConversion`, `isConversionComplete`, `readLastConversionC`) |
| **KeypadInput** | 4×4 matrix keypad wrapper with 20 ms debounce — `init()`, `getKey()` |
| **LcdDisplay** | I2C LCD 16×2 wrapper — `init()`, `clear()`, `printLine()`, `showTwoLines()` |
| **Led** | GPIO LED driver — `init()`, `turnOn()`, `turnOff()`, `toggle()`, `isOn()` |
| **LockFSM** | 10-state lock FSM — `processKey()`, `isLocked()`, `getDisplay()` |
| **StdioSerial** | Redirects C `stdout`/`stdin` to UART via `fdevopen()` — `stdioSerialInit(baud)` |
| **TaskScheduler** | Deadline-driven cooperative scheduler — `schedulerInit()`, `schedulerRun()` |
| **ThresholdAlert** | 4-state hysteresis + debounce FSM — `update(value)`, `getState()`, `isAlertActive()`, `getDebounceCounter()` |

---

## Reports

Each lab includes a detailed LaTeX report following an academic structure:

1. **Domain Analysis** — Technologies, hardware/software components, case study
2. **Design** — Architecture diagrams (TikZ), FSM state diagrams, flowcharts, CircuiTikZ schematics
3. **Results** — Build output, simulation screenshots, test scenarios, serial logs
4. **Conclusions** — Performance analysis, limitations, improvements

Compiled PDFs are included in the repository. To recompile from source:

```bash
cd reports/lab3.1
latexmk -pdf -interaction=nonstopmode main.tex
```

All diagrams are drawn directly in LaTeX using **TikZ** and **CircuiTikZ** — no external tools required.

---

## Pin Mapping Summary

| Pin | Lab 1.1 | Lab 1.2 | Lab 2.1 | Lab 2.2 | Lab 3.1 |
|-----|---------|---------|---------|---------|----------|
| 2 | — | — | — | — | DS18B20 DQ |
| 6 | — | Green LED | — | — | — |
| 7 | Red LED | Red LED | Button (INPUT_PULLUP) | Button (INPUT_PULLUP) | — |
| 8 | — | — | Green LED | Green LED | Green LED |
| 9 | — | — | Red LED | Red LED | Red LED |
| 10 | — | — | Yellow LED | Yellow LED | Yellow LED |
| 20 (SDA) | — | LCD I2C | — | — | LCD I2C |
| 21 (SCL) | — | LCD I2C | — | — | LCD I2C |
| 22–25 | — | Keypad rows | — | — | — |
| 26–29 | — | Keypad cols | — | — | — |
| A0 | — | — | — | — | NTC thermistor OUT |

---

## Codebase Statistics

| Metric | Value |
|--------|-------|
| Total C/C++ source lines | ~3 600 |
| Reusable libraries | 10 |
| Lab modules | 5 |
| PlatformIO environments | 5 |
| Wokwi simulations | 5 |
| Report pages (total) | ~195 |
| Target MCU | ATmega2560 @ 16 MHz |
| RAM budget | 8 KB |
| Flash budget | 256 KB |

---

## Tech Stack

- **Hardware:** Arduino Mega 2560 (ATmega2560, 16 MHz, 8 KB RAM, 256 KB Flash)
- **Framework:** Arduino
- **RTOS:** FreeRTOS 11.1.0 (Labs 2.2 & 3.1 — via [feilipu/FreeRTOS](https://github.com/feilipu/Arduino_FreeRTOS_Library))
- **Build:** PlatformIO (AVR toolchain)
- **Simulation:** [Wokwi for VS Code](https://wokwi.com/)
- **Documentation:** LaTeX (pdflatex, TikZ, CircuiTikZ)

---

## License

This repository contains university coursework. All code and reports are provided for educational purposes.

---

## Author

**Adrian Vremere** — Technical University of Moldova (UTM)  
Embedded Systems Course, 2025–2026
