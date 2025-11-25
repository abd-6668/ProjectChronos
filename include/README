# Project Chronos - AI Coding Agent Instructions

## Project Overview
**Project Chronos** is an Arduino-based multi-timer application with an ILI9341 TFT LCD display (240x320), RGB LED indicators, and a 9-button control interface. The device supports up to 3 simultaneous timers with editing, stopwatch functionality, and a comprehensive settings menu.

## Architecture & Key Components

### Hardware Stack
- **MCU**: Arduino Uno (ATmega328P)
- **Display**: ILI9341 TFT LCD (240x320px, SPI interface)
- **Audio**: Buzzer on pin 5
- **RGB LED**: 3 PWM pins (6=RED, 7=GREEN, 8=BLUE)
- **Input**: 9 buttons on digital/analog pins (see `main.cpp` pin definitions)
- **Simulation**: Wokwi simulator (`diagram.json`, `wokwi.toml`)

### Code Organization
- **`src/main.cpp`**: Single monolithic file (~900 lines) with all logic
- **`lib/bmp/`**: Unused library directory (BMP image support, not integrated)
- **`platformio.ini`**: Build config with Adafruit GFX/ILI9341 libraries

### Core Data Structure
```cpp
struct TimerData {
  uint8_t digits[6];              // HH:MM:SS format
  unsigned long remainingMillis;  // Countdown value
  bool running;                   // Timer state
  bool editing;                   // Edit mode flag
  int editDigit;                  // 0-5 (digit being edited)
  unsigned long stopwatchElapsedMillis;
  bool triggered;                 // Alert state
  // ... "last*" fields for dirty-flag optimization
};
```

Three timers stored in `timerArr[3]`.

## Developer Workflows

### Build & Test
```bash
# PlatformIO build
platformio run -e uno

# Simulate in Wokwi
# Uses diagram.json for circuit + firmware.bin from build output
# Firmware path: .pio/build/uno/firmware.bin
```

### Pin Mapping (Critical)
| Function | Pin | Type |
|----------|-----|------|
| Button OK | 2 | Digital |
| Button Remove | 3 | Digital |
| Button Start | 4 | Digital |
| Buzzer | 5 | Digital (tone) |
| RGB Red | 6 | PWM |
| RGB Green | 7 | PWM |
| RGB Blue | 8 | PWM |
| LCD CS | 9 | Digital (SPI) |
| LCD RST | 10 | Digital |
| LCD DC | 11 | Digital |
| LCD MOSI | 12 | Digital (SPI) |
| LCD SCK | 13 | Digital (SPI) |
| Button Esc | A0 | Analog Input |
| Button Up | A1 | Analog Input |
| Button Down | A2 | Analog Input |
| Button Left | A3 | Analog Input |
| Button Right | A4 | Analog Input |
| Button Add | A5 | Analog Input |

**Never change these without updating `diagram.json` connections.**

## Code Patterns & Conventions

### 1. **Debounce via State Tracking**
```cpp
int lastButtonOkState = HIGH;
// In loop():
if (stateButtonOk == LOW && lastButtonOkState == HIGH) {
  // Action on falling edge only
  delay(75); // Hardware debounce
}
lastButtonOkState = stateButtonOk;
```
All 9 buttons follow this pattern. Adding new buttons requires:
- Global `lastButton*State = HIGH`
- `digitalRead()` in loop
- Falling-edge check with 75ms delay

### 2. **Dirty-Flag Optimization for Display**
Instead of redrawing on every loop iteration, track previous state:
```cpp
if (T.lastRemainingSec != remSec || T.lastTriggered != T.triggered) {
  // Redraw only changed regions
  T.lastRemainingSec = remSec;
}
```
Every timer field has `last*` variants to minimize TFT writes (slow operation).

