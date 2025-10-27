![C](https://img.shields.io/badge/C-00599C.svg?style=for-the-badge&logo=c&logoColor=white)
![STM32](https://img.shields.io/badge/STM32-F103-000000.svg?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![QEMU](https://img.shields.io/badge/QEMU-Emulator-FF6600.svg?style=for-the-badge&logo=qemu&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-Ready-FCC624.svg?style=for-the-badge&logo=linux&logoColor=black)

<div align="center">
  <h1>STM32F1 Button Debounce (FSM + SysTick)</h1>
  <p><strong>Professional debounce – No HAL – Non-blocking – QEMU Emulator</strong></p>
</div>

---

## Introduction

This project **emulates STM32F103 (Blue Pill)** on **QEMU** to demonstrate **professional button debouncing** using:

- **4-state Finite State Machine (FSM)**
- **SysTick 1 ms interrupt**
- **Non-blocking UART**
- **UART-controlled LED toggle** via `'P'`

> **No hardware required — Test instantly on Linux!**

---

## Features

| Feature              | Description |
|----------------------|-----------|
| **Dual-edge debounce** | Filters noise on **press & release** |
| **Single callback**    | `on_press()` called **once** per stable press |
| **Non-blocking CPU**   | `while(1)` empty → safe for `__WFI()` |
| **QEMU STM32-P103**    | Full GPIO + UART + SysTick emulation |
| **UART toggle**        | Send `'P'` → toggle LED |
| **20ms debounce**      | Configurable via `DEBOUNCE_MS` |

---

## Quick Start (Linux)

### 1. Install Dependencies

```bash
sudo apt update
sudo apt install -y qemu-system-arm gcc-arm-none-eabi netcat-openbsd make

### 2. Clone & Enter Directory


```bash
git clone https://github.com/huyhoang-1501/Debouncing.git
cd Debouncing

### 3. Build & Run QEMU


```bash
make flash
QEMU starts with UART on localhost:1234

### 4. Toggle LED (New Terminal)

```bash
echo "P" | nc localhost 1234

---

### Expected Output
=== DEBOUNCE TEST ===
GÕ 'P' → TOGGLE LED 
>>> TOGGLE (P)
LED ON
>>> TOGGLE (P)
LED OFF

Debounce delay: 20ms — LED toggles only after stable input!

---

### File Structure

Debouncing/
├── main.c              # Core: FSM, UART, SysTick, Vector Table
├── stm32f103.ld        # Linker script (64K Flash, 20K RAM)
├── Makefile            # Build, flash, clean
├── run.sh              # Auto-test: 3x toggle with 3s delay
├── README.md           # This file
└── .gitignore          # Clean repo