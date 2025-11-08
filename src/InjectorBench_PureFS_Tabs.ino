
// InjectorBench_PureFS_Tabs.ino
// Entry point only: setup/loop. UI is in LittleFS index.html (no .gz).

#include "globals.h"

Preferences prefs;
WebServer server(80);

String sta_ssid, sta_pass;
IPAddress staIP, apIP;

Telemetry telem = {12.0f, 58.0f, 20.0f, 0.744f};
Settings  settings = {false, 100.0f, 58.0f, 13.8f, "TestFluid"};
float     scales_vals[8] = {0};

void setup() {
  Serial.begin(115200);
  delay(100);

  loadSettingsFromNVS();
  axes_load_from_nvs();
  fluid_load_from_nvs();

  // WiFi: AP first so there's always a way in, then try STA
  startAP();
  if (!sta_ssid.isEmpty()) {
    bool ok = connectSTA(15000);
    Serial.println(ok ? String("STA OK: ") + WiFi.localIP().toString() : "STA failed; AP only");
  } else {
    Serial.println("No STA creds; AP only");
  }

  drivers_init();

  // Filesystem
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
  }

  // Web
  wireStaticRoutes();
  wireApiRoutes();
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  characterize_tick();
}
