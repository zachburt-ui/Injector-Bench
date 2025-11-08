
// api_reports.ino
#include "globals.h"


static void api_report_get() {
  // Build current report from live buffers
  StaticJsonDocument<2048> doc;
  doc["ok"] = true;
  report_build_json(doc);
  report_add_suite(doc);

  // Also compute rates using math_flow
  float flow_mL[8]; for (int i=0;i<8;i++) flow_mL[i] = scales_vals[i];
  double on_s[8]; for (int i=0;i<8;i++) on_s[i] = (double)injOnTimeUs[i] / 1000000.0;

  double cc_min[8], lbs_hr[8], mlps[8], gps[8];
  compute_flow_arrays(flow_mL, on_s, cc_min, lbs_hr, mlps, gps, telem.rho);

  JsonArray ccmin = doc.createNestedArray("cc_min");
  JsonArray lbhr  = doc.createNestedArray("lbs_hr");
  for (int i=0;i<8;i++) { ccmin.add(cc_min[i]); lbhr.add(lbs_hr[i]); }

  // Tables
  double p59[5][8]; build_p59_table(cc_min, settings.psi_nom, p59, 5);
  JsonArray p59tab = doc.createNestedArray("p59_ccmin_kpa");
  for (int r=0;r<5;r++){ JsonArray row = p59tab.createNestedArray(); for(int i=0;i<8;i++) row.add(p59[r][i]); }

  const double volts[6] = {9.0, 10.5, 12.0, 13.5, 14.0, 15.0};
  double holley[6][8]; build_holley_volt_table(cc_min, settings.volt_nom, holley, volts, 6);
  JsonArray holtab = doc.createNestedArray("holley_ccmin_vs_volt");
  for (int r=0;r<6;r++){ JsonArray row = holtab.createNestedArray(); for(int i=0;i<8;i++) row.add(holley[r][i]); }

  // Include config snapshot
  doc["psi_nom"] = settings.psi_nom;
  doc["volt_nom"] = settings.volt_nom;
  doc["rho"] = telem.rho;

  // Log this snapshot
  log_append(doc);

  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

static void api_logs() {
  StaticJsonDocument<1024> doc;
  doc["ok"] = true;
  log_list(doc);
  String out; serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void wireApi_Reports() {
  server.on("/api/report", HTTP_GET, api_report_get);
  server.on("/api/logs",   HTTP_GET, api_logs);
}
