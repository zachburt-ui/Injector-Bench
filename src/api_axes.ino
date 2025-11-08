// api_axes.ino
#include "globals.h"

void api_axes_get() {
  StaticJsonDocument<512> doc;
  JsonArray v = doc.createNestedArray("volts"); for (int i=0;i<gAxes.n_volts;i++) v.add(gAxes.volts[i]);
  JsonArray k = doc.createNestedArray("dkp");   for (int i=0;i<gAxes.n_dkp;i++)   k.add(gAxes.dkp[i]);
  JsonArray sp= doc.createNestedArray("spa");   for (int i=0;i<gAxes.n_spa;i++)   sp.add(gAxes.spa_bins[i]);
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void api_axes_post() {
  String body = readBody();
  StaticJsonDocument<512> in;
  if (deserializeJson(in, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  if (in.containsKey("volts")) {
    JsonArray v = in["volts"].as<JsonArray>();
    gAxes.n_volts = min(6, (int)v.size());
    for (int i=0;i<gAxes.n_volts;i++) gAxes.volts[i] = v[i];
  }
  if (in.containsKey("dkp")) {
    JsonArray k = in["dkp"].as<JsonArray>();
    gAxes.n_dkp = min(5, (int)k.size());
    for (int i=0;i<gAxes.n_dkp;i++) gAxes.dkp[i] = k[i];
  }
  if (in.containsKey("spa")) {
    JsonArray sp = in["spa"].as<JsonArray>();
    gAxes.n_spa = min(6, (int)sp.size());
    for (int i=0;i<gAxes.n_spa;i++) gAxes.spa_bins[i] = sp[i];
  }
  axes_save_to_nvs();
  server.send(200, "application/json", "{\"ok\":true}");
}

void wireApi_Axes() {
  server.on("/api/axes", HTTP_GET, api_axes_get);
  server.on("/api/axes", HTTP_POST, api_axes_post);
}
