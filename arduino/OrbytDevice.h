#pragma once

#include <Arduino.h>

#ifdef ESP32
  #include <WiFi.h>
  #include <WebServer.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
#else
  #error "Orbyt requires ESP32 or ESP8266"
#endif

#include "OrbytJson.h"
#include "OrbytPin.h"

#define ORBYT_DEFAULT_PORT 80
#define ORBYT_MAX_PINS 32
#define ORBYT_DEFAULT_TIMEOUT_MS 2500

class OrbytDevice {
public:
  OrbytDevice();

  void begin(const char* deviceName = "Orbyt Device",
             const char* firmwareVersion = "orbyt-fw-1.0.0",
             int port = ORBYT_DEFAULT_PORT);

  void handle();

  // Register a pin. Call from setup() after begin().
  // Returns index (>=0) on success, -1 on full.
  int registerPin(uint8_t pin, const String& mode, const String& label = "",
                  int minValue = 0, int maxValue = 255);
  int registerOutput(uint8_t pin, const String& label = "");
  int registerInput(uint8_t pin, const String& label = "");
  int registerPwm(uint8_t pin, const String& label = "", int minValue = 0, int maxValue = 255);

  // Write a pin value from the app
  bool writePin(uint8_t pin, int value, const String& mode);

  // Read a pin value for the app
  OrbytPin::ReadResult readPin(uint8_t pin);

  // Return registered pins for /capabilities endpoint
  OrbytPin* getPinByIndex(int idx);
  int getPinCount() const { return _pinCount; }

  // Status
  bool isConnected() const { return _lastRequestMs > 0 && (millis() - _lastRequestMs) < 5000; }
  unsigned long uptimeMs() const { return millis(); }

  const char* deviceName() const { return _deviceName; }
  const char* firmwareVersion() const { return _firmwareVersion; }

#ifdef ESP32
  String boardType() const { return "ESP32"; }
#elif defined(ESP8266)
  String boardType() const { return "ESP8266"; }
#endif

  void setOnWriteCallback(std::function<void(uint8_t pin, int value)> cb);
  void setOnReadCallback(std::function<int(uint8_t pin)> cb);

private:
  const char* _deviceName;
  const char* _firmwareVersion;
  int _port;

  OrbytPin _pins[ORBYT_MAX_PINS];
  int _pinCount = 0;

  unsigned long _lastRequestMs = 0;

  std::function<void(uint8_t, int)> _onWrite;
  std::function<int(uint8_t)> _onRead;

#ifdef ESP32
  WebServer* _server = nullptr;
#elif defined(ESP8266)
  ESP8266WebServer* _server = nullptr;
#endif

  void _handleStatus();
  void _handlePinWrite();
  void _handlePinRead();
  void _handleCapabilities();
  void _handleNotFound();
  void _sendJson(int code, const String& body);
};
