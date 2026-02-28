# Embedded Systems Labs — `es-labs`

A collection of four embedded systems laboratory projects for the **Arduino Mega 2560** (ATmega2560), progressing from basic serial I/O to real-time operating system multitasking with FreeRTOS.

Each lab ships with **working firmware** (PlatformIO), a **Wokwi simulation**, and a **LaTeX report** (compiled PDF included).

---

## Labs at a Glance

| Lab | Title | Scheduling | Key Concepts | Hardware |
|-----|-------|------------|--------------|----------|
| **1.1** | Serial LED Control | `loop()` polling | UART, C STDIO redirection, command parsing | LED |
| **1.2** | LCD + Keypad Lock System | `loop()` event-driven | I2C LCD, 4×4 keypad, 10-state FSM, password management | LCD 1602, membrane keypad, 2 LEDs |
| **2.1** | Button Press Monitor — Bare-Metal | Non-preemptive cooperative scheduler | Deadline-driven task dispatch, button debounce FSM, shared globals | Push button, 3 LEDs |
| **2.2** | Button Press Monitor — FreeRTOS | Preemptive RTOS | Binary semaphore, mutex with priority inheritance, `vTaskDelay` | Push button, 3 LEDs |

Labs 2.1 and 2.2 implement **the same application** (button duration monitoring, LED feedback, 10-second statistics reports) under two different scheduling paradigms, allowing a direct comparison between cooperative and preemptive multitasking.

---

## Repository Structure

```
es-labs/
├── labs/                          # PlatformIO project (all firmware)
│   ├── platformio.ini             # 4 build environments (lab1_1 … lab2_2)
│   ├── src/
│   │   └── main.cpp               # Lab selector via #ifdef guards
│   ├── lab/                       # Per-lab entry points
│   │   ├── lab1_1/                #   Serial LED control
│   │   ├── lab1_2/                #   LCD + Keypad lock FSM
│   │   ├── lab2_1/                #   Cooperative scheduler monitor
│   │   └── lab2_2/                #   FreeRTOS preemptive monitor
│   ├── lib/                       # Reusable libraries
│   │   ├── CommandParser/         #   Text → command enum parser
│   │   ├── KeypadInput/           #   4×4 matrix keypad driver
│   │   ├── LcdDisplay/            #   I2C 16×2 LCD driver
│   │   ├── Led/                   #   Single-pin LED driver
│   │   ├── LockFSM/              #   10-state electronic lock FSM
│   │   ├── StdioSerial/          #   printf/fgets → UART redirection
│   │   └── TaskScheduler/        #   Bare-metal cooperative scheduler
│   └── wokwi/                     # Wokwi simulation configs
│       ├── lab1.1/                #   diagram.json + wokwi.toml
│       ├── lab1.2/
│       ├── lab2.1/
│       └── lab2.2/
├── reports/                       # LaTeX lab reports (with compiled PDFs)
│   ├── lab1.1/
│   ├── lab1.2/
│   ├── lab2.1/
│   └── lab2.2/
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
```

Build all labs at once:

```bash
pio run
```

### Upload to Hardware

```bash
pio run -e lab2_2 --target upload
pio device monitor -b 9600
```

### Run in Wokwi Simulator

1. Open the workspace in VS Code.
2. Navigate to the Wokwi config for the desired lab (e.g., `labs/wokwi/lab2.2/wokwi.toml`).
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

## Library Reference

All reusable libraries live in `labs/lib/` and are auto-discovered by PlatformIO.

| Library | Files | Lines | Description |
|---------|-------|-------|-------------|
| **Led** | `Led.h` / `Led.cpp` | 102 | GPIO LED driver — `init()`, `turnOn()`, `turnOff()`, `toggle()`, `isOn()` |
| **StdioSerial** | `StdioSerial.h` / `StdioSerial.cpp` | 120 | Redirects C `stdout`/`stdin` to UART via `fdevopen()` — `stdioSerialInit(baud)` |
| **CommandParser** | `CommandParser.h` / `CommandParser.cpp` | 126 | Parses trimmed serial input into command enums — `parseCommand(input)` |
| **KeypadInput** | `KeypadInput.h` / `KeypadInput.cpp` | 107 | 4×4 matrix keypad wrapper with 20 ms debounce — `init()`, `getKey()` |
| **LcdDisplay** | `LcdDisplay.h` / `LcdDisplay.cpp` | 155 | I2C LCD 16×2 wrapper — `init()`, `clear()`, `printLine()`, `showTwoLines()` |
| **LockFSM** | `LockFSM.h` / `LockFSM.cpp` | 553 | 10-state lock FSM — `processKey()`, `isLocked()`, `getDisplay()` |
| **TaskScheduler** | `TaskScheduler.h` / `TaskScheduler.cpp` | 130 | Deadline-driven cooperative scheduler — `schedulerInit()`, `schedulerRun()` |

---

## Reports

Each lab includes a detailed LaTeX report following an academic structure:

1. **Domain Analysis** — Technologies, hardware/software components, case study
2. **Design** — Architecture diagrams (TikZ), FSM state diagrams, flowcharts, CircuiTikZ schematics
3. **Results** — Build output, simulation screenshots, test scenarios, serial logs
4. **Conclusions** — Performance analysis, limitations, improvements

Compiled PDFs are included in the repository. To recompile from source:

```bash
cd reports/lab2.2
latexmk -pdf -interaction=nonstopmode main.tex
```

All diagrams are drawn directly in LaTeX using **TikZ** and **CircuiTikZ** — no external tools required.

---

## Pin Mapping Summary

A single source of truth for pin assignments across all labs:

| Pin | Lab 1.1 | Lab 1.2 | Lab 2.1 | Lab 2.2 |
|-----|---------|---------|---------|---------|
| 6 | — | Green LED | — | — |
| 7 | Red LED | Red LED | Button (INPUT_PULLUP) | Button (INPUT_PULLUP) |
| 8 | — | — | Green LED | Green LED |
| 9 | — | — | Red LED | Red LED |
| 10 | — | — | Yellow LED | Yellow LED |
| 20 (SDA) | — | LCD I2C | — | — |
| 21 (SCL) | — | LCD I2C | — | — |
| 22–25 | — | Keypad rows | — | — |
| 26–29 | — | Keypad cols | — | — |

---

## Codebase Statistics

| Metric | Value |
|--------|-------|
| Total C/C++ source lines | ~2,885 |
| Reusable libraries | 7 |
| Lab modules | 4 |
| PlatformIO environments | 4 |
| Wokwi simulations | 4 |
| Report pages (total) | ~120 |
| Target MCU | ATmega2560 @ 16 MHz |
| RAM budget | 8 KB |
| Flash budget | 256 KB |

---

## Tech Stack

- **Hardware:** Arduino Mega 2560 (ATmega2560, 16 MHz, 8 KB RAM, 256 KB Flash)
- **Framework:** Arduino
- **RTOS:** FreeRTOS 11.1.0 (Lab 2.2 — via [feilipu/FreeRTOS](https://github.com/feilipu/Arduino_FreeRTOS_Library))
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
