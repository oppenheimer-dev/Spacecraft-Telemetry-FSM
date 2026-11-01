
# 🛰️ Spacecraft-Telemetry-FSM ()
A robust, non-blocking telemetry and life-support flight software for ESP32, written in pure C99/C++ using direct hardware register manipulation, asynchronous software timers (`millis()`), and interrupt-driven manual overrides.


### Small mission legends:
Mission legends: This is a prototype of an autonomous control unit for a spacecraft compartment (or a lunar lander module). The system monitors critical environmental parameters, responds automatically to emergencies, and displays telemetry on an onboard terminal.

---

## 🚀 System Architecture & Features

- **Asynchronous Task Dispatcher:** Zero blocking `delay()` calls. All sensors and controllers run concurrently via non-blocking state loops.
- **Direct Register Manipulation:** High-speed atomic GPIO operations (`GPIO.out_w1ts`, `GPIO.out_w1tc`) for microsecond-level hardware response.
- **Hardware Interrupts (`IRAM_ATTR`):** Debounced hardware interrupt handler for instant manual/auto mode switching.
- **Binary Telemetry Stream:** Packs system telemetry (uptime, packet ID, motion counters, solar panel status) into a compact raw binary structure (`SpacecraftPacket`) transmitted via Serial.
- **Visual Telemetry Display:** Real-time data rendering on an SSD1306 0.96" I2C OLED display.

---

## 📌 Hardware Pinout Table

| Component | ESP32 GPIO Pin | Mode / Protocol | Description |
| :--- | :--- | :--- | :--- |
| **OLED SDA** | `GPIO 21` | I2C | Serial Data line for display |
| **OLED SCL** | `GPIO 22` | I2C | Serial Clock line for display |
| **DHT11 Sensor** | `GPIO 27` | Digital / DHT Library | Cabin temperature & humidity monitor |
| **Relay 1 (Climate)** | `GPIO 4` | `OUTPUT` (Active LOW) | Thermal control / Heater actuator |
| **Relay 2 (Power Bus)** | `GPIO 5` | `OUTPUT` (Active LOW) | Auxiliary power / Systems actuator |
| **PIR Motion Sensor** | `GPIO 25` | `INPUT` | Crew presence / Motion detector |
| **Photoresistor (LDR)** | `GPIO 34` (ADC1) | `INPUT` (RTC_GPIO) | Solar array deployment sensor |
| **Red LED (Warning)** | `GPIO 13` | `OUTPUT` | Thermal alert / Active climate indicator |
| **Green LED (Heartbeat)**| `GPIO 12` | `OUTPUT` | System operational heartbeat |
| **Manual Switch** | `GPIO 26` | `INPUT_PULLUP` (Interrupt) | AUTO/MANUAL mode toggle button |
| **Kill Switch / Action** | `GPIO 0` | `INPUT` | Manual relay override action |

---

## 🛠️ Software Stack

- **Microcontroller:** ESP32 (NodeMCU / ESP-WROOM-32)
- **Framework:** Arduino Core on ESP-IDF
- **Toolchain:** PlatformIO Core (VS Code)
- **OS Environment:** CachyOS (Arch Linux)

---

## 📜 Code Snippet: ISR & Direct Registers
```cpp
void IRAM_ATTR manual_switch_iram() {
  uint64_t now = esp_timer_get_time() / 1000;
  if(now - last_click_iram > dreb_delay){
    last_click_iram = now;
    manual = !manual;
  }
}
```

---

## 📜 License
Open-source and distributed under the **MIT License**.

> *Ad Astra, broo!*  
> — **Dr. Oppie** (`oppenheimer-dev`) // 02.10.2026
