
// characterize_dynamic.ino
#include "globals.h"

extern hw_timer_t* dynTimer;
extern portMUX_TYPE injMux;

extern volatile RunMode runMode;
extern volatile bool runActive;
extern volatile bool runStopping;
extern volatile uint8_t seqIndex;
extern volatile uint64_t periodUs;
extern volatile uint32_t pwUs;
extern volatile uint32_t selectedMask;
extern volatile uint64_t injOnTimeUs[8];

extern const uint8_t INJ_PINS[8];

// ISR bookkeeping
volatile uint64_t lastRiseUs_dyn = 0;
volatile bool pulseHigh_dyn = false;

void IRAM_ATTR dynTimerISR_v3() {
  const uint64_t nowUs = (uint64_t)esp_timer_get_time();
  if (runMode != RM_DYNAMIC || !runActive) return;

  if (!pulseHigh_dyn) {
    for (int tries=0; tries<8; ++tries) {
      if (selectedMask & (1u << seqIndex)) break;
      seqIndex = (seqIndex + 1) & 7;
    }
    uint8_t ch = seqIndex;
    for (int i=0;i<8;i++) digitalWrite(INJ_PINS[i], LOW);
    digitalWrite(INJ_PINS[ch], HIGH);
    lastRiseUs_dyn = nowUs;
    pulseHigh_dyn = true;
  } else {
    if ((nowUs - lastRiseUs_dyn) >= pwUs) {
      uint8_t ch = seqIndex;
      digitalWrite(INJ_PINS[ch], LOW);
      injOnTimeUs[ch] += (nowUs - lastRiseUs_dyn);
      pulseHigh_dyn = false;
      seqIndex = (seqIndex + 1) & 7;
      if (runStopping && seqIndex == 0) runActive = false;
    }
  }
}
