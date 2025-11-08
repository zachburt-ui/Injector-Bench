
// reports.ino
#include "globals.h"
extern CharSuite gSuite;
extern volatile SuiteStatus gSuiteStatus;
extern SpaData gSpa;

void report_reset() {
  for (int i=0;i<8;i++) injOnTimeUs[i] = 0;
}

void report_build_json(JsonDocument& doc) {
  JsonArray flow = doc.createNestedArray("flow_mL");
  for (int i=0;i<8;i++) flow.add(scales_vals[i]);
  JsonArray ontime = doc.createNestedArray("on_time_s");
  for (int i=0;i<8;i++) ontime.add((double)injOnTimeUs[i] / 1000000.0);
  doc["mode"] = (runMode==RM_DYNAMIC?"dynamic":(runMode==RM_STATIC?"static":(runMode==RM_CLEAN?"clean":"idle")));
}


void report_add_suite(JsonDocument& doc) {
  // status
  JsonObject st = doc.createNestedObject("suite_status");
  st["phase"] = (int)gSuiteStatus.phase; st["index"] = gSuiteStatus.index; st["base_filled"] = gSuiteStatus.base_filled; JsonArray hf = st.createNestedArray("holley_filled"); for(int r=0;r<6;r++) hf.add(gSuiteStatus.holley_filled[r]); JsonArray pf = st.createNestedArray("p59_filled"); for(int r=0;r<5;r++) pf.add(gSuiteStatus.p59_filled[r]);
  // include suite tables if populated
  JsonArray base = doc.createNestedArray("suite_base_ccmin");
  for (int i=0;i<8;i++) base.add(gSuite.base_cc_min[i]);
  JsonArray p59 = doc.createNestedArray("suite_p59_ccmin_kpa");
  for (int r=0;r<5;r++){ JsonArray row = p59.createNestedArray(); for(int i=0;i<8;i++) row.add(gSuite.p59_ccmin_kpa[r][i]); }
  JsonArray hol = doc.createNestedArray("suite_holley_ccmin_v");
  for (int r=0;r<6;r++){ JsonArray row = hol.createNestedArray(); for(int i=0;i<8;i++) row.add(gSuite.holley_ccmin_v[r][i]); }
}

void report_add_suite_p59v(JsonDocument& doc) {
  JsonArray off = doc.createNestedArray("suite_p59_offset_ms_v");
  for (int r=0;r<6;r++){ JsonArray row = off.createNestedArray(); for(int i=0;i<8;i++) row.add(gSuite.p59_offset_ms_v[r][i]); }
}


void report_add_spa(JsonDocument& doc) {
  JsonObject spa = doc.createNestedObject("spa");
  JsonArray bins = spa.createNestedArray("bins_ms");
  JsonArray add  = spa.createNestedArray("adder_ms");
  for (int i=0;i<6;i++){ bins.add(gSpa.bins_ms[i]); add.add(gSpa.adder_ms[i]); }
  spa["min_pw_ms"] = gSpa.min_pw_ms;
}

// helper: convert cc/min to lb/hr per injector using density rho
static double ccmin_to_lbhr(double ccmin, double rho) {
  // cc=ml; ml/min -> g/min = ml/min * rho; to lb/hr: *60 / 453.59237
  return (ccmin * rho) * 60.0 / 453.59237;
}

void report_add_ifr_lbhr(JsonDocument& doc) {
  // derive IFR (lb/hr) vs ΔkPa by unit conversion from suite_p59_ccmin_kpa
  JsonArray p59cc = doc["suite_p59_ccmin_kpa"].as<JsonArray>();
  if (p59cc.isNull()) return;
  JsonArray out = doc.createNestedArray("suite_p59_ifr_lbhr");
  for (JsonVariant row : p59cc) {
    JsonArray r2 = out.createNestedArray();
    for (int i=0;i<8;i++) {
      double ccmin = row[i] | 0.0;
      r2.add(ccmin_to_lbhr(ccmin, doc["rho"] | 0.744));
    }
  }
}

