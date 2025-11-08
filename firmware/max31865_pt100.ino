// max31865_pt100.ino — PT-100 via MAX31865 (SPI)
//  SCK=18, MISO=19, MOSI=23, CS=15, DRDY=35 (optional)
// Replace stub reads with your MAX31865 library for accurate °C.

#include "globals.h"
#include "pins.h"

void pt100_begin(){
  pinMode(PIN_MAX_CS, OUTPUT); digitalWrite(PIN_MAX_CS, HIGH);
  // init MAX31865 here (library)
}
double pt100_read_c(){
  // stub: gentle drift around 20–22 °C
  static double t=21.0; t += (random(0,100)-50)/10000.0; return t;
}
