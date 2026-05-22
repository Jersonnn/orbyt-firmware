import json
import network
import socket
import time
from machine import Pin, PWM

WIFI_SSID = "YourWiFiSSID"
WIFI_PASSWORD = "YourWiFiPassword"
BOARD_MODEL = "Raspberry Pi Pico W"
DEVICE_FAMILY = "rp2040-wifi"
PORT = 80

SUPPORTED_PINS = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
PWM_PINS = SUPPORTED_PINS[:]
ANALOG_PINS = [26, 27, 28]

started_at = time.ticks_ms()
pin_modes = {}
pin_values = {}
pwm_channels = {}


def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        wlan.connect(WIFI_SSID, WIFI_PASSWORD)
        while not wlan.isconnected():
            time.sleep(0.2)
    return wlan.ifconfig()[0]


def response(status, payload):
    body = json.dumps(payload)
    return (
        "HTTP/1.1 {} OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: {}\r\n"
        "Connection: close\r\n\r\n{}"
    ).format(status, len(body), body)


def parse_body(request):
    parts = request.split("\r\n\r\n", 1)
    if len(parts) != 2 or not parts[1]:
        return {}
    try:
        return json.loads(parts[1])
    except ValueError:
        return {}


def status_payload():
    return {
        "connected": True,
        "apiVersion": "1.1",
        "deviceFamily": DEVICE_FAMILY,
        "boardModel": BOARD_MODEL,
        "board": BOARD_MODEL,
        "firmware": "orbyt-micropython-1.0.0",
        "uptimeMs": time.ticks_diff(time.ticks_ms(), started_at),
    }


def capabilities_payload():
    return {
        "apiVersion": "1.1",
        "deviceFamily": DEVICE_FAMILY,
        "boardModel": BOARD_MODEL,
        "board": BOARD_MODEL,
        "transport": "http",
        "features": ["digital-read", "digital-write", "pwm-write"],
        "supportedPins": SUPPORTED_PINS,
        "digitalPins": SUPPORTED_PINS,
        "analogPins": ANALOG_PINS,
        "pwmPins": PWM_PINS,
        "registeredPins": [],
        "notes": "Edit pin lists for your specific MicroPython board.",
    }


def write_pin(payload):
    pin = int(payload.get("pin", -1))
    value = int(payload.get("value", 0))
    mode = payload.get("mode", "OUTPUT")
    if pin not in SUPPORTED_PINS:
        return 400, {"success": False, "message": "Unsupported pin"}
    if mode == "PWM":
        if pin not in PWM_PINS:
            return 400, {"success": False, "message": "PWM unsupported for pin"}
        pwm = pwm_channels.get(pin)
        if pwm is None:
            pwm = PWM(Pin(pin))
            pwm.freq(1000)
            pwm_channels[pin] = pwm
        pwm.duty_u16(max(0, min(65535, value * 257)))
    elif mode == "INPUT":
        Pin(pin, Pin.IN)
    else:
        Pin(pin, Pin.OUT).value(1 if value else 0)
    pin_modes[pin] = mode
    pin_values[pin] = value
    return 200, {"success": True, "message": "Pin write completed"}


def read_pin(pin):
    pin = int(pin)
    if pin not in SUPPORTED_PINS and pin not in ANALOG_PINS:
        return 400, {"message": "Unsupported pin"}
    mode = pin_modes.get(pin, "INPUT")
    value = pin_values.get(pin, Pin(pin, Pin.IN).value())
    return 200, {"pin": pin, "value": value, "mode": mode}


def handle(request):
    first = request.split("\r\n", 1)[0]
    if first.startswith("GET /api/v1/device/status"):
        return response(200, status_payload())
    if first.startswith("GET /api/v1/device/capabilities"):
        return response(200, capabilities_payload())
    if first.startswith("POST /api/v1/pin/write"):
        status, payload = write_pin(parse_body(request))
        return response(status, payload)
    if first.startswith("GET /api/v1/pin/read"):
        query = first.split("?", 1)[1].split(" ", 1)[0] if "?" in first else ""
        pin = dict(part.split("=", 1) for part in query.split("&") if "=" in part).get("pin", "-1")
        status, payload = read_pin(pin)
        return response(status, payload)
    return response(404, {"message": "Not found"})


def start():
    ip = connect_wifi()
    addr = socket.getaddrinfo("0.0.0.0", PORT)[0][-1]
    server = socket.socket()
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(addr)
    server.listen(2)
    print("Orbyt MicroPython ready at http://{}:{}".format(ip, PORT))
    while True:
        client, _ = server.accept()
        request = client.recv(2048).decode()
        client.send(handle(request))
        client.close()


start()

