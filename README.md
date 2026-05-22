# Orbyt Firmware

Official firmware libraries, templates, and local adapters for devices that work with the Orbyt app.

Orbyt is local-first. A device does not need a cloud account or proprietary broker to be controlled by the app. It only needs to expose the Orbyt JSON API over HTTP on the same local network.

## What To Use

| Device or runtime | Use this package | Status |
| --- | --- | --- |
| ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C5, ESP32-C6 | [`arduino/`](./arduino) | Official library where Arduino networking is available |
| ESP8266, NodeMCU, Wemos D1 mini | [`arduino/`](./arduino) | Official library |
| Raspberry Pi Pico W, Pico 2 W | [`micropython/`](./micropython) | Template |
| ESP32 running MicroPython | [`micropython/`](./micropython) | Template |
| Wi-Fi capable CircuitPython boards | [`circuitpython/`](./circuitpython) | Template |
| Raspberry Pi, Orange Pi, Linux gateway, desktop bridge | [`node/`](./node) | Adapter |
| Any custom board with local HTTP support | Implement the API below | Custom HTTP |

Not supported natively in this repo: Bluetooth, Zigbee, Thread, LoRa, Z-Wave, MQTT, Matter, Home Assistant, Tasmota, and cloud-only devices. Those can still work later through a local adapter that exposes the Orbyt HTTP API.

## Quick Start

1. Pick the package that matches your board.
2. Flash or run the firmware/template.
3. Put your phone and device on the same Wi-Fi network.
4. Open Orbyt.
5. Go to `Settings -> Discover Devices -> Scan Common Subnets`, or add the device manually by IP and port.
6. In the project screen, select `Load Device Capabilities`.
7. Add pin controls and use the Control screen.

Default HTTP port: `80`.

## Arduino Setup

Use this for ESP32 and ESP8266 boards.

1. Copy the [`arduino/`](./arduino) folder into your Arduino libraries folder as `Orbyt`.

```text
Arduino/libraries/Orbyt/
  library.properties
  OrbytAuto.h
  OrbytAuto.cpp
  OrbytDevice.h
  OrbytDevice.cpp
  OrbytPin.h
  OrbytPin.cpp
  OrbytJson.h
  OrbytJson.cpp
  examples/
```

2. Restart Arduino IDE.
3. Open `File -> Examples -> Orbyt -> OrbytAuto`.
4. Set your Wi-Fi credentials.
5. Upload to the board.
6. Open Serial Monitor at `115200` baud and copy the printed IP address.

Minimal sketch:

```cpp
#include <WiFi.h>
#include <OrbytAuto.h>

const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";

OrbytAuto board;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  Serial.println(WiFi.localIP());
  board.begin("My Orbyt Board", "orbyt-auto-1.0.0", 80);
}

void loop() {
  board.handle();
}
```

Use `OrbytAuto` for most boards. Use `OrbytDevice` when the firmware should register fixed pins or run custom callbacks.

## MicroPython Setup

Use this for Raspberry Pi Pico W, Pico 2 W, or ESP32 boards running MicroPython.

1. Copy [`micropython/orbyt_server.py`](./micropython/orbyt_server.py) to the board.
2. Edit:
   - `WIFI_SSID`
   - `WIFI_PASSWORD`
   - `BOARD_MODEL`
   - `DEVICE_FAMILY`
   - `SUPPORTED_PINS`
   - `PWM_PINS`
   - `ANALOG_PINS`
3. Save it as `main.py` if you want it to start on boot.
4. Reset the board.
5. Add the printed IP address in Orbyt.

## CircuitPython Setup

Use this as a starting point for Wi-Fi capable CircuitPython boards.

The [`circuitpython/code.py`](./circuitpython/code.py) file defines the Orbyt payloads and function shape. CircuitPython HTTP server support varies by board, so wire those functions into the board-specific HTTP server library you use.

Required handlers:

- `GET /api/v1/device/status` returns `status_payload()`
- `GET /api/v1/device/capabilities` returns `capabilities_payload()`
- `POST /api/v1/pin/write` calls `write_pin(pin, value, mode)`
- `GET /api/v1/pin/read?pin=<number>` calls `read_pin(pin)`

## Node Adapter Setup

Use this for Raspberry Pi, Orange Pi, Linux gateways, development machines, and bridge devices.

```bash
cd node
node server.js
```

By default the adapter simulates GPIO. Replace `readPin` and `writePin` in [`node/server.js`](./node/server.js) with real GPIO bindings such as `onoff`, `pigpio`, a serial bridge, or a vendor SDK.

Environment variables:

```bash
ORBYT_PORT=80
ORBYT_BOARD_MODEL="Raspberry Pi Adapter"
ORBYT_SUPPORTED_PINS=0,1,2,3,4,5,6,7
ORBYT_PWM_PINS=1,2,3
```

## Orbyt JSON API v1.1

Every official package and custom adapter should expose these endpoints.

### Device Status

```http
GET /api/v1/device/status
```

```json
{
  "connected": true,
  "apiVersion": "1.1",
  "deviceFamily": "esp32",
  "boardModel": "ESP32-S3",
  "board": "ESP32-S3",
  "firmware": "orbyt-auto-1.0.0",
  "uptimeMs": 124920
}
```

### Device Capabilities

```http
GET /api/v1/device/capabilities
```

```json
{
  "apiVersion": "1.1",
  "deviceFamily": "rp2040-wifi",
  "boardModel": "Raspberry Pi Pico W",
  "board": "Raspberry Pi Pico W",
  "transport": "http",
  "features": ["digital-read", "digital-write", "pwm-write"],
  "supportedPins": [0, 1, 2, 3, 4, 5],
  "digitalPins": [0, 1, 2, 3, 4, 5],
  "analogPins": [26, 27, 28],
  "pwmPins": [0, 1, 2, 3, 4, 5],
  "registeredPins": []
}
```

Known `deviceFamily` values:

- `esp8266`
- `esp32`
- `arduino-wifi`
- `rp2040-wifi`
- `linux-adapter`
- `custom-http`

### Write Pin

```http
POST /api/v1/pin/write
Content-Type: application/json
```

```json
{
  "pin": 2,
  "value": 1,
  "mode": "OUTPUT"
}
```

```json
{
  "success": true,
  "message": "Pin write completed"
}
```

Supported modes:

- `INPUT`
- `OUTPUT`
- `PWM`

### Read Pin

```http
GET /api/v1/pin/read?pin=2
```

```json
{
  "pin": 2,
  "value": 1,
  "mode": "OUTPUT"
}
```

## Troubleshooting

If Orbyt cannot find the device:

- Confirm the phone and device are on the same Wi-Fi network.
- Confirm the device IP address from Serial Monitor, REPL, or terminal logs.
- Open `http://DEVICE_IP/api/v1/device/status` from a browser on the same network.
- Make sure the HTTP port matches the port in Orbyt.
- Try manual device entry instead of subnet scan.

If pin writes do nothing:

- Confirm the pin is in `supportedPins`.
- Confirm the pin supports `PWM` before using a slider.
- Check whether the board uses GPIO numbers instead of printed board labels.
- For adapters, confirm `writePin` is connected to real GPIO or hardware logic.

## License
 
MIT License. See [`LICENSE`](./LICENSE).
 