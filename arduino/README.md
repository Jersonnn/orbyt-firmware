[![Orbyt App](https://img.shields.io/badge/Orbyt-App-34A853?style=for-the-badge&logo=google-play)](https://play.google.com/store/apps/details?id=com.jersondev.orbyt)

# Orbyt Arduino Library

A local-first IoT controller library for ESP32 and ESP8266 boards. It exposes the Orbyt JSON API v1.1 for instant pin control from the Orbyt app with no cloud dependency.

```cpp
#include <OrbytAuto.h>

const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";

OrbytAuto board;

void setup() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  board.begin("MyESP", "orbyt-auto-1.0.0");
}

void loop() {
  board.handle();
}
```

Flash once. Configure every pin from the app. That's it.

---

## Two Library Variants

| Library | Best For |
|---------|----------|
| **OrbytAuto** | Flash once, configure everything from the app. No per-project code changes. |
| **OrbytDevice** | Projects needing custom write/read callbacks, or firmware-baked pin roles at flash time. |

**Use OrbytAuto for most cases.** OrbytDevice is for when you need `setOnWriteCallback`/`setOnReadCallback` or want to hard-code pin roles in firmware.

---

## Supported Boards

- ESP32 (all variants)
- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)

For Arduino Wi-Fi boards outside the ESP family, use the Orbyt JSON API v1.1 contract with the board's native Wi-Fi/server library, or start from the templates in `../micropython`, `../circuitpython`, and `../node`.

---

## Quick Start

### 1. Install the Library

Copy the `orbyt-firmware/arduino/` folder into your Arduino `libraries/Orbyt/` directory:

```
~/Arduino/libraries/Orbyt/
├── library.properties
├── OrbytAuto.h / .cpp
├── OrbytDevice.h / .cpp
├── OrbytPin.h / .cpp
├── OrbytJson.h / .cpp
└── examples/
    ├── OrbytAuto/OrbytAuto.ino
    ├── esp32/esp32.ino
    └── esp8266/esp8266.ino
```

Restart Arduino IDE. The library appears under **Sketch → Include Library → Orbyt**.

### 2. Flash OrbytAuto

1. Open **File → Examples → Orbyt → OrbytAuto → OrbytAuto**
2. Set your WiFi credentials:

```cpp
const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";
```

3. Upload to your ESP32 or ESP8266
4. Open **Serial Monitor** (115200 baud) — you'll see the board's IP address:

```
Connected! IP: 192.168.1.42
OrbytAuto ready — configure pins from the Orbyt app.
```

### 3. Connect in the Orbyt App

1. Open Orbyt → **Settings → Discover Devices → Scan Common Subnets**
2. The board appears automatically — tap **Add Device**
3. Or manually: enter the IP and port `80`
4. Tap **Load Device Capabilities** — the app fetches all available pins
5. Add pin configs: pick any GPIO, pick a widget type (toggle / slider / button / readout)
6. Go to **Control** tab — start interacting

**No firmware changes needed for new projects.** Flash once, use forever.

---

## OrbytDevice (Advanced)

Use when you want explicit pin registration and optional callbacks:

```cpp
#include <Orbyt.h>

OrbytDevice orbyt;

void setup() {
  WiFi.begin("SSID", "PASS");
  orbyt.begin("MyBoard", "orbyt-fw-1.0.0", 80);

  // Pins baked into firmware
  orbyt.registerOutput(2, "LED");
  orbyt.registerPwm(5, "Dimmer", 0, 255);

  // Custom logic on write/read
  orbyt.setOnWriteCallback([](uint8_t pin, int value) {
    Serial.printf("Pin %d -> %d\n", pin, value);
  });
}

void loop() {
  orbyt.handle();
}
```

---

## API Reference

### OrbytAuto

```cpp
void begin(const char* deviceName, const char* firmwareVersion, int port = 80);
void handle();                          // Must be called every loop() iteration
void setUptime(unsigned long ms);      // Optional: override reported uptime
```

### OrbytDevice

```cpp
void begin(const char* deviceName, const char* firmwareVersion, int port = 80);
void handle();

int registerPin(uint8_t pin, const String& mode, const String& label = "",
                int minValue = 0, int maxValue = 255);
int registerOutput(uint8_t pin, const String& label = "");
int registerInput(uint8_t pin, const String& label = "");
int registerPwm(uint8_t pin, const String& label = "", int minValue = 0, int maxValue = 255);

bool writePin(uint8_t pin, int value, const String& mode);
OrbytPin::ReadResult readPin(uint8_t pin);

void setOnWriteCallback(std::function<void(uint8_t pin, int value)> cb);
void setOnReadCallback(std::function<int(uint8_t pin)> cb);
```

---

## HTTP API Endpoints

The library exposes a JSON REST API. All responses are JSON.

### `GET /api/v1/device/status`

```json
{
  "connected": true,
  "board": "ESP32",
  "firmware": "orbyt-auto-1.0.0",
  "uptimeMs": 1234000
}
```

### `GET /api/v1/device/capabilities`

(OrbytAuto) Returns all available pins on the board.

```json
{
  "board": "ESP32",
  "supportedPins": [0, 1, 2, ..., 39],
  "pwmPins": [0, 1, 2, ..., 33],
  "registeredPins": []
}
```

(OrbytDevice) Returns only registered pins.

### `POST /api/v1/pin/write`

Body:

```json
{ "pin": 2, "value": 1, "mode": "OUTPUT" }
```

Response:

```json
{ "success": true, "message": "Pin write completed" }
```

### `GET /api/v1/pin/read?pin=34`

```json
{ "pin": 34, "value": 0, "mode": "INPUT" }
```

---

## Pin Quick Reference

| Property | ESP32 | ESP8266 |
|----------|-------|---------|
| OUTPUT capable | All GPIO | All GPIO |
| PWM capable | GPIO 0–23, 25–27, 32–33 | GPIO 0, 2, 4, 5, 12–15 |
| Analog input | GPIO 34–39 | A0 (ADC) |
| Avoid | GPIO 1, 3 (USB Serial) | GPIO 1, 3 (TX/RX) |

> **Note:** ESP32 GPIO 34–39 are input-only and cannot be used for OUTPUT or PWM writes.

---

## Troubleshooting

### "curl: (7) Failed to connect"

- Board and phone/computer must be on the **same WiFi network**
- Try pinging the board: `ping 192.168.1.42`
- Check the Serial Monitor for the connection confirmation and IP

### Board not found by Discover Devices

- Tap **Scan Common Subnets** again — the board may need a few seconds after boot
- Manually add: use the IP from Serial Monitor + port `80`

### Pin writes do nothing

- ESP32: GPIO 34–39 are **input-only** — cannot be used for OUTPUT/PWM
- ESP8266: GPIO 16 lacks PWM support
- Check the `/api/v1/device/capabilities` response to confirm which pins support your chosen mode

### Library not found in Arduino IDE

- Restart Arduino IDE after copying the library folder
- Verify: **Sketch → Include Library → Orbyt** appears

---

## License

MIT License — see [LICENSE](LICENSE) for details.
