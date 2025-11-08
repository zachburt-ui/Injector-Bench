
// utils.ino
#include "globals.h"

String readBody() {
  String body;
  if (server.hasArg("plain")) body = server.arg("plain");
  while (server.client().available()) {
    body += char(server.client().read());
    if (body.length() > 16384) break;
  }
  return body;
}
void sendOK() {
  server.send(200, "application/json", "{\"ok\":true}");
}
