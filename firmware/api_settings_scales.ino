
// api_settings_scales.ino
#include "globals.h"

static void api_telemetry() {
  StaticJsonDocument<256> doc;
  doc["ok"] = true;
  doc["voltage"] = telem.voltage;
  doc["pressure"] = telem.pressure;
  doc["tempC"] = telem.tempC;
  doc["rho"] = telem.rho;
  JsonArray arr = doc.createNestedArray("scales");
  for (int i=0;i<8;i++) arr.add(scales_vals[i]);
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

static void api_settings_get() {
  StaticJsonDocument<256> doc;
  doc["scales_on"] = settings.scales_on;
  doc["targetMl"]  = settings.targetMl;
  doc["psi_nom"]   = settings.psi_nom;
  doc["volt_nom"]  = settings.volt_nom;
  doc["fluid"]     = settings.fluid;
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}
static void api_settings_post() {
  String body = readBody();
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, body)) { server.send(400, "application/json", "{\"ok\":false,\"err\":\"bad json\"}"); return; }
  if (doc.containsKey("scales_on")) settings.scales_on = doc["scales_on"];
  if (doc.containsKey("targetMl"))  settings.targetMl  = doc["targetMl"];
  if (doc.containsKey("psi_nom"))   settings.psi_nom   = doc["psi_nom"];
  if (doc.containsKey("volt_nom"))  settings.volt_nom  = doc["volt_nom"];
  if (doc.containsKey("fluid")) { String s = doc["fluid"].as<String>(); s.toCharArray(settings.fluid, sizeof(settings.fluid)); }
  saveSettingsToNVS();
  server.send(200, "application/json", "{\"ok\":true}");
}

static void api_scales_get() {
  StaticJsonDocument<256> doc;
  JsonArray arr = doc.createNestedArray("values");
  for (int i=0;i<8;i++) arr.add(scales_vals[i]);
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}
static void api_scales_post() {
  String body = readBody();
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  if (doc["cmd"] == "tare") scales_tare();
  if (doc.containsKey("values")) {
    JsonArray arr = doc["values"].as<JsonArray>();
    int i=0; for (JsonVariant v : arr) if (i<8) scales_vals[i++] = v.as<float>();
  }
  server.send(200, "application/json", "{\"ok\":true}");
}

void wireApi_SettingsScales() {
  server.on("/api/telemetry", HTTP_GET, api_telemetry);
  server.on("/api/settings",  HTTP_GET, api_settings_get);
  server.on("/api/settings",  HTTP_POST, api_settings_post);
  server.on("/api/scales",    HTTP_GET, api_scales_get);
  server.on("/api/scales",    HTTP_POST, api_scales_post);
}


static void api_scales_tare() {
  // body: {"ch": -1}  (-1 = all)
  String body = readBody();
  StaticJsonDocument<128> in;
  if (deserializeJson(in, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  int ch = in["ch"] | -1;
  // TODO: hook into your real scale driver tare per channel
  if (ch < 0) { /*tare_all_scales()*/; }
  else { /*tare_scale(ch)*/; }
  server.send(200, "application/json", "{\"ok\":true}");
}

static void api_scales_span() {
  // body: {"ch":0, "mass_g":100.0}  -> set span against current reading
  String body = readBody();
  StaticJsonDocument<256> in;
  if (deserializeJson(in, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  int ch = in["ch"] | 0;
  double mass_g = in["mass_g"] | 0.0;
  // TODO: compute and store scale factor for channel ch based on current raw/counts vs mass_g
  // save to NVS if desired
  server.send(200, "application/json", "{\"ok\":true}");
}

static void api_scales_map() {
  // body: {"map":[0,1,2,3,4,5,6,7]} maps injector index -> scale index
  String body = readBody();
  StaticJsonDocument<256> in;
  if (deserializeJson(in, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  JsonArray m = in["map"].as<JsonArray>();
  // TODO: persist channel mapping for scales to injectors
  server.send(200, "application/json", "{\"ok\":true}");
}
