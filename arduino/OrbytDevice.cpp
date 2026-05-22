#include "OrbytDevice.h"

OrbytDevice::OrbytDevice() {}

void OrbytDevice::begin(const char* deviceName, const char* firmwareVersion, int port) {
  _deviceName = deviceName;
  _firmwareVersion = firmwareVersion;
  _port = port;

#ifdef ESP32
  _server = new WebServer(_port);
  _server->on("/api/v1/device/status", [this]() { _handleStatus(); });
  _server->on("/api/v1/pin/write", HTTP_POST, [this]() { _handlePinWrite(); });
  _server->on("/api/v1/pin/read", [this]() { _handlePinRead(); });
  _server->on("/api/v1/device/capabilities", [this]() { _handleCapabilities(); });
  _server->onNotFound([this]() { _handleNotFound(); });
  _server->begin();

  Serial.print("OrbytDevice ready — ");
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

  Serial.print("OrbytDevice ready — ");
  Serial.print(_deviceName);
  Serial.print(" | IP: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(_port);
#endif
}

void OrbytDevice::handle() {
#ifdef ESP32
  if (_server) _server->handleClient();
#elif defined(ESP8266)
  if (_server) _server->handleClient();
#endif
}

int OrbytDevice::registerPin(uint8_t pin, const String& mode, const String& label, int minValue, int maxValue) {
  if (_pinCount >= ORBYT_MAX_PINS) return -1;

  OrbytPin::Mode m;
  if (mode == "INPUT") m = OrbytPin::Mode::MODE_INPUT;
  else if (mode == "PWM") m = OrbytPin::Mode::MODE_PWM;
  else m = OrbytPin::Mode::MODE_OUTPUT;

  if (m == OrbytPin::Mode::MODE_INPUT) {
    pinMode(pin, INPUT);
  } else {
    pinMode(pin, OUTPUT);
  }

  _pins[_pinCount].configure(_pinCount, pin, m, label, minValue, maxValue);

  if (m == OrbytPin::Mode::MODE_OUTPUT) digitalWrite(pin, LOW);

#ifdef ESP32
  if (m == OrbytPin::Mode::MODE_PWM) {
    ledcAttach(pin, 1000, 8);
  }
#endif

  return _pinCount++;
}

int OrbytDevice::registerOutput(uint8_t pin, const String& label) {
  return registerPin(pin, "OUTPUT", label, 0, 1);
}

int OrbytDevice::registerInput(uint8_t pin, const String& label) {
  return registerPin(pin, "INPUT", label, 0, 1);
}

int OrbytDevice::registerPwm(uint8_t pin, const String& label, int minValue, int maxValue) {
  return registerPin(pin, "PWM", label, minValue, maxValue);
}

OrbytPin::ReadResult OrbytDevice::readPin(uint8_t pin) {
  OrbytPin::ReadResult result = { pin, 0, "OUTPUT" };

  for (int i = 0; i < _pinCount; i++) {
    if (_pins[i].pin() == pin) {
      OrbytPin::Mode m = _pins[i].mode();

      if (m == OrbytPin::Mode::MODE_INPUT) {
        result.value = digitalRead(pin);
        result.mode = "INPUT";
      } else if (m == OrbytPin::Mode::MODE_OUTPUT) {
        result.value = digitalRead(pin);
        result.mode = "OUTPUT";
      } else if (m == OrbytPin::Mode::MODE_PWM) {
#ifdef ESP32
        result.value = ledcRead(pin);
#elif defined(ESP8266)
        result.value = analogRead(pin);
#endif
        result.mode = "PWM";
      }

      if (_onRead) {
        int cbVal = _onRead(pin);
        if (cbVal >= 0) result.value = cbVal;
      }

      return result;
    }
  }
  return result;
}

bool OrbytDevice::writePin(uint8_t pin, int value, const String& mode) {
  for (int i = 0; i < _pinCount; i++) {
    if (_pins[i].pin() == pin) {
      OrbytPin::Mode m;
      if (mode == "INPUT") m = OrbytPin::Mode::MODE_INPUT;
      else if (mode == "PWM") m = OrbytPin::Mode::MODE_PWM;
      else m = OrbytPin::Mode::MODE_OUTPUT;

      if (m == OrbytPin::Mode::MODE_OUTPUT) {
        digitalWrite(pin, value ? HIGH : LOW);
        _pins[i].setLastValue(value);
      } else if (m == OrbytPin::Mode::MODE_PWM) {
#ifdef ESP32
        ledcWrite(pin, value);
#elif defined(ESP8266)
        analogWrite(pin, value);
#endif
        _pins[i].setLastValue(value);
      }

      if (_onWrite) _onWrite(pin, value);
      return true;
    }
  }
  return false;
}

void OrbytDevice::setOnWriteCallback(std::function<void(uint8_t, int)> cb) {
  _onWrite = cb;
}

void OrbytDevice::setOnReadCallback(std::function<int(uint8_t)> cb) {
  _onRead = cb;
}

void OrbytDevice::_handleStatus() {
  _lastRequestMs = millis();
  OrbytJson json;
  String body = json.statusResponse(true, boardType(), _firmwareVersion, uptimeMs());
  _sendJson(200, body);
}

void OrbytDevice::_handlePinWrite() {
  _lastRequestMs = millis();

  String jsonStr;

#ifdef ESP32
  if (!_server->hasArg("plain")) {
    OrbytJson json;
    _sendJson(400, json.errorResponse("Missing request body"));
    return;
  }
  jsonStr = _server->arg("plain");
#elif defined(ESP8266)
  if (!_server->hasArg("plain")) {
    OrbytJson json;
    _sendJson(400, json.errorResponse("Missing request body"));
    return;
  }
  jsonStr = _server->arg("plain");
#endif

  int pin = -1;
  int value = 0;
  String mode = "OUTPUT";

  int pinIdx = jsonStr.indexOf("\"pin\"");
  int valIdx = jsonStr.indexOf("\"value\"");
  int modeIdx = jsonStr.indexOf("\"mode\"");

  if (pinIdx >= 0) {
    int start = jsonStr.indexOf(":", pinIdx) + 1;
    int end = jsonStr.indexOf(",", start);
    if (end < 0) end = jsonStr.indexOf("}", start);
    String pinStr = jsonStr.substring(start, end);
    pinStr.trim();
    pin = pinStr.toInt();
  }

  if (valIdx >= 0) {
    int start = jsonStr.indexOf(":", valIdx) + 1;
    int end = jsonStr.indexOf(",", start);
    if (end < 0) end = jsonStr.indexOf("}", start);
    String valStr = jsonStr.substring(start, end);
    valStr.trim();
    value = valStr.toInt();
  }

  if (modeIdx >= 0) {
    int start = jsonStr.indexOf("\"", jsonStr.indexOf("\"", modeIdx) + 1) + 1;
    int end = jsonStr.indexOf("\"", start);
    if (end > start) mode = jsonStr.substring(start, end);
  }

  if (pin < 0) {
    OrbytJson json;
    _sendJson(400, json.errorResponse("Invalid pin"));
    return;
  }

  bool ok = writePin((uint8_t)pin, value, mode);
  OrbytJson json;
  String body = json.writeResponse(ok, ok ? "Pin write completed" : "Pin not found");
  _sendJson(ok ? 200 : 404, body);
}

void OrbytDevice::_handlePinRead() {
  _lastRequestMs = millis();

  int pin = -1;
#ifdef ESP32
  if (_server->hasArg("pin")) pin = _server->arg("pin").toInt();
#elif defined(ESP8266)
  if (_server->hasArg("pin")) pin = _server->arg("pin").toInt();
#endif

  if (pin < 0) {
    OrbytJson json;
    _sendJson(400, json.errorResponse("Missing pin parameter"));
    return;
  }

  OrbytPin::ReadResult result = readPin((uint8_t)pin);
  OrbytJson json;
  String body = json.readResponse(result);
  _sendJson(200, body);
}

void OrbytDevice::_handleCapabilities() {
  _lastRequestMs = millis();
  OrbytJson json;
  String body = json.capabilitiesResponse(_pins, _pinCount);
  _sendJson(200, body);
}

void OrbytDevice::_handleNotFound() {
  _lastRequestMs = millis();
  OrbytJson json;
  _sendJson(404, json.errorResponse("Not found"));
}

void OrbytDevice::_sendJson(int code, const String& body) {
#ifdef ESP32
  _server->send(code, "application/json", body);
#elif defined(ESP8266)
  _server->send(code, "application/json", body);
#endif
}
