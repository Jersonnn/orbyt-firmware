/**
 * OrbytAuto — Zero-Configuration Orbyt Device
 *
 * Flash this ONCE to any ESP32 or ESP8266. No pin definitions needed.
 * Configure everything from the Orbyt app — pins, widgets, labels.
 *
 * Usage:
 *   1. Set your WiFi SSID and password below
 *   2. Upload to ESP32/ESP8266
 *   3. Open Orbyt app → Discover Devices → auto-find the board
 *   4. Or manually: add device with the board's IP
 *   5. In the app, pick any GPIO and assign a widget type
 *
 * That's it. No per-project firmware changes needed.
 */

#include <WiFi.h>
#include <OrbytAuto.h>

const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";

OrbytAuto board;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  board.begin("OrbytAuto", "orbyt-auto-1.0.0", 80);
  Serial.println("OrbytAuto ready — configure pins from the Orbyt app.");
}

void loop() {
  board.handle();
}