// Holley single "Injector Flow Rate (lb/hr)" helper at nominal row (ΔkPa=0) averaged
void report_add_holley_ifr_nominal(JsonDocument& doc) {
  JsonArray p59cc = doc["suite_p59_ccmin_kpa"].as<JsonArray>();
  if (p59cc.isNull()) return;
  JsonArray row0 = p59cc[0].as<JsonArray>(); // ΔkPa=0 row
  double sum=0; int cnt=0;
  for (int i=0;i<8;i++){ sum += (double)(row0[i] | 0.0); cnt++; }
  double avg_ccmin = cnt? sum/cnt : 0.0;
  double lbhr = (avg_ccmin * (doc["rho"] | 0.744)) * 60.0 / 453.59237;
  doc["holley_ifr_lbhr_nominal"] = lbhr;
}


#include "fluids.h"
extern FluidState gFluid;
static double fluid_rho_at(double rho20, double alpha, double tempC);
static double current_rho(double tempC){ double rho20 = (strcmp(gFluid.name,"Custom")==0)? gFluid.rho_custom : gFluid.rho20; return rho20 * (1.0 - gFluid.alpha * (tempC - 20.0)); }
static double norm_ccmin(double ccmin, double dp_meas, double dp_nom){ if(dp_meas<=0||dp_nom<=0) return ccmin; return ccmin * sqrt(dp_nom/dp_meas); }

void report_add_norm_and_units(JsonDocument& doc){
  double tempC = doc["telemetry"]["tempC"] | 20.0;
  double dp_meas = doc["telemetry"]["pressure"] | (doc["psi_nom"] | 58.0);
  double dp_nom  = doc["psi_nom"] | 58.0;
  double rho = current_rho(tempC);
  doc["rho"] = rho;

  // lbs/hr
  JsonArray cc = doc["cc_min"].as<JsonArray>();
  if(!cc.isNull()){
    JsonArray lb = doc.createNestedArray("lbs_hr");
    for(JsonVariant v: cc){ double ccmin = v | 0.0; double gpm = ccmin*rho; double lbhr=(gpm*60.0)/453.59237; lb.add(lbhr); }
    JsonArray nrm = doc.createNestedArray("cc_min_norm");
    for(JsonVariant v: cc){ double ccmin = v | 0.0; nrm.add( norm_ccmin(ccmin, dp_meas, dp_nom) ); }
  }
}


#include "globals.h"
// Compute arrays for report: convert mL & on_time to cc/min, then lb/hr using rho;
// also compute normalized cc/min if dp given (supply in caller).
void compute_flow_arrays(double flow_mL[8], double on_s[8],
                         double cc_min[8], double lbs_hr[8],
                         double mlps[8], double gps[8],
                         double rho)
{
  for(int i=0;i<8;i++){
    double ml = flow_mL[i];
    double ts = (on_s[i] > 0.001)? on_s[i] : 1.0;
    mlps[i] = ml / ts;             // mL/s
    gps[i]  = mlps[i] * rho / 1000.0; // g/s (rho in kg/L == g/mL)
    cc_min[i] = mlps[i] * 60.0;    // cc/min
    lbs_hr[i] = (gps[i] * 3600.0) / 453.59237; // lb/hr
  }
}


// --- Type-flexible flow array computation (accepts float* or double*) ---
template<typename T>
void compute_flow_arrays_t(const T flow_mL[8], const T on_s[8],
                           T cc_min[8], T lbs_hr[8],
                           T mlps[8], T gps[8],
                           double rho)
{
  for(int i=0;i<8;i++){
    double ml = static_cast<double>(flow_mL[i]);
    double ts = static_cast<double>(on_s[i]);
    if(ts < 0.001) ts = 1.0;
    double mlps_d = ml / ts;                  // mL/s
    double gps_d  = mlps_d * rho / 1000.0;    // g/s  (rho kg/L == g/mL)
    double ccmin  = mlps_d * 60.0;            // cc/min
    double lbhr   = (gps_d * 3600.0) / 453.59237; // lb/hr
    mlps[i]  = static_cast<T>(mlps_d);
    gps[i]   = static_cast<T>(gps_d);
    cc_min[i]= static_cast<T>(ccmin);
    lbs_hr[i]= static_cast<T>(lbhr);
  }
}

// Legacy double* signature kept for compatibility
