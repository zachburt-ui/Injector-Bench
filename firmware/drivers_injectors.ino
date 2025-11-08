
// drivers_injectors.ino
#include "globals.h"

const uint8_t INJ_PINS[8] = { PIN_INJ_1, PIN_INJ_2, PIN_INJ_3, PIN_INJ_4,
                              PIN_INJ_5, PIN_INJ_6, PIN_INJ_7, PIN_INJ_8 };

hw_timer_t* dynTimer = nullptr;
portMUX_TYPE injMux = portMUX_INITIALIZER_UNLOCKED;

volatile RunMode runMode = RM_IDLE;
volatile bool runActive = false;
volatile bool runStopping = false;
volatile uint8_t seqIndex = 0;
volatile uint64_t periodUs = 0;
volatile uint32_t pwUs = 0;
volatile uint32_t selectedMask = 0;
volatile uint64_t injOnTimeUs[8] = {0};

void drivers_init() {
  for (int i=0;i<8;i++) { pinMode(INJ_PINS[i], OUTPUT); digitalWrite(INJ_PINS[i], LOW); }
  dynTimer = timerBegin(0, 80, true); // 1us tick
  // ISR attached in dynamic_timer_start to the v3 handler defined in characterize_dynamic.ino
}

void dynamic_timer_start() {
  timerAttachInterrupt(dynTimer, &dynTimerISR_v3, true);
  timerAlarmWrite(dynTimer, 200, true); // 200us
  timerAlarmEnable(dynTimer);
}

void dynamic_timer_stop() {
  timerAlarmDisable(dynTimer);
  for (int i=0;i<8;i++) digitalWrite(INJ_PINS[i], LOW);
}

void drivers_allOff() { for (int i=0;i<8;i++) digitalWrite(INJ_PINS[i], LOW); }
void drivers_set(uint8_t ch, bool on) { if (ch < 8) digitalWrite(INJ_PINS[ch], on ? HIGH : LOW); }
