# 🛰️ Spacecraft ECLSS & Avionics Engine (V2)
A hardened, headless telemetry and Environmental Control & Life Support System (ECLSS) firmware for ESP32. Engineered in modern low-level C/C++ using non-blocking asynchronous state loops, atomic register operations, discrete DSP filtering, and a packed binary communication protocol.

> **Mission Objective:** Prototype avionics unit for a deep-space habitat / Martian descent capsule. Operates autonomously, filters sensor noise, manages thermal/power subsystems, and streams synchronized binary telemetry at hypersonic UART baud rates.

---

## 🚀 Key Architectural Improvements (V1 ➔ V2)

- **Headless Architecture:** Eliminated blocking I2C OLED displays and heavy peripherals, eradicating 40ms loop latency and converting the firmware into a true headless flight controller.
- **Hypersonic Telemetry Stream:** Serial communication speed boosted from legacy 9600 baud to **921,600 baud**, streaming a strictly packed 10-byte binary frame.
- **Deterministic Finite State Machine (FSM):** 4-stage operational life cycle:
  - `STATE_BOOT`: Safe hardware self-diagnostics and sensor warmup.
  - `STATE_AUTO`: Autonomous climate control with hysteresis & automated solar array deployment.
  - `STATE_MANUAL`: Edge-triggered latching manual overrides for crew intervention.
  - `STATE_EMERGENCY`: Fail-safe subsystem shutdown on critical sensor / bus failures.
- **Discrete Signal Filtering (DSP):** Integrated single-pole Exponential Moving Average (EMA) low-pass filter (`k = 0.1f`) on the ADC pipeline to suppress raw ESP32 voltage noise.
- **Solar Deployment Dynamics:** Time-delayed 3-state mechanical deployment simulation (`STOWED` ➔ `DEPLOYING` ➔ `DEPLOYED`) with hysteresis dead-bands.

---

## 📡 Binary Telemetry Protocol (`proto.h`)

Telemetry frames are packed without compiler padding (`__attribute__((packed))`) and protected by an XOR checksum.

| Offset | Field | Type | Description |
| :---: | :--- | :--- | :--- |
| `0x00` | `sync1` | `uint8_t` | Frame Preamble 1 (`0xAA`) |
| `0x01` | `sync2` | `uint8_t` | Frame Preamble 2 (`0x55`) |
| `0x02` | `hum` | `uint8_t` | Relative Humidity (%) |
| `0x03` | `temp` | `uint16_t` | Temperature in fixed-point (`T * 10`) |
| `0x05` | `relay1` | `uint8_t` | Cabin Main Lighting Status (0/1) |
| `0x06` | `relay2` | `uint8_t` | Cabin Thermal Actuator Status (0/1) |
| `0x07` | `solar_state` | `uint8_t` | Solar Array Generation Zone (0=Dark, 1=Penum, 2=Direct) |
| `0x08` | `manual` | `uint8_t` | Operational Mode Flag (0=Auto, 1=Manual) |
| `0x09` | `check_sum` | `uint8_t` | 8-bit XOR Frame Checksum |
| **Total** | | | **Exactly 10 Bytes** |

---

## 📌 Hardware Pinout Table

| Component | ESP32 GPIO Pin | Mode / Configuration | Description |
| :--- | :--- | :--- | :--- |
| **DHT11 Sensor** | `GPIO 13` | Digital | Cabin temperature & humidity bus |
| **Relay 1 (Lighting)**| `GPIO 26` | `OUTPUT` (Active LOW) | Primary cabin illumination actuator |
| **Relay 2 (Heater)** | `GPIO 25` | `OUTPUT` (Active LOW) | Thermal regulation / heater relay |
| **Solar Array (LDR)**| `GPIO 35` / `34` | `ANALOG INPUT` (ADC1) | Photodiode array with discrete EMA filter |
| **Manual Button (Light)**| `GPIO 27` | `INPUT_PULLUP` | Crew cabin light latching toggle |
| **Manual Button (Warm)** | `GPIO 14` | `INPUT_PULLUP` | Crew cabin heater latching toggle |
| **Mode Switch** | `GPIO 0` | `INPUT` (Interrupt) | Instant `AUTO`/`MANUAL` FSM transition |

---

## 🛠️ Software Stack

- **Target Hardware:** ESP32 (NodeMCU / ESP-WROOM-32)
- **Core Platform:** PlatformIO Core (VS Code) on ESP-IDF / Arduino Framework
- **Baud Rate:** 921,600 bps
- **Development OS:** CachyOS (Arch Linux, custom performance kernel)

---

## 📜 License
Distributed under the **MIT License**.

> *“We make the future closer.”*  
> — **Tim Oppie Trofimov** // Twin Engineering // November 2026
