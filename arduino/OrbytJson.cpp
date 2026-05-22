#include "OrbytJson.h"

String OrbytJson::statusResponse(bool connected, const String& board,
                                 const String& firmware, unsigned long uptimeMs) {
  String family =
#ifdef ESP32
    "esp32";
#elif defined(ESP8266)
    "esp8266";
#endif
  return "{\"connected\":" + String(connected ? "true" : "false") +
         ",\"apiVersion\":\"1.1\"" +
         ",\"deviceFamily\":\"" + family + "\"" +
         ",\"boardModel\":\"" + board + "\"" +
         ",\"board\":\"" + board +
         "\",\"firmware\":\"" + firmware +
         "\",\"uptimeMs\":" + String(uptimeMs) + "}";
}

String OrbytJson::writeResponse(bool success, const String& message) {
  return "{\"success\":" + String(success ? "true" : "false") +
         ",\"message\":\"" + message + "\"}";
}

String OrbytJson::readResponse(const OrbytPin::ReadResult& result) {
  return "{\"pin\":" + String(result.pin) +
         ",\"value\":" + String(result.value) +
         ",\"mode\":\"" + result.mode + "\"}";
}

String OrbytJson::errorResponse(const String& message) {
  return "{\"error\":\"" + message + "\"}";
}

String OrbytJson::capabilitiesResponse(OrbytPin* registeredPins, int registeredCount) {
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
  String json = "{";
  json += "\"board\":\"" + board + "\",";
  json += "\"apiVersion\":\"1.1\",";
  json += "\"deviceFamily\":\"" + family + "\",";
  json += "\"boardModel\":\"" + board + "\",";
  json += "\"transport\":\"http\",";
  json += "\"features\":[\"digital-read\",\"digital-write\",\"pwm-write\"],";

  // Build supported pins array from board defaults
  size_t supCount = 0, pwmCount = 0;
  #ifdef ESP32
    const uint16_t* sup = OrbytPin::supportedPins32(supCount);
    const uint16_t* pwm = OrbytPin::pwmPins32(pwmCount);
  #elif defined(ESP8266)
    const uint8_t* sup = OrbytPin::supportedPins8266(supCount);
    const uint8_t* pwm = OrbytPin::pwmPins8266(pwmCount);
  #endif

  json += "\"supportedPins\":[";
  for (size_t i = 0; i < supCount; i++) {
    if (i > 0) json += ",";
    json += String(sup[i]);
  }
  json += "],";

  json += "\"pwmPins\":[";
  for (size_t i = 0; i < pwmCount; i++) {
    if (i > 0) json += ",";
    json += String(pwm[i]);
  }
  json += "],";

  json += "\"digitalPins\":[";
  for (size_t i = 0; i < supCount; i++) {
    if (i > 0) json += ",";
    json += String(sup[i]);
  }
  json += "],";

  json += "\"analogPins\":[";
  #ifdef ESP32
    const uint8_t analogPins[] = {32, 33, 34, 35, 36, 37, 38, 39};
    for (size_t i = 0; i < sizeof(analogPins) / sizeof(analogPins[0]); i++) {
      if (i > 0) json += ",";
      json += String(analogPins[i]);
    }
  #elif defined(ESP8266)
    json += "17";
  #endif
  json += "],";

  json += "\"registeredPins\":[";
  for (int i = 0; i < registeredCount; i++) {
    if (i > 0) json += ",";
    OrbytPin& pin = registeredPins[i];
    String modeStr = pin.mode() == OrbytPin::Mode::MODE_INPUT  ? "INPUT" :
                     pin.mode() == OrbytPin::Mode::MODE_PWM     ? "PWM" : "OUTPUT";
    json += "{\"pin\":" + String(pin.pin()) +
            ",\"mode\":\"" + modeStr + "\"" +
            ",\"role\":\"generic\"" +
            ",\"label\":\"" + pin.label() + "\"" +
            ",\"minValue\":" + String(pin.minValue()) +
            ",\"maxValue\":" + String(pin.maxValue()) + "}";
  }
  json += "]}";

  return json;
}
