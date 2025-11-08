
// wizard_state.ino
#include "globals.h"

// Track wizard step; firmware mostly stateless but this helps UI recover mid-run
static uint8_t wizardStep = 0;
void wizard_reset() { wizardStep = 0; }
void wizard_set(uint8_t s) { wizardStep = s; }
uint8_t wizard_get() { return wizardStep; }