### 3. **Page-Based State Machine**
```
page = 0  → Main timer view
page = 1  → Menu (Settings, Help, About, Close)
page = 2  → Settings submenu
page = 3+ → Subpages (Help, About, Contact, Light Mode, etc.)
```
Navigation via `stateButtonEsc` (ESC key) or button OK selections. Page transitions handle all state initialization.

### 4. **RGB LED Status Codes**
- **Green + Running timer**: Active countdown
- **Red (pulsing 1.2s) + Triggered**: Timer expired
- **Blue**: Menu is open
- **Default (no timer)**: Blue LED steady

Implemented in `loop()` at the bottom with priority: `recentTrigger > anyRunning > anyTriggered > default`.

### 5. **Timer Input Format**
Digits stored as array: `digits[0-1]` = hours, `[2-3]` = minutes, `[4-5]` = seconds.
Edit mode validates: hours/minutes 0-9, tens-digits capped at 5 (e.g., `:59` max).

## Integration Points & Data Flows

### Event Loop Sequence (in `loop()`)
1. **Poll all 9 buttons** → check falling edges
2. **Handle button actions** → navigate pages, edit timers, start/stop
3. **Update running timers** → decrement `remainingMillis`, trigger buzzer when elapsed
4. **Redraw changed UI** → dirty-flag checks minimize redraws
5. **Update RGB LED** → reflect current state

### Display Redraw Paths
- `drawTimer(i)` → renders individual timer box (optimized via dirty flags)
- `drawMenu()` → renders main menu with gold highlighting
- `drawSettingsMenu()` → renders 7-item settings submenu
- `drawSubPage()` → renders static Help/About/Contact pages (no state update needed)
- `mainPage()` → initializes timer view with header + empty state message

### Audio Alert
```cpp
tone(buzzerPin, 1200, TRIGGER_RED_DURATION); // 1.2kHz, 1.2s duration
lastTriggerTime = millis(); // For RGB LED pulsing logic
```

## Critical Project-Specific Rules

1. **No dynamic memory** – Fixed array `timerArr[3]` only. Never use malloc/new.
2. **Minimize TFT writes** – Each `tft.fillRect()` blocks ~10ms+. Use dirty flags aggressively.
3. **Wokwi simulation** – Always test circuit in `diagram.json`. Pin mismatches cause silent failures.
4. **Version tracking** – Update `textMiddle()` intro splash ("Version 1.13") when releasing.
5. **Settings persistence** – Currently no EEPROM: all settings reset on power cycle.
6. **Stopwatch co-counter** – Timers can count up (stopwatch) after countdown finishes; both `remainingMillis` and `stopwatchElapsedMillis` tracked.

## Examples of Discoverable Patterns

### Adding a New Button
```cpp
// 1. Add global state
int lastButtonNewState = HIGH;
const int buttonNew = A6;

// 2. Add pin setup in setup()
pinMode(buttonNew, INPUT_PULLUP);

// 3. Add poll & action in loop()
int stateNew = digitalRead(buttonNew);
if (stateNew == LOW && lastButtonNewState == HIGH) {
  // Your action
  delay(75);
}
lastButtonNewState = stateNew;
```

### Adding a Settings Field
```cpp
// 1. Expand settingsField range
if (settingsField > 6) settingsField = 0;  // was 6, now 7

// 2. Add option text to array in drawSettingsMenu()
const char* options[7] = {..., "New Option"};

// 3. Handle selection in loop() page==2 switch
case 7:
  page = 17;
  drawSubPage("New Option", "");
  break;
```

## Testing & Debugging

- **Serial output**: `Serial.begin(9600)` enabled; logs on startup
- **Wokwi console**: Simulates serial output in browser
- **Buzzer volume**: Set to 0.1 in `diagram.json` to avoid audio warnings in sim
- **RGB LED states**: Visually verify in Wokwi with real-time color changes

---

**Last Updated**: November 25, 2025 | **Version**: 1.13  
**Team**: Abdallah (Lead Software), Roni (Software & Hardware), Jawad (Lead Hardware), Adam (Testing)
