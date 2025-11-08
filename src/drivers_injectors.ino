// drivers_injectors.ino — injector driver (LEDC + blocking pulses; one-hot)

#include "globals.h"
#include "pins.h"

const uint8_t INJ_PINS[8] = { PIN_INJ1, PIN_INJ2, PIN_INJ3, PIN_INJ4, PIN_INJ5, PIN_INJ6, PIN_INJ7, PIN_INJ8 };

void drivers_init(){
  for(int i=0;i<8;i++){ pinMode(INJ_PINS[i], OUTPUT); digitalWrite(INJ_PINS[i], LOW); }
}

// one-hot static open
void inject_static(uint8_t idx, uint32_t ms){
  if(idx>7) return;
  digitalWrite(INJ_PINS[idx], HIGH);
  delay(ms);
  digitalWrite(INJ_PINS[idx], LOW);
}

// one-hot dynamic pulses (hz, pw_ms, seconds)
void inject_dynamic(uint8_t idx, float hz, float pw_ms, float seconds){
  if(idx>7) return;
  const uint32_t period_ms = (hz>0)? (uint32_t)(1000.0f/hz) : 100;
  uint32_t t_end = millis() + (uint32_t)(seconds*1000.0f);
  while((int32_t)(millis() - t_end) < 0){
    digitalWrite(INJ_PINS[idx], HIGH);
    delay((uint32_t)pw_ms);
    digitalWrite(INJ_PINS[idx], LOW);
    uint32_t rest = (period_ms>(uint32_t)pw_ms)? (period_ms - (uint32_t)pw_ms) : 0;
    delay(rest);
  }
}
