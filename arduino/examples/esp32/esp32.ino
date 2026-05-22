/**
 * Orbyt ESP32 Example
 *  
 * Setup:
 * 1. Copy src/orbyt-arduino/ to your Arduino libraries/ folder
 * 2. Set your WiFi SSID and password below
 * 3. Upload to your ESP32
 * 4. Open the Orbyt app and add a device at the board's IP
 *
 * For zero-config (recommended): uses OrbytAuto below.
 * For custom pin definitions: uncomment OrbytDevice and register pins.
 */

#include <WiFi.h>

// ─── Zero-config: flash once, configure everything from the app ───
#include <OrbytAuto.h>
OrbytAuto board;

// ─── Or use explicit pin registration instead ───
// #include <OrbytDevice.h>
// OrbytDevice orbyt;

const char* WIFI_SSID = "YourWiFiSSID";
const char* WIFI_PASS = "YourWiFiPassword";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP: ");
  Serial.println(WiFi.localIP());

  // ─── OrbytAuto: zero pin registration ───
  board.begin("My ESP32", "orbyt-auto-1.0.0", 80);

  /* ─── OrbytDevice: explicit pins + callbacks ───
  orbyt.begin("My ESP32", "orbyt-fw-1.0.0", 80);
  orbyt.registerOutput(2, "LED");
  orbyt.registerPwm(5, "Dimmer", 0, 255);
  orbyt.registerInput(34, "Light Sensor");
  orbyt.setOnWriteCallback([](uint8_t pin, int value) {
    Serial.printf("Pin %d written: %d\n", pin, value);
  });
  */

  Serial.println("Orbyt ready! Configure pins from the Orbyt app.");
}

void loop() {
  board.handle();
  // orbyt.handle();
}
