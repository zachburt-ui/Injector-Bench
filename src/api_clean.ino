// api_clean.ino
#include "globals.h"

struct CleanState {
  bool pump_on=false;
  bool backflush_on=false;
  int  mask=255;
  String solvent="Cleaner";
  unsigned long until_ms=0;
} gClean;

static void api_clean_get(){
  StaticJsonDocument<256> d;
  d["pump_on"]=gClean.pump_on; d["backflush_on"]=gClean.backflush_on;
  d["mask"]=gClean.mask; d["solvent"]=gClean.solvent;
  d["remaining_ms"] = (long)((long)gClean.until_ms - (long)millis());
  if ((long)d["remaining_ms"] < 0) d["remaining_ms"]=0;
  String s; serializeJson(d,s); server.send(200,"application/json",s);
}

static void api_clean_post(){
  String body = readBody();
  StaticJsonDocument<256> in;
  if (deserializeJson(in, body)) { server.send(400,"application/json","{\"ok\":false}"); return; }
  if (in.containsKey("pump_on")) gClean.pump_on = in["pump_on"];
  if (in.containsKey("backflush_on")) gClean.backflush_on = in["backflush_on"];
  if (in.containsKey("mask")) gClean.mask = in["mask"];
  if (in.containsKey("solvent")) gClean.solvent = (const char*)in["solvent"];
  int seconds = in["seconds"] | 0;
  if (seconds>0) gClean.until_ms = millis() + (unsigned long)seconds*1000UL;
  server.send(200,"application/json","{\"ok\":true}");
}

void wireApi_Clean(){
  server.on("/api/clean", HTTP_GET, api_clean_get);
  server.on("/api/clean", HTTP_POST, api_clean_post);
}
