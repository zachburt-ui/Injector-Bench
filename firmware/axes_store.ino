// axes_store.ino
#include "globals.h"

Axes gAxes;

void axes_load_from_nvs() {
  prefs.begin("bench", true);
  String s = prefs.getString("axes_json", "");
  prefs.end();
  if (s.length() == 0) return;
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, s)) return;
  JsonArray v = doc["volts"]; if (!v.isNull()) { gAxes.n_volts = min(6, (int)v.size()); for (int i=0;i<gAxes.n_volts;i++) gAxes.volts[i] = v[i]; }
  JsonArray k = doc["dkp"];   if (!k.isNull()) { gAxes.n_dkp   = min(5, (int)k.size()); for (int i=0;i<gAxes.n_dkp;i++)   gAxes.dkp[i]   = k[i]; }
  JsonArray sp= doc["spa"];   if (!sp.isNull()){ gAxes.n_spa   = min(6, (int)sp.size());for (int i=0;i<gAxes.n_spa;i++)   gAxes.spa_bins[i] = sp[i]; }
}

void axes_save_to_nvs() {
  StaticJsonDocument<512> doc;
  JsonArray v = doc.createNestedArray("volts"); for (int i=0;i<gAxes.n_volts;i++) v.add(gAxes.volts[i]);
  JsonArray k = doc.createNestedArray("dkp");   for (int i=0;i<gAxes.n_dkp;i++)   k.add(gAxes.dkp[i]);
  JsonArray sp= doc.createNestedArray("spa");   for (int i=0;i<gAxes.n_spa;i++)   sp.add(gAxes.spa_bins[i]);
  String out; serializeJson(doc, out);
  prefs.begin("bench", false);
  prefs.putString("axes_json", out);
  prefs.end();
}
