# Car Black Box - PIC16F877A microcontroller

[![Author](https://img.shields.io/badge/Author-Nilesh%20Mahajan-blue.svg)](https://github.com/Nileshkm911)
[![Microcontroller](https://img.shields.io/badge/MCU-PIC16F877A%20%40%2020MHz-orange.svg)](https://www.microchip.com/)
[![Firmware](https://img.shields.io/badge/Firmware-Embedded%20C%20(XC8)-green.svg)](https://www.microchip.com/mplab/compilers)
[![Proteus Demo](https://img.shields.io/badge/YouTube-Proteus%20Demo-red.svg)](https://youtu.be/YhkAp6gAiU4)
[![PICSimLab Demo](https://img.shields.io/badge/YouTube-PICSimLab%20Demo-red.svg)](https://youtu.be/SL1gIpM6bI0)

**Author:** Nilesh Mahajan  
**Target Architecture:** Microchip PIC16F877A (8-bit Microcontroller @ 20 MHz)  
**Toolchain:** Microchip MPLAB X IDE & XC8 (C99 Mode)  
**Simulation Environments:** Proteus Design Suite & PICSimLab (PICGenios Configuration)  

---

## 1. Video Demonstrations

Watch the live simulation runs demonstrating real-time dashboard tracking, gear transitions, collision logging, password security lockouts, time configuration, and UART log offloading:

| Proteus Design Suite Simulation | PICSimLab (PICGenios) Simulation |
| :---: | :---: |
| [![Watch Proteus Demo](https://img.youtube.com/vi/YhkAp6gAiU4/0.jpg)](https://youtu.be/YhkAp6gAiU4) | [![Watch PICSimLab Demo](https://img.youtube.com/vi/SL1gIpM6bI0/0.jpg)](https://youtu.be/SL1gIpM6bI0) |
| ▶️ [Watch Proteus Simulation Video](https://youtu.be/YhkAp6gAiU4) | ▶️ [Watch PICSimLab Simulation Video](https://youtu.be/SL1gIpM6bI0) |

---

## 2. Overview

The **Car Black Box (CBB)** is an automotive embedded data logging system inspired by aeronautical flight data recorders. It continuously tracks and logs critical vehicular telemetry — including vehicle speed, real-time clock timestamps, gear shift transitions, and collisions — into the microcontroller's non-volatile internal EEPROM.

Designed with robust state-machine logic, the system features:
- **Telemetry Dashboard:** Real-time 16x2 LCD display showing current time (`HH:MM:SS`), active event code, and vehicle speed (`00-99 km/h`).
- **Throttle Position Sensing:** ADC-based speed acquisition with proportional linear interpolation from an analog potentiometer.
- **Real-Time Clock (RTC):** Dedicated DS1307 RTC communication over bit-banged / hardware I2C bus with BCD time synchronization.
- **Fail-Safe Event Storage:** 10-slot circular FIFO buffer stored in the PIC16F877A internal data EEPROM with write-cycle interrupt locks.
- **Security & Password Management:** 4-digit passcode protection with a 5-second inactivity timeout, 3-attempt brute-force protection, and dynamic 2-minute lockout timer.
- **Serial Telemetry Offloading:** On-demand UART transmission (9600-8-N-1) formatting stored records into tabular reports compatible with Tera Term and standard serial consoles.

---

## 3. Hardware Architecture & Pin Interface

```
                       +-----------------------+
                       |      PIC16F877A       |
                       |       (20 MHz)        |
                       +-----------------------+
                               |   |   |
         +---------------------+   |   +---------------------+
         |                         |                         |
         v                         v                         v
+-----------------+       +-----------------+       +-----------------+
|   Analog Speed  |       |  Control Matrix |       |    DS1307 RTC   |
| Potentiometer   |       |  Tactile Keys   |       |    (I2C Bus)    |
|    (RA0/AN0)    |       |    (RB0-RB5)    |       |   (RC3, RC4)    |
+-----------------+       +-----------------+       +-----------------+
                               |   |   |
         +---------------------+   |   +---------------------+
         |                         |                         |
         v                         v                         v
+-----------------+       +-----------------+       +-----------------+
|  16x2 Character |       |  Internal MCU   |       |   UART Serial   |
|   LCD Display   |       |   Data EEPROM   |       |   Transceiver   |
|  (RD4-7, RE1-2) |       |  (256 B Flash)  |       |   (RC6, RC7)    |
+-----------------+       +-----------------+       +-----------------+
```

### Complete Pin Allocation Table

| Microcontroller Pin | Peripheral Function | Signal Direction | Hardware Interface Description |
| :--- | :--- | :--- | :--- |
| **RA0 / AN0** | Speed Throttle Dial | Input (Analog) | Reads 0V - 5V from potentiometer; converted via 10-bit ADC into speed (0 - 99 km/h). |
| **RB0** | SW1 (Collision / Up) | Input (Digital) | Active-low switch with internal PORTB weak pull-up enabled. |
| **RB1** | SW2 (Gear Up / Down) | Input (Digital) | Active-low switch with internal PORTB weak pull-up enabled. |
| **RB2** | SW3 (Gear Down) | Input (Digital) | Active-low switch with internal PORTB weak pull-up enabled. |
| **RB3** | SW4 (Menu / Key '1') | Input (Digital) | Active-low switch. Also serves as Menu Enter and binary '1' entry. |
| **RB4** | SW5 (Exit / Key '0') | Input (Digital) | Active-low switch. Also serves as Back/Exit and binary '0' entry. |
| **RC3** | RTC SCL | In / Out | I2C Clock line connected to DS1307 RTC with external 4.7kΩ pull-up. |
| **RC4** | RTC SDA | In / Out | I2C Data line connected to DS1307 RTC with external 4.7kΩ pull-up. |
| **RC6 / TX** | Serial Transmit | Output (Digital) | UART TX line transmitting log table to PC serial terminal at 9600 bps. |
| **RC7 / RX** | Serial Receive | Input (Digital) | UART RX line for serial communication. |
| **RD4 - RD7** | LCD Data Bus (D4-D7)| Output (Digital) | High nibble lines for HD44780 16x2 character LCD in 4-bit mode. |
| **RE1** | LCD Enable (EN) | Output (Digital) | Strobed enable line for LCD command and data transfers. |
| **RE2** | LCD Register Select | Output (Digital) | High for character data registers, Low for instruction registers. |

---

## 4. EEPROM Memory Architecture & Mapping

Telemetry logs are permanently persisted in the MCU's internal non-volatile EEPROM (256 bytes) using an indexed circular FIFO buffer.

```
+------------+------------+-------------------------------------------------------------+
| Addr: 0x05 | Addr: 0x06 | Addr: 0x10 - 0x41                                           |
| Log Count  | Head Index | Circular Buffer: 10 Log Slots x 5 Bytes Each (50 Bytes)     |
|  (0 to 10) |  (0 to 9)  | [Slot 0][Slot 1][Slot 2][Slot 3]...[Slot 9]                 |
+------------+------------+-------------------------------------------------------------+
```

### 5-Byte Log Entry Layout
Each recorded telemetry record consists of 5 bytes:

| Byte Offset | Parameter | Format | Description / Range |
| :---: | :--- | :---: | :--- |
| **Byte 0** | Hours | BCD | `0x00` - `0x23` (24-hour format from DS1307 RTC) |
| **Byte 1** | Minutes | BCD | `0x00` - `0x59` |
| **Byte 2** | Seconds | BCD | `0x00` - `0x59` |
| **Byte 3** | Event Index | Integer | Event ID: `0=ON`, `1=GN`, `2=G1`, `3=G2`, `4=G3`, `5=G4`, `6=GR`, `7=C` |
| **Byte 4** | Vehicle Speed | Integer | `00` - `99` (Scaled vehicle speed in km/h) |

### Event Types
- **ON**: Vehicle Power-On cycle (logged once at system boot)
- **GN**: Gear Neutral
- **G1 - G4**: Forward Driving Gears 1 through 4
- **GR**: Reverse Gear
- **C**: Collision event

---

## 5. State Machine Operations & Switch Control Matrix

The firmware operates as a non-blocking finite state machine (FSM) governed by Timer0 1 ms hardware interrupts for debounce, hold timing, and lockout countdowns.

### Switch Functions by System State

| State | SW1 (RB0) | SW2 (RB1) | SW3 (RB2) | SW4 (RB3) | SW5 (RB4) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Dashboard** | Register Collision (`C`) | Gear Up (`GN` &rarr; `G4`) | Gear Down (`G4` &rarr; `GR`) | Long Press (2s): Open Login | *None* |
| **Security Login** | *None* | *None* | *None* | Enter binary `'1'` | Enter binary `'0'` |
| **Lockout Screen** | *Blocked* | *Blocked* | *Blocked* | *Blocked* | *Blocked* |
| **Main Menu** | Scroll Up | Scroll Down | *None* | Enter Selected Option | Exit to Dashboard |
| **View Log** | Scroll Up (Older) | Scroll Down (Newer) | *None* | *None* | Exit to Main Menu |
| **Set Time** | Increment Blinking Field | Select Next Field (H&rarr;M&rarr;S)| *None* | Save & Exit to Dashboard | Discard & Return to Menu |
| **Change Password**| *None* | *None* | *None* | Input `'1'` | Input `'0'` |

---

## 6. Security Features

1. **Default Passcode:** Initialized to `0000` (press SW5 four times).
2. **Inactivity Timeout:** If no key is detected for 5 consecutive seconds in the login screen, the system automatically cancels authentication and reverts to the Dashboard.
3. **Lockout Protection:** Three consecutive incorrect passcodes trigger the Security Lockout state:
   - All switches are hardware-blocked.
   - A live 120-second countdown timer is displayed on the LCD.
   - Upon timer expiration, access is safely restored to the Dashboard.

---

## 7. UART Telemetry Log Format

Selecting **Download log** transmits the EEPROM buffer over UART at 9600-8-N-1 in the following formatted layout:

```text
#  Time      E  SP
0  19:14:41  ON  67
1  19:15:02  GN  00
2  19:15:15  G1  18
3  19:15:30  G2  34
4  19:15:48  G3  52
5  19:16:12  G4  78
6  19:16:45  C   85
```

---

## 8. Project Structure

```text
Car-Black-Box-PIC16F877A/
├── .gitignore                          # Clean build & simulation exclusion list
├── README.md                           # Technical reference & project documentation
├── docs/
│   └── Car_Black_Box_Operations_Manual.pdf # Comprehensive user manual & switch specifications
├── hex/
│   └── car_black_box.hex               # Production HEX binary compiled for PIC16F877A
├── simulation/
│   ├── car_black_box.PDF               # Circuit schematic export
│   ├── car_black_box.pdsprj            # Proteus simulation project
│   └── Proteus_Schematic_Layout_Guide.png # Wiring schematic reference
├── screenshots/
│   ├── proteus/                        # Screen captures from Proteus simulation
│   └── picsimlab/                      # Screen captures from PICSimLab simulation
└── src/
    ├── adc.c / adc.h                   # 10-bit ADC throttle speed driver
    ├── common.h                        # System state defines, clock macros, pins
    ├── dashboard.c / dashboard.h       # Dashboard UI and gear control implementation
    ├── eeprom.c / eeprom.h             # Safe internal EEPROM byte read/write driver
    ├── event_log.c / event_log.h       # Circular FIFO log buffer & UART reporting
    ├── i2c.c / i2c.h                   # Bit-banged / hardware I2C master driver
    ├── lcd.c / lcd.h                   # 16x2 character LCD 4-bit nibble driver
    ├── main.c / main.h                 # ISR, Timer0 timebase, system initialization
    ├── menu.c / menu.h                 # Scrolling menu and submenu navigation
    ├── password.c / password.h         # Passcode verification, lockout & change UI
    ├── rtc.c / rtc.h                   # DS1307 RTC time retrieval & BCD handling
    ├── switch.c / switch.h             # Matrix keypad driver (state & level detection)
    └── uart.c / uart.h                 # UART serial driver (9600 Baud)
```

---

## 9. Simulation & Verification Instructions

### Running in Proteus
1. Launch **Proteus 8 Professional**.
2. Open `simulation/car_black_box.pdsprj`.
3. Right-click the **PIC16F877A** MCU &rarr; select **Edit Properties**.
4. Confirm Processor Clock Frequency is set to `20MHz`.
5. Point Program File to `hex/car_black_box.hex`.
6. Click **Play** at the bottom-left to run the simulation.
7. Open the **Virtual Terminal** window to view UART log downloads.

### Running in PICSimLab
1. Launch **PICSimLab**.
2. Under **Board**, select `PICGenios`.
3. Under **Microcontroller**, select `PIC16F877A`.
4. Set Clock to `20 MHz`.
5. Under **File** &rarr; **Load Hex**, select `hex/car_black_box.hex`.
6. Open **Tools** &rarr; **Serial Terminal** (9600 baud, 8-N-1) to receive downloaded logs.

---

## 10. Compilation Instructions

The project can be built using Microchip MPLAB X IDE with the XC8 compiler:
1. Open **MPLAB X IDE**.
2. Select **File &rarr; Open Project** and navigate to the project directory.
3. Select **XC8** as the compiler and **PIC16F877A** as the device.
4. Click **Clean and Build Project** (`Shift + F11`).
5. Output HEX file will be generated in `dist/default/production/`.
