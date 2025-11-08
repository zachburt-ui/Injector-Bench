// characterize_suite.ino
#include "globals.h"

CharSuite gSuite;
volatile SuiteStatus gSuiteStatus = {SP_NONE, -1, false, {false,false,false,false,false,false}, {false,false,false,false,false}}; // separate p59V tracked by holley_filled for volts if needed

static const double P59_KPA_ROWS[5] = {0,20,40,60,80};
static const double HOLLEY_VOLTS[6] = {9.0,10.5,12.0,13.5,14.0,15.0};

// Helper to convert current run buffers into base_cc_min using math from report
extern void compute_flow_arrays(const float flow_mL[8], const double on_time_s[8],
                         double cc_min[8], double lbs_hr[8], double ml_per_s[8], double g_per_s[8], double rho);

// Called after each subtest to stash results into a target row
void suite_store_base_from_current() {
  float flow_mL[8]; for (int i=0;i<8;i++) flow_mL[i] = scales_vals[i];
  double on_s[8]; for (int i=0;i<8;i++) on_s[i] = (double)injOnTimeUs[i] / 1e6;
  double cc_min[8], lbs_hr[8], mlps[8], gps[8];
  compute_flow_arrays(flow_mL, on_s, cc_min, lbs_hr, mlps, gps, telem.rho);
  for (int i=0;i<8;i++) gSuite.base_cc_min[i] = cc_min[i];
}

void suite_build_p59() {
  // Scale from base_cc_min according to sqrt(dp/dp_nom)
  const double psi_to_kpa = 6.89476;
  double dp_nom_kpa = settings.psi_nom * psi_to_kpa;
  for (int r=0;r<5;r++) {
    double dp = dp_nom_kpa + P59_KPA_ROWS[r];
    double scale = sqrt(dp / dp_nom_kpa);
    for (int i=0;i<8;i++) gSuite.p59_ccmin_kpa[r][i] = gSuite.base_cc_min[i] * scale;
  }
}
void suite_build_holley() {
  // Simple proportional scaling vs nominal voltage (can refine later with latency/offset curves)
  for (int r=0;r<6;r++) {
    double scale = HOLLEY_VOLTS[r] / settings.volt_nom;
    for (int i=0;i<8;i++) gSuite.holley_ccmin_v[r][i] = gSuite.base_cc_min[i] * scale;
  }
}


void suite_clear() {
  gSuiteStatus.phase = SP_NONE; gSuiteStatus.index = -1;
  gSuiteStatus.base_filled = false;
  for (int i=0;i<6;i++) gSuiteStatus.holley_filled[i]=false;
  for (int r=0;r<5;r++) gSuiteStatus.p59_filled[r]=false;
  for (int i=0;i<8;i++) gSuite.base_cc_min[i]=0;
  for (int r=0;r<5;r++) for (int i=0;i<8;i++) gSuite.p59_ccmin_kpa[r][i]=0;
  for (int r=0;r<6;r++) for (int i=0;i<8;i++) gSuite.holley_ccmin_v[r][i]=0;
}

void suite_arm(SuitePhase ph, int idx){
  gSuiteStatus.phase = ph;
  gSuiteStatus.index = idx;
}

// Measure from current run buffers into armed slot
void suite_commit_from_current() {
  float flow_mL[8]; for (int i=0;i<8;i++) flow_mL[i] = scales_vals[i];
  double on_s[8]; for (int i=0;i<8;i++) on_s[i] = (double)injOnTimeUs[i] / 1e6;
  double cc_min[8], lbs_hr[8], mlps[8], gps[8];
  compute_flow_arrays(flow_mL, on_s, cc_min, lbs_hr, mlps, gps, telem.rho);

  if (gSuiteStatus.phase == SP_BASE) {
    for (int i=0;i<8;i++) gSuite.base_cc_min[i] = cc_min[i];
    gSuiteStatus.base_filled = true;
  } else if (gSuiteStatus.phase == SP_HOLLEY) {
    int r = gSuiteStatus.index;
    if (r >=0 && r < 6) { for (int i=0;i<8;i++) gSuite.holley_ccmin_v[r][i] = cc_min[i]; gSuiteStatus.holley_filled[r] = true; }
  } else if (gSuiteStatus.phase == SP_P59) {
    int r = gSuiteStatus.index;
    if (r >=0 && r < 5) { for (int i=0;i<8;i++) gSuite.p59_ccmin_kpa[r][i] = cc_min[i]; gSuiteStatus.p59_filled[r] = true; }
  }
}


static void regress_offset_ms(const double* pw_ms, const double* ml_per_pulse, int n, double* out_offset_ms, double* out_k) {
  // Fit y = k*(pw - D) = k*pw - k*D => linear: y = a*pw + b; then D = -b/a
  double Sx=0, Sy=0, Sxx=0, Sxy=0;
  for (int i=0;i<n;i++){ Sx+=pw_ms[i]; Sy+=ml_per_pulse[i]; Sxx+=pw_ms[i]*pw_ms[i]; Sxy+=pw_ms[i]*ml_per_pulse[i]; }
  double denom = (n*Sxx - Sx*Sx);
  double a = (denom!=0) ? (n*Sxy - Sx*Sy)/denom : 0.0;
  double b = (n!=0) ? (Sy - a*Sx)/n : 0.0;
  if (out_k) *out_k = a;
  if (out_offset_ms) *out_offset_ms = (a!=0)? -b/a : 0.0;
}
