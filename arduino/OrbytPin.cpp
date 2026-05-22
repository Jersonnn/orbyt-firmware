#include "OrbytPin.h"

void OrbytPin::configure(int index, uint8_t pin, Mode mode, const String& label,
                          int minValue, int maxValue) {
  _index = index;
  _pin = pin;
  _mode = mode;
  _label = label;
  _minValue = minValue;
  _maxValue = maxValue;
}

// ESP32 supported digital GPIOs (excluding reserved)
static const uint16_t ESP32_SUPPORTED_PINS[] = {
  0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19,
  21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 37, 38, 39
};
static const size_t ESP32_SUPPORTED_COUNT = sizeof(ESP32_SUPPORTED_PINS) / sizeof(ESP32_SUPPORTED_PINS[0]);

// ESP32 PWM-capable GPIOs (all except input-only 34-39)
static const uint16_t ESP32_PWM_PINS[] = {
  0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19,
  21, 22, 23, 25, 26, 27, 32, 33
};
static const size_t ESP32_PWM_COUNT = sizeof(ESP32_PWM_PINS) / sizeof(ESP32_PWM_PINS[0]);

// ESP8266 supported digital GPIOs
static const uint8_t ESP8266_SUPPORTED_PINS[] = { 0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16 };
static const size_t ESP8266_SUPPORTED_COUNT = sizeof(ESP8266_SUPPORTED_PINS) / sizeof(ESP8266_SUPPORTED_PINS[0]);

// ESP8266 PWM-capable GPIOs (all except GPIO16 which lacks PWM)
static const uint8_t ESP8266_PWM_PINS[] = { 0, 2, 4, 5, 12, 13, 14, 15 };
static const size_t ESP8266_PWM_COUNT = sizeof(ESP8266_PWM_PINS) / sizeof(ESP8266_PWM_PINS[0]);

const uint16_t* OrbytPin::supportedPins32(size_t& count) {
  count = ESP32_SUPPORTED_COUNT;
  return ESP32_SUPPORTED_PINS;
}

const uint16_t* OrbytPin::pwmPins32(size_t& count) {
  count = ESP32_PWM_COUNT;
  return ESP32_PWM_PINS;
}

const uint8_t* OrbytPin::supportedPins8266(size_t& count) {
  count = ESP8266_SUPPORTED_COUNT;
  return ESP8266_SUPPORTED_PINS;
}

const uint8_t* OrbytPin::pwmPins8266(size_t& count) {
  count = ESP8266_PWM_COUNT;
  return ESP8266_PWM_PINS;
}
