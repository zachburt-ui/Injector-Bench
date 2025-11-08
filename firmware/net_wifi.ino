
// net_wifi.ino
#include "globals.h"

static const char* AP_SSID  = "InjectorBench";
static const char* AP_PASS  = "injector123"; // change as desired

void startAP() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(50);
  apIP = WiFi.softAPIP();
}

bool connectSTA(unsigned long timeoutMs) {
  if (sta_ssid.isEmpty()) return false;
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(100);
  }
  if (WiFi.status() == WL_CONNECTED) {
    staIP = WiFi.localIP();
    MDNS.end();
    if (MDNS.begin("injectorbench")) {
      MDNS.addService("http", "tcp", 80);
    }
    return true;
  }
  return false;
}
