
// storage.ino
#include "globals.h"

void loadSettingsFromNVS() {
  prefs.begin("bench", true);
  settings.scales_on = prefs.getBool("scales_on", settings.scales_on);
  settings.targetMl  = prefs.getFloat("targetMl", settings.targetMl);
  settings.psi_nom   = prefs.getFloat("psi_nom", settings.psi_nom);
  settings.volt_nom  = prefs.getFloat("volt_nom", settings.volt_nom);
  String fluid = prefs.getString("fluid", settings.fluid);
  sta_ssid = prefs.getString("sta_ssid", "");
  sta_pass = prefs.getString("sta_pass", "");
  prefs.end();
  if (fluid.length() < sizeof(settings.fluid)) fluid.toCharArray(settings.fluid, sizeof(settings.fluid));
}

void saveSettingsToNVS() {
  prefs.begin("bench", false);
  prefs.putBool("scales_on", settings.scales_on);
  prefs.putFloat("targetMl", settings.targetMl);
  prefs.putFloat("psi_nom", settings.psi_nom);
  prefs.putFloat("volt_nom", settings.volt_nom);
  prefs.putString("fluid", settings.fluid);
  prefs.end();
}
