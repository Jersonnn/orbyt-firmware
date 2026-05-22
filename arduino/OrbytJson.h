#pragma once

#include <Arduino.h>
#include "OrbytPin.h"

class OrbytJson {
public:
  String statusResponse(bool connected, const String& board,
                       const String& firmware, unsigned long uptimeMs);

  String writeResponse(bool success, const String& message = "");

  String readResponse(const OrbytPin::ReadResult& result);

  String errorResponse(const String& message);

  // Builds capabilities JSON from registered pins
  String capabilitiesResponse(OrbytPin* registeredPins, int registeredCount);
};
