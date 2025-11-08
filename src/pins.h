// pins.h — ESP32-WROOM pin map (V5.1)
#pragma once
// SPI (shared: ADS1256 x2, MAX31865)
#define PIN_SCK    18
#define PIN_MISO   19
#define PIN_MOSI   23
// ADS1256 A (scales 1-4)
#define PIN_ADS_A_CS   5
#define PIN_ADS_A_DRDY 36
// ADS1256 B (scales 5-8)
#define PIN_ADS_B_CS   4
#define PIN_ADS_B_DRDY 39
// MAX31865 (PT-100)
#define PIN_MAX_CS     15
#define PIN_MAX_DRDY   35  // optional
// Injectors (one-hot)
#define PIN_INJ1 13
#define PIN_INJ2 14
#define PIN_INJ3 16
#define PIN_INJ4 17
#define PIN_INJ5 25
#define PIN_INJ6 27
#define PIN_INJ7 4   // strap: keep pulled-up at boot
#define PIN_INJ8 5   // strap: keep pulled-up at boot
// Pump trigger (active-HIGH)
#define PIN_PUMP 26
// Analog sensors
#define PIN_VSENSE 33
#define PIN_PSENSE 34
