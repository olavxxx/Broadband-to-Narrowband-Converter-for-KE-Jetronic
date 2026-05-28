# Broadband-to-Narrowband Converter for KE-Jetronic

A PlatformIO project that converts broadband lambda sensor signals to narrowband (0.1V–0.9V) signals for use with KE-Jetronic engine management systems. Runs on an Arduino UNO R4 WiFi with configurable AFR targeting.

## Overview

This project bridges modern broadband lambda sensors with legacy KE-Jetronic fuel injection systems. It reads a continuous broadband sensor voltage, simulates a narrowband sensor response around a target AFR, and outputs the converted signal to the engine ECU.

**Key Features:**
- Real-time broadband-to-narrowband signal conversion
- Configurable AFR target (e.g., 14.3, 14.1 for richer mixtures)
- Dynamic narrowband slope simulation
- LED matrix visualization of output voltage
- Serial monitor diagnostics (AFR, output voltage, system status)
- Thermal management detection (warmup & error states)

## Hardware Requirements

- **Microcontroller:** Arduino UNO R4 WiFi
- **Broadband Lambda Sensor:** AEM or compatible (0.0V–5.0V output)
- **DAC Output:** Converted narrowband signal
- **Power:** 5V USB or external supply
- **Optional:** 0.1µF capacitor on DAC output (noise filtering)

## Pinout

| Pin | Function | Purpose |
|-----|----------|---------|
| A1  | Analog Input | Broadband lambda sensor (AEM) signal |
| A0  | Analog Output | Narrowband lambda simulation (to KE-Jetronic ECU) |
| GND | Ground | **Must connect to the same reference ground point where KE-Jetronic is grounded** (e.g., intake manifold or dedicated engine ground) |

**⚠️ Critical:** Use the same ground reference point as the KE-Jetronic ECU—typically the intake manifold or engine block ground point. This ensures proper signal integrity and prevents ground loops that can cause signal corruption or erratic ECU behavior.

## Proper Grounding & Installation

**Ground Connection (Most Important):**
1. Locate the KE-Jetronic ECU's ground point (usually on the intake manifold or engine block)
2. Connect the converter's GND pin directly to this same point
3. Use a dedicated wire, **not** the car's chassis ground or battery negative
4. Strip ~6mm of insulation and secure with a ring terminal or solder joint
5. Ensure the connection is clean, tight, and corrosion-free

**Signal Connections:**
- **Broadband Input (A1):** Tap into the AEM wideband sensor's signal wire (typically the thinner wire)
- **Narrowband Output (A0):** Connect to the KE-Jetronic's original narrowband lambda input wire (usually at the ECU connector pin)
- Use shielded cable for both inputs/outputs to minimize electrical noise

**Power & USB:**
- Power the Arduino via USB connected to a 12V car charger/converter, or wire 12V from the vehicle power system
- Ensure the 12V converter shares the same ground reference (engine block)

## Configuration

Edit the following constants in `src/main.cpp` to customize behavior:

```cpp
const float TARGET_AFR = 14.3;        // Target air-fuel ratio (lower = richer)
const float STOICH_AFR = 14.7;        // Stoichiometric AFR for your fuel type
const float SLOPE_STEEPNESS = 5.0;    // Narrowband slope steepness (higher = more binary)
```

### Common AFR Targets
- **14.7:** Stoichiometric (default, balanced)
- **14.3–14.1:** Slightly rich (better performance, more fuel consumption)
- **15.0+:** Lean (better economy, higher temps)

## Installation & Build

1. **Install PlatformIO** (VS Code extension or CLI)
2. **Clone or open this project**
3. **Configure your board and libraries** (platformio.ini):
   ```ini
   [env:uno_r4_wifi]
   platform = renesas-ra
   board = uno_r4_wifi
   framework = arduino
   lib_deps = arduino-libraries/ArduinoGraphics
   ```
4. **Build and upload:**
   ```bash
   platformio run --target upload
   ```

## How It Works

### Signal Conversion Process

1. **Read Broadband Signal:** Analog input on A1 (0.0–5.0V) from broadband lambda sensor
2. **Calculate Lambda:** Converts voltage to lambda value using AEM sensor curve:
   ```
   lambda = (0.1621 × voltage) + 0.4990
   ```
3. **Calculate Narrowband Output:** Simulates narrowband response around target AFR:
   ```
   outputVoltage = 0.5 - ((lambda - targetLambda) × SLOPE_STEEPNESS)
   clamped to 0.1V–0.9V range
   ```
4. **Output DAC Signal:** Analog output on A0 fed to KE-Jetronic ECU

### System States

| Status | Condition | Output | Reason |
|--------|-----------|--------|--------|
| **WARMING** | AEM voltage < 0.5V | 0.5V | Cold start/sensor warmup |
| **ERROR** | AEM voltage > 4.5V | 0.5V | Sensor malfunction or wiring issue |
| **RUNNING** | Normal operation | Dynamic | Active conversion |

### LED Matrix Display

The built-in LED matrix shows a real-time graph of the output voltage (0.1V–0.9V), scrolling left continuously. This helps verify correct operation and diagnose issues.

## Serial Output

Connect via USB (9600 baud) to monitor:

```
System Ready. Target AFR: 14.3
Target Lambda: 0.973
AFR: 14.2 | Out: 0.512V | Status: RUNNING
AFR: 14.1 | Out: 0.525V | Status: RUNNING
```

**Output every 1 second** during normal operation. Check the serial monitor to verify:
- Correct AFR calculation
- Proper DAC output voltage
- System health (RUNNING vs ERROR/WARMING)

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| Output stuck at 0.5V | Cold sensor or error | Warm up engine; check AEM wiring |
| Output not changing | No signal from AEM | Verify A1 connection; check sensor |
| High voltage spikes | Electrical noise | Add 0.1µF capacitor on output; use shielded cable |
| LED matrix not displaying | Display initialization failed | Restart board; check USB power |
| AFR reading incorrect | Wrong sensor curve | Verify AEM sensor type; adjust formula |

## Performance Notes

- **Update Rate:** DAC output updated every loop cycle (~5–10ms)
- **Graph Refresh:** LED matrix updates every 50ms
- **Serial Output:** Throttled to 1 per second to avoid overwhelming serial buffer
- **Resolution:** 12-bit DAC (4096 levels over 0–5V = ~1.22mV per step)

## License

See LICENSE file in this repository.

## References

- [Arduino UNO R4 WiFi Documentation](https://docs.arduino.cc/hardware/uno-r4-wifi/)
- [KE-Jetronic Fuel Injection System](https://en.wikipedia.org/wiki/Jetronic#K-Jetronic_/_KE-Jetronic)
- AEM Wideband Sensor Output Specifications
- ArduinoGraphics Library

## Contact & Support

For issues or improvements, refer to the project repository or contact the maintainer.
