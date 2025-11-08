// api_sensors_cal.ino
#include "globals.h"

// Simple placeholders: set zero offsets and span factors for telemetry sensors.
struct SensorCal { double v_zero=0, v_span=1, p_zero=0, p_span=1, t_zero=0, t_span=1; };
SensorCal gSensorCal;

static void api_sensors_get(){
  StaticJsonDocument<256> d;
  d["v_zero"]=gSensorCal.v_zero; d["v_span"]=gSensorCal.v_span;
  d["p_zero"]=gSensorCal.p_zero; d["p_span"]=gSensorCal.p_span;
  d["t_zero"]=gSensorCal.t_zero; d["t_span"]=gSensorCal.t_span;
  String s; serializeJson(d,s); server.send(200,"application/json",s);
}
static void api_sensors_post(){
  String body = readBody();
  StaticJsonDocument<256> in; if (deserializeJson(in, body)) { server.send(400,"application/json","{\"ok\":false}"); return; }
  if (in.containsKey("v_zero")) gSensorCal.v_zero = in["v_zero"];
  if (in.containsKey("v_span")) gSensorCal.v_span = in["v_span"];
  if (in.containsKey("p_zero")) gSensorCal.p_zero = in["p_zero"];
  if (in.containsKey("p_span")) gSensorCal.p_span = in["p_span"];
  if (in.containsKey("t_zero")) gSensorCal.t_zero = in["t_zero"];
  if (in.containsKey("t_span")) gSensorCal.t_span = in["t_span"];
  server.send(200,"application/json","{\"ok\":true}");
}

void wireApi_SensorsCal(){
  server.on("/api/sensors", HTTP_GET, api_sensors_get);
  server.on("/api/sensors", HTTP_POST, api_sensors_post);
}
