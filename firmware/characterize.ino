
// characterize.ino
#include "globals.h"
#include "characterize.h"

void report_reset();

static uint32_t runStartMs = 0;
static uint32_t runDurationMs = 0;

void characterize_begin(const RunConfig& cfg) {
  runCfg = cfg;
  report_reset();
  seqIndex = 0;
  runStopping = false;
  runActive = true;
  runMode = cfg.mode;
  selectedMask = cfg.inj_mask;
  runStartMs = millis();
  runDurationMs = cfg.seconds * 1000UL;

  if (runMode == RM_DYNAMIC) {
    periodUs = (uint64_t)(1000000.0f / max(1.0f, cfg.hz));
    pwUs = (uint32_t)(cfg.pw_ms * 1000.0f);
    dynamic_timer_start();
  } else {
    // STATIC or CLEAN
    static_begin_mask(selectedMask);
  }
}

void characterize_stop_graceful() {
  if (runMode == RM_DYNAMIC) {
    runStopping = true;
  } else {
    characterize_stop_now();
  }
}

void characterize_stop_now() {
  runActive = false;
  runStopping = false;
  if (runMode == RM_DYNAMIC) dynamic_timer_stop();
  else static_all_off();
  runMode = RM_IDLE;
}

void characterize_tick() {
  if (!runActive) return;
  uint32_t elapsedMs = millis() - runStartMs;
  if (elapsedMs >= runDurationMs) characterize_stop_graceful();
  if (runMode == RM_STATIC || runMode == RM_CLEAN) static_tick_accumulate();
}
