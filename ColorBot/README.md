# ColourBot: 2-DOF Pick-and-Place Robotic Arm with Web Control
### *Mechatronics System Design Project, Dept. of Engineering Design, IIT Madras*

An autonomous 2-Degree-of-Freedom (2-DOF) robotic arm system powered by an **Arduino UNO R4 WiFi**. ColourBot uses an active RGB LDR optical reflective sensor to identify colored ground targets (RED, GREEN, BLUE) against a WHITE background, pick up target objects using an electromagnet end-effector, and transport them to designated drop zones—all configured via a mobile web interface.

[![Watch ColourBot in Action](https://img.youtube.com/vi/_IE9yJssTY4/hqdefault.jpg)](https://youtube.com/shorts/_IE9yJssTY4)
---

## System Architecture & Workflow

ColourBot operates using a deterministic **Finite State Machine (FSM)** coordinated with an onboard Web Server hosted on an Access Point (AP).

```
   ┌──────────────┐
   │  WAIT_INPUT  │◄──────────── Wi-Fi Web Interface
   └──────┬───────┘
          │ (Mission Received)
          ▼
  ┌────────────────┐
  │ SWEEP_FOR_PICK │ ── Pulses RGB LEDs & reads LDR while sweeping base
  └──────┬─────────┘
          │ (Target Color Detected + Offset Applied)
          ▼
     ┌──────────┐
     │   PICK   │ ── Lowers shoulder servo until IR sensor triggers ground
     └───┬──────┘    Energizes Electromagnet and lifts object
         │
         ▼
  ┌────────────────┐
  │ SWEEP_FOR_DROP │ ── Sweeps base to drop location
  └──────┬─────────┘
          │ (Drop Zone Detected + Offset Applied)
          ▼
     ┌──────────┐
     │   DROP   │ ── De-energizes Electromagnet & raises arm
     └───┬──────┘
         │
         ▼
     ┌──────────┐
     │   HOME   │ ── Returns servos to initial position & resets mission
     └──────────┘
```

---

## Hardware Components

1. **Microcontroller:** Arduino UNO R4 WiFi (or compatible ESP32/WiFi board).
2. **Servos (2-DOF):**
   * **Base Servo ("Black Servo"):** Controls horizontal rotational pan ($0^\circ$ to $180^\circ$).
   * **Shoulder Servo ("Blue Servo"):** Controls vertical elevation ($180^\circ$ UP to $\sim120^\circ$ DOWN).
3. **End Effector:** $5\text{V} / 12\text{V}$ Electromagnet controlled via transistor/relay module.
4. **Color Sensor Module:**
   * $1\times$ RGB LED (Common Cathode or Anode with appropriate resistors).
   * $1\times$ Light Dependent Resistor (LDR) configured with a pull-down resistor voltage divider.
5. **Proximity Sensor:** Analog IR Proximity / Distance sensor.

---

## Pin Mapping

| Component | Pin | Type | Function |
| :--- | :--- | :--- | :--- |
| **Black Servo (Base)** | `D5` | PWM Output | Rotates arm horizontally across color strips |
| **Blue Servo (Shoulder)** | `D6` | PWM Output | Lowers/raises electromagnet end-effector |
| **Electromagnet (EM)** | `D7` | Digital Output | Drives relay/transistor to grab metallic objects |
| **RGB LED - RED** | `D9` | PWM/Digital Output | Red illumination channel for optical sensor |
| **RGB LED - GREEN**| `D10` | PWM/Digital Output | Green illumination channel for optical sensor |
| **RGB LED - BLUE** | `D11` | PWM/Digital Output | Blue illumination channel for optical sensor |
| **IR Proximity Sensor**| `A0` | Analog Input | Detects surface proximity during pickup |
| **LDR Sensor** | `A2` | Analog Input | Reads reflected light intensity from ground |

---

## Web Interface & Control

The board initializes an Access Point (AP) named **`ColourBot`**, hosting a lightweight responsive single-page application.

* **Hotspot SSID:** `ColourBot`
* **Hotspot Password:** `colourbot123`
* **Web Page URL:** `http://192.168.4.1`

### Web Features
* **Pick & Drop Selector:** Visual matrix to select source and destination colors (`RED`, `GREEN`, `BLUE`).
* **Live Status Telemetry:** Displays step progress (`find` $\rightarrow$ `pick` $\rightarrow$ `carry` $\rightarrow$ `drop` $\rightarrow$ `home`), active messages, and electromagnet state.
* **Emergency STOP:** Instantly interrupts any active operation and commands the arm to safely return to the home position.

---

## Optical Color Detection Algorithm

The optical sensor identifies surface colors by sequentially pulsing RED, GREEN, and BLUE LEDs, measuring the reflected intensity via the LDR, and normalizing the values against a pre-calibrated **WHITE** ground plane.

$$\text{Intensity} = 1023 - \text{Raw LDR Reading}$$

$$\text{Norm}_C = \frac{\text{Intensity}_C}{\text{WHITE}_C} \quad \text{for } C \in \{R, G, B\}$$

1. **White Check:** If $\text{Norm}_R, \text{Norm}_G, \text{Norm}_B > 0.85$, the surface is identified as **WHITE** (background).
2. **Color Matching:** The channel with the highest normalized reflection ratio determines whether the detected color strip is **RED**, **GREEN**, or **BLUE**.

---

## Getting Started

### 1. Requirements & Libraries
Make sure you have the following installed in your Arduino IDE:
* [Arduino IDE 2.x](https://www.arduino.cc/en/software)
* **Board Package:** Arduino UNO R4 Boards (`renesas_uno`)
* **Libraries:**
  * `Servo.h` (Built-in)
  * `WiFiS3.h` (Built-in for UNO R4 WiFi)

### 2. File Structure
Ensure all source files are placed in the same project directory:
```
ColourBot/
├── ColourBot.ino       # Main sketch with FSM and sensor loop logic
├── wifi_control.h      # WiFi and API interface header
├── wifi_control.cpp    # Access Point, HTTP server, and JSON router
└── page.h              # Embedded HTML/CSS/JS frontend page
```

### 3. Uploading the Code
1. Connect your Arduino board via USB.
2. Select your board (`Arduino UNO R4 WiFi`) and port under **Tools > Board**.
3. Open `ColourBot.ino` and click **Upload**.

### 4. Calibration & Tuning
* **Color Calibration:** Measure reflected raw LDR values over your white surface under ambient light conditions and adjust `WHITE_R`, `WHITE_G`, and `WHITE_B` in `ColourBot.ino` if needed.
* **Offsets & Thresholds:**
  * `ANGLE_OFFSET` ($35^\circ$): Offset added after target color detection to align the electromagnet directly above the target strip.
  * `proximityDetection` ($30$): Analog IR threshold level indicating ground contact.

