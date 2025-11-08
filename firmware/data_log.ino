
// data_log.ino
#include "globals.h"

static const char* LOG_DIR = "/logs";

static String nowIso() {
  // Without RTC, we just return millis-based pseudo timestamp
  char buf[48];
  snprintf(buf, sizeof(buf), "t+%lu", (unsigned long)millis());
  return String(buf);
}

void log_append(const JsonDocument& doc) {
  if (!LittleFS.exists(LOG_DIR)) LittleFS.mkdir(LOG_DIR);
  String name = String(LOG_DIR) + "/" + nowIso() + ".json";
  File f = LittleFS.open(name, "w");
  if (!f) return;
  String out; serializeJson(doc, out);
  f.print(out);
  f.close();
}

// Enumerate simple list (names only)
void log_list(JsonDocument& out) {
  JsonArray arr = out.createNestedArray("files");
  File dir = LittleFS.open(LOG_DIR, "r");
  if (!dir) return;
  File f;
  while (true) {
    f = dir.openNextFile();
    if (!f) break;
    if (!f.isDirectory()) arr.add(String(f.name()));
    f.close();
  }
  dir.close();
}
