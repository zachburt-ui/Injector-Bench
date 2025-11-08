// scale_cal.ino — simple helpers to set tare/span via API
#include "globals.h"

// Optional: functions you can call from calibration endpoints if needed
void scale_set_tare(int ch, double tare){ /* stored via /api/scales */ }
void scale_set_span(int ch, double span){ /* stored via /api/scales */ }
