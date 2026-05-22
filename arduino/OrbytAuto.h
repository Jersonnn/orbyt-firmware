#pragma once

#include <Arduino.h>

#ifdef ESP32
  #include <WiFi.h>
  #include <WebServer.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
#else
  #error "OrbytAuto requires ESP32 or ESP8266"
#endif

/**
 * OrbytAuto — Zero-configuration Orbyt device.
 *
 * Unlike OrbytDevice, this requires NO pin registration.
 * The app drives everything — configure pins, write values, read sensors.
 * Just flash this once and control everything from the Orbyt app.
 *
 * Usage:
 *   #include <OrbytAuto.h>
 *   OrbytAuto board;
 *   board.begin("MyBoard", "orbyt-fw-1.0.0");
 *   in loop(): board.handle();
 */
class OrbytAuto {
public:
  OrbytAuto();

  // Start the HTTP server. Call from setup().
  // deviceName: shown in the Orbyt app
  // firmwareVersion: reported in /api/v1/device/status
  // port: HTTP server port (default 80)
  void begin(const char* deviceName = "OrbytAuto",
             const char* firmwareVersion = "orbyt-auto-1.0.0",
             int port = 80);

  // Must be called every loop() iteration to process HTTP requests
  void handle();

  // Optional: keep the board awake during heavy config
  void setUptime(unsigned long ms);

private:
  const char* _deviceName;
  const char* _firmwareVersion;
  int _port = 80;

  unsigned long _startMs = 0;
  unsigned long _customUptime = 0;
  bool _useCustomUptime = false;

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

  // Low-level pin operations — no prior config needed
  void _configPin(uint8_t pin, const String& mode);
  int  _readPin(uint8_t pin, const String& mode);
  void _writePin(uint8_t pin, int value, const String& mode);

  // Board pin tables
  static const uint8_t ESP32_PINS[];
  static const uint8_t ESP32_PWM_PINS[];
  static const uint8_t ESP8266_PINS[];
  static const uint8_t ESP8266_PWM_PINS[];

  static size_t esp32PinCount() ;
  static size_t esp32PwmPinCount();
  static size_t esp8266PinCount();
  static size_t esp8266PwmPinCount();
};
