
// api_wifi_net.ino
#include "globals.h"

static void api_wifi_post() {
  String body = readBody();
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  sta_ssid = String((const char*)doc["sta_ssid"] | "");
  sta_pass = String((const char*)doc["sta_pass"] | "");
  prefs.begin("bench", false);
  prefs.putString("sta_ssid", sta_ssid);
  prefs.putString("sta_pass", sta_pass);
  prefs.end();
  bool ok = connectSTA(15000);
  StaticJsonDocument<256> out;
  out["ok"] = ok;
  out["sta_ip"] = ok ? WiFi.localIP().toString() : "";
  out["ap_ip"]  = WiFi.softAPIP().toString();
  String s; serializeJson(out, s);
  server.send(200, "application/json", s);
}

static void api_net() {
  StaticJsonDocument<256> doc;
  doc["sta_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["sta_ip"] = (WiFi.status() == WL_CONNECTED) ? WiFi.localIP().toString() : "";
  doc["ap_ip"] = WiFi.softAPIP().toString();
  doc["ssid"] = sta_ssid;
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void wireApi_WifiNet() {
  server.on("/api/wifi", HTTP_POST, api_wifi_post);
  server.on("/api/net",  HTTP_GET,  api_net);
}
