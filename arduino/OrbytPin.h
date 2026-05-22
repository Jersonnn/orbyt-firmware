#pragma once

#include <Arduino.h>

class OrbytPin {
public:
  enum class Mode { MODE_INPUT, MODE_OUTPUT, MODE_PWM };

  OrbytPin() {}

  void configure(int index, uint8_t pin, Mode mode, const String& label,
                 int minValue = 0, int maxValue = 255);

  uint8_t pin() const { return _pin; }
  Mode mode() const { return _mode; }
  const String& label() const { return _label; }
  int minValue() const { return _minValue; }
  int maxValue() const { return _maxValue; }
  int lastValue() const { return _lastValue; }
  void setLastValue(int v) { _lastValue = v; }
  int index() const { return _index; }

  static const uint16_t* supportedPins32(size_t& count);
  static const uint16_t* pwmPins32(size_t& count);
  static const uint8_t* supportedPins8266(size_t& count);
  static const uint8_t* pwmPins8266(size_t& count);

  struct ReadResult {
    uint8_t pin;
    int value;
    String mode;
  };

private:
  int _index = -1;
  uint8_t _pin = 0;
  Mode _mode = Mode::MODE_OUTPUT;
  String _label;
  int _minValue = 0;
  int _maxValue = 255;
  int _lastValue = 0;
};
