#include "OrbytAuto.h"

// ─── Board Pin Tables ─────────────────────────────────────────────────────────

const uint8_t OrbytAuto::ESP32_PINS[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 37, 38, 39
};
const uint8_t OrbytAuto::ESP32_PWM_PINS[] = {
  0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19,
  21, 22, 23, 25, 26, 27, 32, 33
};
const uint8_t OrbytAuto::ESP8266_PINS[] = { 0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16 };
const uint8_t OrbytAuto::ESP8266_PWM_PINS[] = { 0, 2, 4, 5, 12, 13, 14, 15 };

size_t OrbytAuto::esp32PinCount() { return sizeof(ESP32_PINS) / sizeof(ESP32_PINS[0]); }
size_t OrbytAuto::esp32PwmPinCount() { return sizeof(ESP32_PWM_PINS) / sizeof(ESP32_PWM_PINS[0]); }
size_t OrbytAuto::esp8266PinCount() { return sizeof(ESP8266_PINS) / sizeof(ESP8266_PINS[0]); }
size_t OrbytAuto::esp8266PwmPinCount() { return sizeof(ESP8266_PWM_PINS) / sizeof(ESP8266_PWM_PINS[0]); }

// ─── Constructor ───────────────────────────────────────────────────────────────

OrbytAuto::OrbytAuto() {}

// ─── begin ────────────────────────────────────────────────────────────────────

