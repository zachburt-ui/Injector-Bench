// ads1256_dual.ino — Dual ADS1256 integration (Scales 1-8)
// Wiring (ESP32-WROOM):
//  SCK=18, MISO=19, MOSI=23
//  ADS_A: CS=5,  DRDY=36  -> channels 1..4
//  ADS_B: CS=4,  DRDY=39  -> channels 5..8
// Notes:
//  - Use differential inputs with gain=128 for 1 kg load cells.
//  - Provide per-channel tare/span in NVS.
//  - This file is a minimal stub to illustrate integration points; replace read stubs with your ADS1256 library calls.

#include "globals.h"
#include "pins.h"

// Simple calibration store
struct ScaleCal { double tare=0.0; double span=1.0; }; // raw -> mL
ScaleCal gScaleCal[8];

void scales_load_from_nvs(){
  prefs.begin("bench", true);
  for(int i=0;i<8;i++){
    String k1=String("sc_tare_")+i, k2=String("sc_span_")+i;
    gScaleCal[i].tare = prefs.getDouble(k1.c_str(), 0.0);
    gScaleCal[i].span = prefs.getDouble(k2.c_str(), 1.0);
  }
  prefs.end();
}
void scales_save_to_nvs(){
  prefs.begin("bench", false);
  for(int i=0;i<8;i++){
    String k1=String("sc_tare_")+i, k2=String("sc_span_")+i;
    prefs.putDouble(k1.c_str(), gScaleCal[i].tare);
    prefs.putDouble(k2.c_str(), gScaleCal[i].span);
  }
  prefs.end();
}

void ads1256_begin(){
  // init SPI and ADS1256 chips here (library-specific)
  pinMode(PIN_ADS_A_CS, OUTPUT); digitalWrite(PIN_ADS_A_CS, HIGH);
  pinMode(PIN_ADS_B_CS, OUTPUT); digitalWrite(PIN_ADS_B_CS, HIGH);
  pinMode(PIN_ADS_A_DRDY, INPUT);
  pinMode(PIN_ADS_B_DRDY, INPUT);
}

static double read_ads_channel_stub(int ch){
  // Replace with real ADS1256 read.
  // Stub: return a small rising value to visualize in UI.
  static double base[8]={0};
  base[ch] += 0.03*(1+(ch%3));
  return base[ch];
}

void scales_read_mL(double out_mL[8]){
  for(int i=0;i<8;i++){
    double raw = read_ads_channel_stub(i);
    // convert using tare/span -> mL
    out_mL[i] = (raw - gScaleCal[i].tare) * gScaleCal[i].span;
    if(out_mL[i] < 0) out_mL[i] = 0;
  }
}

// API: /api/scales (GET current, POST calibrations)
static void api_scales_get(){
  StaticJsonDocument<384> d;
  JsonArray a = d.createNestedArray("mL");
  for(int i=0;i<8;i++){ a.add(0.0); }
  // fill with current read
  double m[8]; scales_read_mL(m);
  for(int i=0;i<8;i++){ a[i] = m[i]; }
  String s; serializeJson(d,s); server.send(200,"application/json",s);
}
static void api_scales_post(){
  String body = readBody(); StaticJsonDocument<512> in;
  if(deserializeJson(in, body)){ server.send(400,"application/json","{\"ok\":false}"); return; }
  if(in.containsKey("tare") && in["tare"].is<JsonArray>()){
    for(int i=0;i<8;i++){ gScaleCal[i].tare = in["tare"][i] | gScaleCal[i].tare; }
  }
  if(in.containsKey("span") && in["span"].is<JsonArray>()){
    for(int i=0;i<8;i++){ gScaleCal[i].span = in["span"][i] | gScaleCal[i].span; }
  }
  scales_save_to_nvs();
  server.send(200,"application/json","{\"ok\":true}");
}

void wireApi_Scales(){
  ads1256_begin();
  scales_load_from_nvs();
  server.on("/api/scales", HTTP_GET, api_scales_get);
  server.on("/api/scales", HTTP_POST, api_scales_post);
}
