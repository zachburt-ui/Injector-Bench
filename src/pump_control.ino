// pump_control.ino — active-HIGH pump control + API
#include "globals.h"
#include "pins.h"

void pump_begin(){ pinMode(PIN_PUMP, OUTPUT); digitalWrite(PIN_PUMP, LOW); }
inline void pump_on(){ digitalWrite(PIN_PUMP, HIGH); }
inline void pump_off(){ digitalWrite(PIN_PUMP, LOW); }
bool pump_is_on(){ return digitalRead(PIN_PUMP)==HIGH; }

static void api_pump_get(){
  StaticJsonDocument<96> d; d["state"]= pump_is_on()?1:0;
  String s; serializeJson(d,s); server.send(200,"application/json",s);
}
static void api_pump_post(){
  String body = readBody(); StaticJsonDocument<96> in;
  if(deserializeJson(in, body)){ server.send(400,"application/json","{\"ok\":false}"); return; }
  if(in.containsKey("state")){ int st=in["state"]; if(st) pump_on(); else pump_off(); }
  server.send(200,"application/json","{\"ok\":true}");
}
void wireApi_Pump(){
  pump_begin();
  server.on("/api/pump", HTTP_GET, api_pump_get);
  server.on("/api/pump", HTTP_POST, api_pump_post);
}
