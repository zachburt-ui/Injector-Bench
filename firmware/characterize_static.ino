
// characterize_static.ino
#include "globals.h"

extern const uint8_t INJ_PINS[8];
extern volatile uint64_t injOnTimeUs[8];
extern volatile uint32_t selectedMask;
extern volatile RunMode runMode;
extern volatile bool runActive;

static uint32_t lastMs_static = 0;
void static_begin_mask(uint32_t mask) {
  for (int i=0;i<8;i++) {
    digitalWrite(INJ_PINS[i], (mask & (1u<<i)) ? HIGH : LOW);
  }
  lastMs_static = millis();
}
void static_tick_accumulate() {
  uint32_t nowMs = millis();
  uint32_t dt = nowMs - lastMs_static;
  lastMs_static = nowMs;
  for (int i=0;i<8;i++) {
    if (selectedMask & (1u<<i)) injOnTimeUs[i] += (uint64_t)dt * 1000ULL;
  }
}
void static_all_off() {
  for (int i=0;i<8;i++) digitalWrite(INJ_PINS[i], LOW);
}
