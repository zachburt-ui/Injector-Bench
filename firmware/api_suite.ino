
// api_suite.ino
#include "globals.h"
extern SpaData gSpa;

// Sweep accumulators (reset when arming a P59V row)
static double sweep_pw_ms[10];
static double sweep_ml_per_pulse[10];
static int sweep_n = 0;
static double sweep_hz = 20.0;
static int sweep_pulses = 0;

static void sweep_reset() { sweep_n = 0; }
static void sweep_add_point(double pw_ms, const float flow_mL[8], double seconds, int pulses, int volt_row) {
  // We'll convert to ml per pulse averaged across injectors that are enabled
  double sum=0; int cnt=0;
  for (int i=0;i<8;i++){ sum += flow_mL[i]; cnt++; }
  double ml_per_pulse = (pulses>0) ? (sum/cnt)/pulses : 0.0;
  if (sweep_n < 10) { sweep_pw_ms[sweep_n] = pw_ms; sweep_ml_per_pulse[sweep_n] = ml_per_pulse; sweep_n++; }
}


extern volatile SuiteStatus gSuiteStatus;

static void api_suite_status() {
  StaticJsonDocument<512> doc;
  doc["phase"] = (int)gSuiteStatus.phase;
  doc["index"] = gSuiteStatus.index;
  doc["base_filled"] = gSuiteStatus.base_filled;
  JsonArray hf = doc.createNestedArray("holley_filled"); for (int r=0;r<6;r++) hf.add(gSuiteStatus.holley_filled[r]);
  JsonArray pf = doc.createNestedArray("p59_filled"); for (int r=0;r<5;r++) pf.add(gSuiteStatus.p59_filled[r]);
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

static void api_suite_arm() {
  String body = readBody();
  StaticJsonDocument<256> in;
  if (deserializeJson(in, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  int ph = in["phase"] | 0;
  int idx = in["index"] | -1;
  suite_arm((SuitePhase)ph, idx);
  server.send(200, "application/json", "{\"ok\":true}");
}

static void api_suite_commit() {
  suite_commit_from_current();
  server.send(200, "application/json", "{\"ok\":true}");
}


// Add one PW sweep point (called after a run completes)
static void api_suite_add_p59v_point() {
  String body = readBody();
  StaticJsonDocument<256> in;
  if (deserializeJson(in, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  double pw = in["pw_ms"] | 0.0;
  int hz = in["hz"] | 20;
  int seconds = in["seconds"] | 5;
  int volt_row = in["volt_row"] | 0;
  // pull current buffers
  float flow_mL[8]; for (int i=0;i<8;i++) flow_mL[i] = scales_vals[i];
  int pulses = (hz/8.0) * seconds; // sequential across 8
  sweep_add_point(pw, flow_mL, seconds, pulses, volt_row);
  server.send(200, "application/json", "{\"ok\":true}");
}

static void api_suite_fit_p59v() {
  // Fit D for the current armed voltage row using accumulated sweep points
  double D_ms=0, k=0;
  regress_offset_ms(sweep_pw_ms, sweep_ml_per_pulse, sweep_n, &D_ms, &k);
  int r = gSuiteStatus.index;
  if (gSuiteStatus.phase == SP_P59V && r>=0 && r<6) {
    for (int i=0;i<8;i++) gSuite.p59_offset_ms_v[r][i] = D_ms; // same D across inj for now (we can split later with per-injector points)
    gSuiteStatus.holley_filled[r] = true; // reuse filled flags array for volts row state
  }
  server.send(200, "application/json", "{\"ok\":true,\"offset_ms\":"+String(D_ms,3)+",\"k\":"+String(k,6)+"}");
}

void wireApi_Suite() {
  server.on("/api/suite", HTTP_GET, api_suite_status);
  server.on("/api/suite/arm", HTTP_POST, api_suite_arm);
  server.on("/api/suite/commit", HTTP_POST, api_suite_commit);
  server.on("/api/suite/add_p59v_point", HTTP_POST, api_suite_add_p59v_point);
  server.on("/api/suite/fit_p59v", HTTP_POST, api_suite_fit_p59v);
  server.on("/api/suite/spa_point", HTTP_POST, api_suite_spa_point);
  server.on("/api/suite/spa_fit", HTTP_POST, api_suite_spa_fit);
}


// SPA sweep storage (bins fixed in gSpa.bins_ms). We'll store measured ml/pulse at each bin.
static double spa_ml_per_pulse[6] = {0,0,0,0,0,0};
static bool   spa_has[6] = {false,false,false,false,false,false};

static void spa_reset() { for (int i=0;i<6;i++){ spa_ml_per_pulse[i]=0; spa_has[i]=false; } }

// POST /api/suite/spa_point  { "bin_index": N, "hz":20, "seconds":5, "pw_ms":<bin> }
static void api_suite_spa_point() {
  String body = readBody();
  StaticJsonDocument<256> in;
  if (deserializeJson(in, body)) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  int idx = in["bin_index"] | -1;
  int hz = in["hz"] | 20;
  int seconds = in["seconds"] | 5;
  if (idx < 0 || idx >= 6) { server.send(400, "application/json", "{\"ok\":false}"); return; }
  // average mL per pulse across injectors
  float flow_mL[8]; for (int i=0;i<8;i++) flow_mL[i] = scales_vals[i];
  int pulses = (hz/8.0)*seconds;
  double sum=0; for (int i=0;i<8;i++) sum += flow_mL[i];
  double ml_per_pulse = (pulses>0)? (sum/8.0)/pulses : 0.0;
  spa_ml_per_pulse[idx] = ml_per_pulse;
  spa_has[idx] = true;
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/suite/spa_fit  { }  -> computes SPA adders with midrange k,D then SPA = (D_eff - D)
static void api_suite_spa_fit() {
  // Need midrange reference: do regression from prior P59V sweep at nominal volts row (e.g., 13.5V)
  // We'll search for any stored row in gSuite.p59_offset_ms_v matching nominal volts (13.5V index=3)
  const int NOM_VOLT_INDEX = 3;
  double D_ms = gSuite.p59_offset_ms_v[NOM_VOLT_INDEX][0]; // take ch1 estimate as D
  // Estimate slope k from previous base run: cc/min for ch1 -> convert to ml per pulse at midrange (6 ms, 20 Hz, 10 s)
  // fallback: compute from base_cc_min average
  double avg_ccmin=0; for (int i=0;i<8;i++) avg_ccmin += gSuite.base_cc_min[i]; avg_ccmin/=8.0;
  // pulses per injector in 10s at 20Hz sequential: 20/8 * 10 = 25 pulses
  double pulses_mid = 25.0;
  double ml_per_s = (avg_ccmin / 60.0);
  double ml_per_pulse_mid = (ml_per_s * 10.0) / pulses_mid; // 10s window
  // k ~ slope in ml/ms: ml_per_pulse = k*(PW - D) => k = ml_per_pulse_mid / (6 - D)
  double k = (6.0 - D_ms) != 0 ? ml_per_pulse_mid / (6.0 - D_ms) : 0.0;

  // Now compute SPA adders for each bin
  for (int i=0;i<6;i++) {
    double pw = gSpa.bins_ms[i];
    if (!spa_has[i]) { gSpa.adder_ms[i] = 0; continue; }
    // Back-solve effective D_eff from measured ml/pulse at this PW: ml = k*(PW - D_eff) => D_eff = PW - ml/k
    double mlp = spa_ml_per_pulse[i];
    double D_eff = (k>0) ? (pw - (mlp / k)) : 0.0;
    double SPA = D_eff - D_ms;
    if (SPA < 0) SPA = 0;
    gSpa.adder_ms[i] = SPA;
  }

  // Estimate Min PW as PW where k*(PW - D) ~ small threshold (e.g., 1% of midrange ml per pulse)
  double ml_thresh = 0.01 * ml_per_pulse_mid;
  gSpa.min_pw_ms = D_ms + (ml_thresh / (k>0?k:1e-9));
  StaticJsonDocument<256> out; out["ok"]=true; out["min_pw_ms"]=gSpa.min_pw_ms; String s; serializeJson(out,s); server.send(200,"application/json",s);
}
