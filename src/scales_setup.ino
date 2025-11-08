
// scales_setup.ino
#include "globals.h"

void scales_init() {
  // TODO: init HX711/ADS etc. For now it's a stub.
}
void scales_tare() {
  for (int i=0;i<8;i++) scales_vals[i] = 0;
}
void scales_snapshot(float out[8]) {
  for (int i=0;i<8;i++) out[i] = scales_vals[i];
}
