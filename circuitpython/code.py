"""Orbyt CircuitPython template.

Install board-specific Wi-Fi and HTTP server libraries, then wire these payloads
to GET/POST handlers for the Orbyt JSON API v1.1 endpoints.
"""

import time

BOARD_MODEL = "CircuitPython Wi-Fi Board"
DEVICE_FAMILY = "custom-http"
SUPPORTED_PINS = []
PWM_PINS = []
ANALOG_PINS = []
STARTED_AT = time.monotonic()


def status_payload():
    return {
        "connected": True,
        "apiVersion": "1.1",
        "deviceFamily": DEVICE_FAMILY,
        "boardModel": BOARD_MODEL,
        "board": BOARD_MODEL,
        "firmware": "orbyt-circuitpython-template-1.0.0",
        "uptimeMs": int((time.monotonic() - STARTED_AT) * 1000),
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
        "notes": "Map these payloads to your CircuitPython HTTP server routes.",
    }


def write_pin(pin, value, mode):
    return {"success": False, "message": "Implement board-specific pin writes"}


def read_pin(pin):
    return {"pin": pin, "value": 0, "mode": "INPUT"}

