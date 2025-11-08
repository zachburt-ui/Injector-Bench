
// math_flow.ino
#include "globals.h"

// Convert per-injector mL collected over known ON time to rates
// Assumptions: flow_mL array holds collected volume per injector (mL) for the test
// on_time_s array is computed from injOnTimeUs
static inline double ml_to_cc(double ml) { return ml; } // 1:1
static inline double mlps_to_ccmin(double mlps) { return mlps * 60.0; }
static inline double grams_per_s(double mlps, double rho) { return mlps * rho; } // rho g/mL
static inline double lb_per_hr(double g_per_s) { return g_per_s * 3600.0 / 453.59237; }

void compute_flow_arrays(const float flow_mL[8], const double on_time_s[8],
                         double cc_min[8], double lbs_hr[8], double ml_per_s[8], double g_per_s[8], double rho) {
  for (int i=0;i<8;i++) {
    double t = on_time_s[i];
    double ml = flow_mL[i];
    double mlps = (t > 0.0) ? (ml / t) : 0.0;
    ml_per_s[i] = mlps;
    cc_min[i] = mlps_to_ccmin(mlps);
    g_per_s[i] = grams_per_s(mlps, rho);
    lbs_hr[i]  = lb_per_hr(g_per_s[i]);
  }
}

// Simple table builders (first-pass approximations)
// P59 injector flow vs delta kPa: assume test at psi_nom and near-linear scaling with sqrt(dp ratio)
void build_p59_table(double base_cc_min[8], double psi_nom, double out_ccmin_kpa[][8], int rows) {
  // kPa deltas typical: 0..80 (we'll use 0, 20, 40, 60, 80 as 5 rows for example)
  // Convert psi to kPa: 1 psi = 6.89476 kPa
  const double psi_to_kpa = 6.89476;
  double dp_nom_kpa = psi_nom * psi_to_kpa;
  double kpa_rows[5] = {0, 20, 40, 60, 80};
  for (int r=0;r<rows;r++) {
    double dp = dp_nom_kpa + kpa_rows[r];
    double scale = sqrt(dp / dp_nom_kpa);
    for (int i=0;i<8;i++) out_ccmin_kpa[r][i] = base_cc_min[i] * scale;
  }
}

// Holley battery voltage offset table (approximation): apply mild scaling per voltage step
void build_holley_volt_table(double base_cc_min[8], double vref, double out_ccmin_v[][8], const double volts[], int rows) {
  for (int r=0;r<rows;r++) {
    double v = volts[r];
    // naive linear correction: higher voltage slightly increases effective flow
    double scale = (v / vref);
    for (int i=0;i<8;i++) out_ccmin_v[r][i] = base_cc_min[i] * scale;
  }
}
