/*
  ESP32 Weather Station Web Interface

  Sensors:
    - BME280 (I2C) for temperature, humidity, pressure
    - BH1750FVI (I2C) for ambient light (lux)
    - Anemometer (reed switch) on GPIO with interrupt
    - LM393-based digital sensor (e.g., rain/sound/vibration) on GPIO

  Web UI:
    - Serves a responsive dashboard at '/'
    - Exposes JSON readings at '/api/sensors'

  Notes:
    - Fill in your WiFi SSID and PASSWORD below
    - Adjust pin assignments and anemometer calibration constants
    - Requires libraries: Adafruit_BME280, Adafruit_Sensor, BH1750
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <Preferences.h>
#include "web_pages.h"

// ========= WiFi config =========
const char* WIFI_SSID = "Domo-EntreMakers";      // TODO: set SSID
const char* WIFI_PASSWORD = "CLMDPdelaDOMO";  // TODO: set password

// ========= Pin assignments =========
// Default ESP32 I2C: SDA=21, SCL=22. Change if needed.
// Anemometer reed switch: use any GPIO with interrupt capability
const int ANEMOMETER_PIN = 27;         // TODO: set your anemometer pin
const bool ANEMO_PULLUP = true;        // enable internal pullup if needed

// Optional LM393 analog output pin (set to -1 to disable)
const int LM393_ANALOG_PIN = 33;       // e.g., 33 for ADC; -1 to disable
// Optional LM393 power control pin for RAIN lib (-1 to disable)
const int LM393_POWER_PIN = -1;        // e.g., 25 to switch VCC via transistor; -1 to disable

// ========= Anemometer calibration =========
// Provided spec: 1 rotation = 2 pulses, and wind speed (m/s) = Hz * 0.34
// Here Hz is the pulse frequency (pulses per second).
// To fit generic formula: m/s = (pps / PPR) * MPR
// choose: PPR = 2, MPR = PPR * 0.34 = 0.68 so that m/s = pps * 0.34
const float ANEMO_PULSES_PER_ROTATION = 2.0f;
const float ANEMO_MPS_PER_ROTATION = 0.68f; // 2 pulses/rotation, 0.34 m/s per pulse Hz

// Calculation window for pulse frequency (ms)
const uint32_t ANEMO_WINDOW_MS = 2000;
const uint32_t WIFI_RECONNECT_INTERVAL_MS = 5000;
uint32_t lastWifiReconnectAttempt = 0;

// ========= Globals =========
WebServer server(80);

Adafruit_BME280 bme;     // I2C
bool hasBME = false;

BH1750 lightMeter;       // I2C
bool hasBH1750 = false;

volatile uint32_t anemoPulseCount = 0; // ISR incremented
volatile uint32_t anemoLastMicros = 0; // for simple debounce
uint32_t lastWindowStart = 0;
uint32_t lastWindowPulses = 0;
float lastPulsesPerSec = 0.0f;

// Rain calibration (ADC raw). Lower -> wet, Higher -> dry
uint16_t RAIN_DRY_RAW = 3500;
uint16_t RAIN_WET_RAW = 800;

// Light calibration (lux). Lower -> dark, Higher -> bright
float LUX_MIN = 0.0f;
float LUX_MAX = 50000.0f; // default span; calibrate in UI

// ========= UI thresholds (percent) =========
// Color mapping for rain percent (0 = wet, 100 = dry)
// red: < orange_min, orange: [orange_min, green_min), green: >= green_min
uint8_t UI_RAIN_GREEN_MIN = 70;
uint8_t UI_RAIN_ORANGE_MIN = 30;

// Light percent color thresholds
uint8_t UI_LIGHT_GREEN_MIN = 70;
uint8_t UI_LIGHT_ORANGE_MIN = 30;

Preferences prefs;

// ========= ISR =========
void IRAM_ATTR anemoISR() {
  uint32_t now = micros();
  // basic debounce (~3 ms) to mitigate reed bounce
  if (now - anemoLastMicros > 3000) {
    anemoPulseCount++;
    anemoLastMicros = now;
  }
}

// ========= Utility: safe JSON number =========
String jsonNumber(float v) {
  if (isnan(v)) return "null";
  String s; s.reserve(16);
  s += String(v, 2);
  return s;
}

// ========= Sensor reading helpers =========
void updateAnemoWindow() {
  uint32_t now = millis();
  uint32_t elapsed = now - lastWindowStart;
  if (elapsed >= ANEMO_WINDOW_MS) {
    uint32_t pulses = anemoPulseCount;
    uint32_t pulsesInWindow = pulses - lastWindowPulses;
    float seconds = (float)elapsed / 1000.0f;
    lastPulsesPerSec = (seconds > 0.0f) ? (pulsesInWindow / seconds) : 0.0f;
    lastWindowStart = now;
    lastWindowPulses = pulses;
  }
}

float computeWindSpeedMps(float pulsesPerSec) {
  if (ANEMO_PULSES_PER_ROTATION <= 0.0f) return NAN;
  float rps = pulsesPerSec / ANEMO_PULSES_PER_ROTATION;
  return rps * ANEMO_MPS_PER_ROTATION;
}

// ========= Web assets =========
// See web_pages.h for HTML templates.

// ========= HTTP Handlers =========
void handleRoot();
void handleConfigPage();
void handleAdvancedPage();

// ========= HTTP Page Handlers =========
void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html; charset=utf-8", MINI_DASH_ASCII);
}

// ASCII-only configuration page to avoid encoding issues
void handleConfigPage() {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html; charset=utf-8", CONFIG_ASCII_HTML);
}

// Advanced page shows full metrics grid
// ASCII-only advanced page
void handleAdvancedPage() {
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html; charset=utf-8", ADV_ASCII_HTML);
}

void handleApiSensors() {
  server.sendHeader("Cache-Control", "no-store");
  updateAnemoWindow();

  float t = NAN, h = NAN, p = NAN;
  if (hasBME) {
    t = bme.readTemperature();      // degC
    h = bme.readHumidity();         // %
    p = bme.readPressure() / 100.0; // hPa
  }

  float lux = NAN;
  if (hasBH1750) {
    lux = lightMeter.readLightLevel();
  }
  float light_pct = NAN;
  if (!isnan(lux) && LUX_MAX != LUX_MIN) {
    float minv = LUX_MIN, maxv = LUX_MAX;
    if (maxv < minv) { float t = maxv; maxv = minv; minv = t; }
    float pct = 100.0f * (lux - minv) / (maxv - minv);
    if (pct < 0) pct = 0; if (pct > 100) pct = 100;
    light_pct = pct;
  }

  int lm393_a = -1;
  float lm393_v = NAN;
  float lm393_pct = NAN;
  if (LM393_ANALOG_PIN >= 0) {
    const int N = 8; long sum = 0;
    for (int i=0;i<N;i++){ sum += analogRead(LM393_ANALOG_PIN); delayMicroseconds(200);} 
    lm393_a = (int)(sum / N);
    lm393_v = (3.3f * lm393_a) / 4095.0f;
    int minRaw = RAIN_WET_RAW, maxRaw = RAIN_DRY_RAW;
    if (maxRaw != minRaw) {
      if (maxRaw < minRaw) { int t = maxRaw; maxRaw = minRaw; minRaw = t; }
      float pct = 100.0f * (float)(lm393_a - minRaw) / (float)(maxRaw - minRaw);
      if (pct < 0) pct = 0; if (pct > 100) pct = 100;
      lm393_pct = pct;
    }
  }

  float pps = lastPulsesPerSec;
  float mps = computeWindSpeedMps(pps);
  float rpm = (ANEMO_PULSES_PER_ROTATION > 0.0f) ? (pps / ANEMO_PULSES_PER_ROTATION) * 60.0f : NAN;
  float kmh = isnan(mps) ? NAN : (mps * 3.6f);

  String json;
  json.reserve(512);
  json += F("{");
  json += F("\"temperature_c\":"); json += jsonNumber(t); json += ',';
  json += F("\"humidity_pct\":"); json += jsonNumber(h); json += ',';
  json += F("\"pressure_hpa\":"); json += jsonNumber(p); json += ',';
  json += F("\"lux\":"); json += jsonNumber(lux); json += ',';
  json += F("\"light_percent\":"); json += jsonNumber(light_pct); json += ',';
  json += F("\"lm393_analog\":"); if (lm393_a < 0) json += F("null"); else json += String(lm393_a); json += ',';
  json += F("\"lm393_voltage_v\":"); json += jsonNumber(lm393_v); json += ',';
  json += F("\"lm393_percent\":"); json += jsonNumber(lm393_pct);
  json += F(",\"wind\":{");
  json += F("\"pulses\":"); json += String(anemoPulseCount); json += ',';
  json += F("\"pulses_per_sec\":"); json += jsonNumber(pps); json += ',';
  json += F("\"rpm\":"); json += jsonNumber(rpm); json += ',';
  json += F("\"speed_mps\":"); json += jsonNumber(mps); json += ',';
  json += F("\"speed_kmh\":"); json += jsonNumber(kmh);
  json += F("}}");
  server.send(200, "application/json; charset=utf-8", json);
}

void handleApiConfigGet() {
  server.sendHeader("Cache-Control", "no-store");
  char buf[256];
  int n = snprintf(
    buf, sizeof(buf),
    "{\"rain\":{\"green_min\":%u,\"orange_min\":%u,\"cal\":{\"dry_raw\":%u,\"wet_raw\":%u}},"
    "\"light\":{\"green_min\":%u,\"orange_min\":%u,\"cal\":{\"lux_min\":%.2f,\"lux_max\":%.2f}}}",
    (unsigned)UI_RAIN_GREEN_MIN, (unsigned)UI_RAIN_ORANGE_MIN,
    (unsigned)RAIN_DRY_RAW, (unsigned)RAIN_WET_RAW,
    (unsigned)UI_LIGHT_GREEN_MIN, (unsigned)UI_LIGHT_ORANGE_MIN,
    (double)LUX_MIN, (double)LUX_MAX
  );
  if (n < 0 || n >= (int)sizeof(buf)) {
    server.send(500, "text/plain", "JSON build error");
    return;
  }
  server.send(200, "application/json; charset=utf-8", buf);
}

void handleApiConfigPost() {
  server.sendHeader("Cache-Control", "no-store");
  Serial.println(F("=== CONFIG POST START ==="));

  // Print all received arguments for debugging
  for (int i = 0; i < server.args(); i++) {
    Serial.printf("Arg[%d]: %s = %s\n", i, server.argName(i).c_str(), server.arg(i).c_str());
  }

  // Rain thresholds
  int g = UI_RAIN_GREEN_MIN;
  int o = UI_RAIN_ORANGE_MIN;
  if (server.hasArg("rain_green_min")) {
    g = server.arg("rain_green_min").toInt();
    Serial.printf("Rain green_min received: %d\n", g);
  }
  if (server.hasArg("rain_orange_min")) {
    o = server.arg("rain_orange_min").toInt();
    Serial.printf("Rain orange_min received: %d\n", o);
  }

  // Validate rain thresholds
  if (g < 0) g = 0; if (g > 100) g = 100;
  if (o < 0) o = 0; if (o > 100) o = 100;
  // Allow orange to be higher than green - user choice
  UI_RAIN_GREEN_MIN = (uint8_t)g;
  UI_RAIN_ORANGE_MIN = (uint8_t)o;
  Serial.printf("Rain thresholds set: Green=%d%%, Orange=%d%%\n", g, o);

  // Optional calibration raw values
  if (server.hasArg("rain_dry_raw")) {
    int dr = server.arg("rain_dry_raw").toInt(); if (dr < 0) dr = 0; if (dr > 4095) dr = 4095; RAIN_DRY_RAW = (uint16_t)dr;
  }
  if (server.hasArg("rain_wet_raw")) {
    int wr = server.arg("rain_wet_raw").toInt(); if (wr < 0) wr = 0; if (wr > 4095) wr = 4095; RAIN_WET_RAW = (uint16_t)wr;
  }
  // Capture current reading if requested
  if (server.hasArg("rain_capture")) {
    String mode = server.arg("rain_capture");
    if (LM393_ANALOG_PIN >= 0) {
      const int N = 16; long sum = 0; for (int i=0;i<N;i++){ sum += analogRead(LM393_ANALOG_PIN); delayMicroseconds(200);} int avg = (int)(sum / N);
      if (mode == "dry") RAIN_DRY_RAW = (uint16_t)avg; else if (mode == "wet") RAIN_WET_RAW = (uint16_t)avg;
    }
  }
  // Light calibration
  if (server.hasArg("lux_min")) { LUX_MIN = server.arg("lux_min").toFloat(); if (LUX_MIN < 0) LUX_MIN = 0; }
  if (server.hasArg("lux_max")) { LUX_MAX = server.arg("lux_max").toFloat(); if (LUX_MAX < 0.001f) LUX_MAX = 0.001f; }
  if (server.hasArg("lux_capture")) {
    String m = server.arg("lux_capture");
    if (hasBH1750) {
      float lv = lightMeter.readLightLevel();
      if (!isnan(lv)) { if (m == "min") LUX_MIN = lv; else if (m == "max") LUX_MAX = lv; }
    }
  }
  // Light thresholds
  int lg = UI_LIGHT_GREEN_MIN;
  int lo = UI_LIGHT_ORANGE_MIN;
  if (server.hasArg("light_green_min")) {
    lg = server.arg("light_green_min").toInt();
    Serial.printf("Light green_min received: %d\n", lg);
  }
  if (server.hasArg("light_orange_min")) {
    lo = server.arg("light_orange_min").toInt();
    Serial.printf("Light orange_min received: %d\n", lo);
  }

  // Validate light thresholds
  if (lg < 0) lg = 0; if (lg > 100) lg = 100;
  if (lo < 0) lo = 0; if (lo > 100) lo = 100;
  // Allow orange to be higher than green - user choice
  UI_LIGHT_GREEN_MIN = (uint8_t)lg;
  UI_LIGHT_ORANGE_MIN = (uint8_t)lo;
  Serial.printf("Light thresholds set: Green=%d%%, Orange=%d%%\n", lg, lo);
  // Persist to flash with error checking
  Serial.println(F("Saving to flash..."));
  if (prefs.begin("ui", false)) {
    size_t written = 0;
    written += prefs.putUChar("rgm", UI_RAIN_GREEN_MIN);
    written += prefs.putUChar("rom", UI_RAIN_ORANGE_MIN);
    written += prefs.putUChar("lgm", UI_LIGHT_GREEN_MIN);
    written += prefs.putUChar("lom", UI_LIGHT_ORANGE_MIN);
    written += prefs.putUShort("r_dry", RAIN_DRY_RAW);
    written += prefs.putUShort("r_wet", RAIN_WET_RAW);
    written += prefs.putFloat("lx_min", LUX_MIN);
    written += prefs.putFloat("lx_max", LUX_MAX);
    prefs.end();
    Serial.printf("Flash write completed, %d bytes written\n", written);
  } else {
    Serial.println(F("ERROR: Failed to open preferences for write!"));
  }

  Serial.println(F("=== CONFIG POST END ==="));
  handleApiConfigGet();
  return;
}

// ========= Setup =========
void setup() {
  // Serial
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("Booting Weather Station..."));

  // Pins
  if (ANEMO_PULLUP) pinMode(ANEMOMETER_PIN, INPUT_PULLUP); else pinMode(ANEMOMETER_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ANEMOMETER_PIN), anemoISR, FALLING);

  if (LM393_POWER_PIN >= 0) {
    pinMode(LM393_POWER_PIN, OUTPUT);
    digitalWrite(LM393_POWER_PIN, HIGH);
  }

  // I2C
  Wire.begin(); // default SDA=21, SCL=22 on ESP32

  // BME280 init (try common addresses 0x76 then 0x77)
  hasBME = bme.begin(0x76) || bme.begin(0x77);
  Serial.print(F("BME280: ")); Serial.println(hasBME ? F("OK") : F("NOT FOUND"));

  // BH1750 init (try 0x23 then 0x5C)
  hasBH1750 = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
  if (!hasBH1750) {
    hasBH1750 = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C, &Wire);
  }
  Serial.print(F("BH1750: ")); Serial.println(hasBH1750 ? F("OK") : F("NOT FOUND"));

  // No init needed for analog rain sensor

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWifiReconnectAttempt = millis();
  Serial.print(F("WiFi: connecting"));
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("WiFi connected: ")); Serial.println(WiFi.localIP());
  } else {
    Serial.println(F("WiFi connect failed; hosting AP 'ESP32-Station'"));
    WiFi.softAP("ESP32-Station");
    lastWifiReconnectAttempt = millis();
    Serial.print(F("AP IP: ")); Serial.println(WiFi.softAPIP());
  }

  // HTTP routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/advanced", HTTP_GET, handleAdvancedPage);
  server.on("/config", HTTP_GET, handleConfigPage);
  server.on("/api/sensors", HTTP_GET, handleApiSensors);
  server.on("/api/config", HTTP_GET, handleApiConfigGet);
  server.on("/api/config", HTTP_POST, handleApiConfigPost);
  server.onNotFound([]() {
    if (server.uri().startsWith("/api/")) {
      server.send(404, "application/json; charset=utf-8", "{\"error\":\"not found\"}");
    } else {
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", "");
    }
  });
  server.begin();
  Serial.println(F("HTTP server started"));

  lastWindowStart = millis();
  lastWindowPulses = 0;

  // Load persisted UI thresholds and calibration data
  prefs.begin("ui", true);
  UI_RAIN_GREEN_MIN = prefs.getUChar("rgm", UI_RAIN_GREEN_MIN);
  UI_RAIN_ORANGE_MIN = prefs.getUChar("rom", UI_RAIN_ORANGE_MIN);
  UI_LIGHT_GREEN_MIN = prefs.getUChar("lgm", UI_LIGHT_GREEN_MIN);
  UI_LIGHT_ORANGE_MIN = prefs.getUChar("lom", UI_LIGHT_ORANGE_MIN);
  RAIN_DRY_RAW = prefs.getUShort("r_dry", RAIN_DRY_RAW);
  RAIN_WET_RAW = prefs.getUShort("r_wet", RAIN_WET_RAW);
  LUX_MIN = prefs.getFloat("lx_min", LUX_MIN);
  LUX_MAX = prefs.getFloat("lx_max", LUX_MAX);
  prefs.end();

  Serial.println(F("Configuration loaded from flash:"));
  Serial.printf("Rain thresholds: Green=%d%%, Orange=%d%%\n", UI_RAIN_GREEN_MIN, UI_RAIN_ORANGE_MIN);
  Serial.printf("Rain calibration: Dry=%d, Wet=%d\n", RAIN_DRY_RAW, RAIN_WET_RAW);
  Serial.printf("Light thresholds: Green=%d%%, Orange=%d%%\n", UI_LIGHT_GREEN_MIN, UI_LIGHT_ORANGE_MIN);
  Serial.printf("Light calibration: Min=%.1flx, Max=%.1flx\n", LUX_MIN, LUX_MAX);
}

// ========= Loop =========
void loop() {
  server.handleClient();
  updateAnemoWindow();

  wifi_mode_t mode = WiFi.getMode();
  bool staActive = (mode == WIFI_MODE_STA) || (mode == WIFI_MODE_APSTA);
  if (staActive) {
    if (WiFi.status() == WL_CONNECTED) {
      lastWifiReconnectAttempt = millis();
    } else {
      uint32_t now = millis();
      if (now - lastWifiReconnectAttempt >= WIFI_RECONNECT_INTERVAL_MS) {
        lastWifiReconnectAttempt = now;
        WiFi.reconnect();
      }
    }
  }
}

