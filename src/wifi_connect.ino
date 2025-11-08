// wifi_connect.ino
#include "globals.h"
#include <WiFi.h>
#include <ESPmDNS.h>

static char WIFI_SSID[64] = "Unit#29_Starlink";
static char WIFI_PASS[64] = "03Silverado";
static char WIFI_HOST[32] = "injectorbench";

void wifi_load_from_nvs(){
  prefs.begin("bench", true);
  String ssid = prefs.getString("wifi_ssid", WIFI_SSID);
  String pass = prefs.getString("wifi_pass", WIFI_PASS);
  String host = prefs.getString("wifi_host", WIFI_HOST);
  prefs.end();
  strncpy(WIFI_SSID, ssid.c_str(), sizeof(WIFI_SSID)-1);
  strncpy(WIFI_PASS, pass.c_str(), sizeof(WIFI_PASS)-1);
  strncpy(WIFI_HOST, host.c_str(), sizeof(WIFI_HOST)-1);
}

void wifi_save_to_nvs(){
  prefs.begin("bench", false);
  prefs.putString("wifi_ssid", WIFI_SSID);
  prefs.putString("wifi_pass", WIFI_PASS);
  prefs.putString("wifi_host", WIFI_HOST);
  prefs.end();
}

bool wifi_try_connect(uint32_t timeout_ms=15000){
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(WIFI_HOST);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while(WiFi.status()!=WL_CONNECTED && (millis()-t0)<timeout_ms){
    delay(200);
  }
  if(WiFi.status()==WL_CONNECTED){
    if(!MDNS.begin(WIFI_HOST)){ /* ignore */ }
    return true;
  }
  return false;
}

void wifi_start_ap(){
  WiFi.mode(WIFI_AP);
  WiFi.softAP("InjectorBench-Setup");
}

void wifi_connect_init(){
  wifi_load_from_nvs();  // loads provided defaults first boot
  if(!wifi_try_connect()){
    wifi_start_ap();
  }else{
    wifi_save_to_nvs();  // persist confirmed credentials
  }
}

// --- Simple API to expose wifi status and accept changes if desired ---
static void api_wifi_get(){
  StaticJsonDocument<256> d;
  d["mode"] = (WiFi.getMode()==WIFI_AP)?"AP":((WiFi.getMode()==WIFI_STA)?"STA":"UNKNOWN");
  d["connected"] = (WiFi.status()==WL_CONNECTED);
  d["ssid"] = String(WIFI_SSID);
  d["host"] = String(WIFI_HOST);
  d["ip"] = (WiFi.status()==WL_CONNECTED)? WiFi.localIP().toString() : WiFi.softAPIP().toString();
  String s; serializeJson(d,s); server.send(200,"application/json",s);
}
static void api_wifi_post(){
  String body = readBody();
  StaticJsonDocument<256> in; 
  if(deserializeJson(in, body)){ server.send(400,"application/json","{\"ok\":false}"); return; }
  if(in.containsKey("ssid")){ String ss=in["ssid"]; strncpy(WIFI_SSID, ss.c_str(), sizeof(WIFI_SSID)-1); }
  if(in.containsKey("pass")){ String pw=in["pass"]; strncpy(WIFI_PASS, pw.c_str(), sizeof(WIFI_PASS)-1); }
  if(in.containsKey("host")){ String hn=in["host"]; strncpy(WIFI_HOST, hn.c_str(), sizeof(WIFI_HOST)-1); }
  wifi_save_to_nvs();
  // attempt reconnect
  bool ok = wifi_try_connect();
  if(!ok){ wifi_start_ap(); }
  server.send(200,"application/json", ok? "{\"ok\":true}":"{\"ok\":false}");
}

void wireApi_Wifi(){
  server.on("/api/wifi", HTTP_GET, api_wifi_get);
  server.on("/api/wifi", HTTP_POST, api_wifi_post);
}
