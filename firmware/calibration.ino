
// calibration.ino
#include "globals.h"

void calibration_apply_from_json(const JsonDocument& in) {
  if (in.containsKey("rho")) telem.rho = in["rho"];
  // TODO: add voltage/pressure calibration coefficients if your UI sends them
}
