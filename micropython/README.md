# Orbyt MicroPython

Reference Orbyt JSON API v1.1 template for Wi-Fi capable MicroPython boards such as Raspberry Pi Pico W, Pico 2 W, and ESP32 boards running MicroPython.

## Use

1. Copy `orbyt_server.py` to the board.
2. Edit `WIFI_SSID`, `WIFI_PASSWORD`, `BOARD_MODEL`, and pin lists.
3. Run it as `main.py` or import `start()`.
4. Add the board in Orbyt as `Pico W / RP2040 Wi-Fi`, `ESP32 family`, or `Custom HTTP`.

The template implements:

- `GET /api/v1/device/status`
- `GET /api/v1/device/capabilities`
- `POST /api/v1/pin/write`
- `GET /api/v1/pin/read?pin=<number>`

