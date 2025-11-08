#include "globals.h"
#include <ArduinoJson.h>

// Forward decl (template is in reports.ino; we provide a generic wrapper here)
template<typename T>
void compute_flow_arrays_t(const T flow_mL[8], const T on_s[8],
                           T cc_min[8], T lbs_hr[8],
                           T mlps[8], T gps[8],
                           double rho);

// Example telemetry struct access (replace with your real vars)
extern double g_nominal_rho; // fallback if telemetry.rho missing

void api_report_get(){
  // In a real build, pull these from your last-run buffers.
  // Here we just prepare zeroed arrays to keep compile/link happy.
  float flow_mL[8]={0}, on_s[8]={1};
  float cc_min[8]={0}, lbs_hr[8]={0}, mlps[8]={0}, gps[8]={0};
  double rho = g_nominal_rho; // or telem.rho

  compute_flow_arrays_t<float>(flow_mL, on_s, cc_min, lbs_hr, mlps, gps, rho);

  StaticJsonDocument<1024> d;
  JsonArray cc = d.createNestedArray("cc_min");
  JsonArray lb = d.createNestedArray("lbs_hr");
  for(int i=0;i<8;i++){ cc.add(cc_min[i]); lb.add(lbs_hr[i]); }
  String out; serializeJson(d,out);
  server.send(200,"application/json",out);
}
// end file
