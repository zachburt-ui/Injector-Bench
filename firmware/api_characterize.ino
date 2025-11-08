
// api_characterize.ino
#include "globals.h"

static void api_char_start() {
  String body = readBody();
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, body)) { server.send(400, "application/json", "{\"ok\":false,\"err\":\"bad json\"}"); return; }
  RunConfig cfg;
  String mode = doc["mode"] | "dynamic";
  cfg.mode = (mode == "static") ? RM_STATIC : (mode == "clean") ? RM_CLEAN : RM_DYNAMIC;
  cfg.hz = doc["params"]["hz"] | 20.0;
  cfg.pw_ms = doc["params"]["pw_ms"] | 6.0;
  cfg.seconds = doc["params"]["seconds"] | 10;
  cfg.inj_mask = doc["params"]["mask"] | 0xFF;
  characterize_begin(cfg);
  server.send(200, "application/json", "{\"ok\":true}");
}

static void api_char_stop()  { characterize_stop_graceful(); server.send(200, "application/json", "{\"ok\":true}"); }
static void api_char_skip()  { characterize_stop_graceful(); server.send(200, "application/json", "{\"ok\":true}"); }

void wireApi_Characterize() {
  server.on("/api/characterize/start", HTTP_POST, api_char_start);
  server.on("/api/characterize/stop",  HTTP_POST, api_char_stop);
  server.on("/api/characterize/skip",  HTTP_POST, api_char_skip);
}
