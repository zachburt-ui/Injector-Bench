
// web_static.ino
#include "globals.h"

static void handleRoot() {
  File f = LittleFS.open("/index.html", "r");
  if (!f) { server.send(404, "text/plain", "index.html not found"); return; }
  server.streamFile(f, "text/html"); f.close();
}
static void handleNotFound() { handleRoot(); }

void wireStaticRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
}