void OrbytAuto::begin(const char* deviceName, const char* firmwareVersion, int port) {
  _deviceName = deviceName;
  _firmwareVersion = firmwareVersion;
  _port = port;
  _startMs = millis();

#ifdef ESP32
  _server = new WebServer(_port);
  _server->on("/api/v1/device/status", [this]() { _handleStatus(); });
  _server->on("/api/v1/pin/write", HTTP_POST, [this]() { _handlePinWrite(); });
  _server->on("/api/v1/pin/read", [this]() { _handlePinRead(); });
  _server->on("/api/v1/device/capabilities", [this]() { _handleCapabilities(); });
  _server->onNotFound([this]() { _handleNotFound(); });
  _server->begin();

  Serial.print("OrbytAuto ready — ");
  Serial.print(_deviceName);
  Serial.print(" | IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(_port);
#elif defined(ESP8266)
  _server = new ESP8266WebServer(_port);
  _server->on("/api/v1/device/status", [this]() { _handleStatus(); });
  _server->on("/api/v1/pin/write", HTTP_POST, [this]() { _handlePinWrite(); });
  _server->on("/api/v1/pin/read", [this]() { _handlePinRead(); });
  _server->on("/api/v1/device/capabilities", [this]() { _handleCapabilities(); });
  _server->onNotFound([this]() { _handleNotFound(); });
  _server->begin();

  Serial.print("OrbytAuto ready — ");
  Serial.print(_deviceName);
  Serial.print(" | IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(_port);
#endif
}

void OrbytAuto::handle() {
#ifdef ESP32
  if (_server) _server->handleClient();
#elif defined(ESP8266)
  if (_server) _server->handleClient();
#endif
}

void OrbytAuto::setUptime(unsigned long ms) {
  _customUptime = ms;
  _useCustomUptime = true;
}

// ─── Pin Config (on-the-fly) ─────────────────────────────────────────────────

void OrbytAuto::_configPin(uint8_t pin, const String& mode) {
  if (mode == "INPUT") {
    pinMode(pin, INPUT);
  } else if (mode == "PWM") {
#ifdef ESP32
    ledcAttach(pin, 1000, 8);
#elif defined(ESP8266)
    pinMode(pin, OUTPUT);
#endif
  } else {
    pinMode(pin, OUTPUT);
  }
}

// ─── Pin Read (on-the-fly) ────────────────────────────────────────────────────

int OrbytAuto::_readPin(uint8_t pin, const String& mode) {
  if (mode == "INPUT") {
    return digitalRead(pin);
  } else if (mode == "PWM") {
#ifdef ESP32
    return digitalRead(pin);  // PWM output reads back last written value
#elif defined(ESP8266)
    return digitalRead(pin);
#endif
  } else {
    return digitalRead(pin);
  }
}

// ─── Pin Write (on-the-fly) ─────────────────────────────────────────────────

void OrbytAuto::_writePin(uint8_t pin, int value, const String& mode) {
  if (mode == "INPUT") return;  // can't write to input

  if (mode == "PWM") {
#ifdef ESP32
    ledcWrite(pin, value);
#elif defined(ESP8266)
    analogWrite(pin, value);
#endif
  } else {
    digitalWrite(pin, value ? HIGH : LOW);
  }
}

// ─── Handlers ────────────────────────────────────────────────────────────────

void OrbytAuto::_handleStatus() {
  unsigned long uptime = _useCustomUptime ? _customUptime : (millis() - _startMs);
  String board =
#ifdef ESP32
    "ESP32";
#elif defined(ESP8266)
    "ESP8266";
#endif
  String family =
#ifdef ESP32
    "esp32";
#elif defined(ESP8266)
    "esp8266";
#endif
  String body = "{\"connected\":true,"
                "\"apiVersion\":\"1.1\","
                "\"deviceFamily\":\"" + family + "\","
                "\"boardModel\":\"" + board + "\","
                "\"board\":\"" + board + "\","
                "\"firmware\":\"" + String(_firmwareVersion) + "\","
                "\"uptimeMs\":" + String(uptime) + "}";
  _sendJson(200, body);
}

void OrbytAuto::_handlePinWrite() {
  String jsonStr;

#ifdef ESP32
  if (!_server->hasArg("plain") || _server->arg("plain").length() == 0) {
    _sendJson(400, "{\"error\":\"Missing request body\"}");
    return;
  }
  jsonStr = _server->arg("plain");
#elif defined(ESP8266)
  if (!_server->hasArg("plain") || _server->arg("plain").length() == 0) {
    _sendJson(400, "{\"error\":\"Missing request body\"}");
    return;
  }
  jsonStr = _server->arg("plain");
#endif

  // Minimal JSON parser — no external dependencies
  int pin = -1, value = 0;
  String mode = "OUTPUT";

  int pinIdx = jsonStr.indexOf("\"pin\"");
  int valIdx = jsonStr.indexOf("\"value\"");
  int modeIdx = jsonStr.indexOf("\"mode\"");

  if (pinIdx >= 0) {
    int colon = jsonStr.indexOf(":", pinIdx);
    int end = jsonStr.indexOf(",", colon);
    if (end < 0) end = jsonStr.indexOf("}", colon);
    String pinStr = jsonStr.substring(colon + 1, end);
    pinStr.trim();
    pin = pinStr.toInt();
  }
  if (valIdx >= 0) {
    int colon = jsonStr.indexOf(":", valIdx);
    int end = jsonStr.indexOf(",", colon);
    if (end < 0) end = jsonStr.indexOf("}", colon);
    String valStr = jsonStr.substring(colon + 1, end);
    valStr.trim();
    value = valStr.toInt();
  }
  if (modeIdx >= 0) {
    int start = jsonStr.indexOf("\"", modeIdx) + 1;
    start = jsonStr.indexOf("\"", start) + 1;
    int end = jsonStr.indexOf("\"", start);
    mode = jsonStr.substring(start, end);
  }

  if (pin < 0) {
    _sendJson(400, "{\"error\":\"Invalid pin\"}");
    return;
  }

  // Validate pin number against board
#ifdef ESP32
  bool valid = false;
  for (size_t i = 0; i < esp32PinCount(); i++) {
    if (ESP32_PINS[i] == pin) { valid = true; break; }
  }
  if (!valid) {
    _sendJson(400, "{\"error\":\"Unsupported pin\"}");
    return;
  }
#elif defined(ESP8266)
  bool valid = false;
  for (size_t i = 0; i < esp8266PinCount(); i++) {
    if (ESP8266_PINS[i] == pin) { valid = true; break; }
  }
  if (!valid) {
    _sendJson(400, "{\"error\":\"Unsupported pin\"}");
    return;
  }
#endif

  // Config + write in one step
  _configPin((uint8_t)pin, mode);
  _writePin((uint8_t)pin, value, mode);

  _sendJson(200, "{\"success\":true,\"message\":\"Pin write completed\"}");
}

void OrbytAuto::_handlePinRead() {
  int pin = -1;
#ifdef ESP32
  if (_server->hasArg("pin")) pin = _server->arg("pin").toInt();
#elif defined(ESP8266)
  if (_server->hasArg("pin")) pin = _server->arg("pin").toInt();
#endif

  if (pin < 0) {
    _sendJson(400, "{\"error\":\"Missing pin parameter\"}");
    return;
  }

  // For reads we default to INPUT mode since we don't track pin modes
  int val = _readPin((uint8_t)pin, "INPUT");
  String body = "{\"pin\":" + String(pin) +
                ",\"value\":" + String(val) +
                ",\"mode\":\"INPUT\"}";
  _sendJson(200, body);
}

void OrbytAuto::_handleCapabilities() {
  unsigned long uptime = _useCustomUptime ? _customUptime : (millis() - _startMs);
  String board =
#ifdef ESP32
    "ESP32";
#elif defined(ESP8266)
    "ESP8266";
#endif
  String family =
#ifdef ESP32
    "esp32";
#elif defined(ESP8266)
    "esp8266";
#endif

  String body = "{\"board\":\"" + board + "\",";
  body += "\"apiVersion\":\"1.1\",";
  body += "\"deviceFamily\":\"" + family + "\",";
  body += "\"boardModel\":\"" + board + "\",";
  body += "\"transport\":\"http\",";
  body += "\"features\":[\"digital-read\",\"digital-write\",\"pwm-write\"],";

  // All supported pins
  body += "\"supportedPins\":[";
#ifdef ESP32
  for (size_t i = 0; i < esp32PinCount(); i++) {
    if (i > 0) body += ",";
    body += String(ESP32_PINS[i]);
  }
  body += "],";
  body += "\"pwmPins\":[";
  for (size_t i = 0; i < esp32PwmPinCount(); i++) {
    if (i > 0) body += ",";
    body += String(ESP32_PWM_PINS[i]);
  }
#elif defined(ESP8266)
  for (size_t i = 0; i < esp8266PinCount(); i++) {
    if (i > 0) body += ",";
    body += String(ESP8266_PINS[i]);
  }
  body += "],";
  body += "\"pwmPins\":[";
  for (size_t i = 0; i < esp8266PwmPinCount(); i++) {
    if (i > 0) body += ",";
    body += String(ESP8266_PWM_PINS[i]);
  }
#endif
  body += "],";

  // No registered pins — all pins are free for the app to claim
  body += "\"digitalPins\":[";
#ifdef ESP32
  for (size_t i = 0; i < esp32PinCount(); i++) {
    if (i > 0) body += ",";
    body += String(ESP32_PINS[i]);
  }
#elif defined(ESP8266)
  for (size_t i = 0; i < esp8266PinCount(); i++) {
    if (i > 0) body += ",";
    body += String(ESP8266_PINS[i]);
  }
#endif
  body += "],";
  body += "\"analogPins\":[";
#ifdef ESP32
  const uint8_t analogPins[] = {32, 33, 34, 35, 36, 37, 38, 39};
  for (size_t i = 0; i < sizeof(analogPins) / sizeof(analogPins[0]); i++) {
    if (i > 0) body += ",";
    body += String(analogPins[i]);
  }
#elif defined(ESP8266)
  body += "17";
#endif
  body += "],";
  body += "\"registeredPins\":[],";
  body += "\"uptimeMs\":" + String(uptime) + ",";
  body += "\"message\":\"All pins available — configure from the Orbyt app\"";
  body += "}";

  _sendJson(200, body);
}

void OrbytAuto::_handleNotFound() {
  _sendJson(404, "{\"error\":\"Not found\"}");
}

void OrbytAuto::_sendJson(int code, const String& body) {
#ifdef ESP32
  _server->send(code, "application/json", body);
#elif defined(ESP8266)
  _server->send(code, "application/json", body);
#endif
}
