// web_api.ino (wiring only)
#include "globals.h"
void wireApiRoutes() {
  wireApi_Characterize();
  wireApi_SettingsScales();
  wireApi_Reports();
  wireApi_WifiNet();
  wireApi_Suite();
  wireApi_Axes();
  wireApi_SensorsCal();
  wireApi_Clean();
  wireApi_Fluid();
  wireApi_Wifi();
  wireApi_Pump();
  wireApi_Scales();
}
