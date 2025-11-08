
#pragma once
// ---- Common includes and shared types ----
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <ESPmDNS.h>

// ---- Pins (see pins.h) ----
#include "pins.h"

// ---- Globals ----
extern Preferences prefs;
extern WebServer server;

// WiFi state
extern String sta_ssid;
extern String sta_pass;
extern IPAddress staIP, apIP;

// Telemetry & Settings model
struct Telemetry {
  float voltage;
  float pressure;
  float tempC;
  float rho;
};
struct Settings {
  bool  scales_on;
  float targetMl;
  float psi_nom;
  float volt_nom;
  char  fluid[16];
};

extern Telemetry telem;
extern Settings  settings;
extern float     scales_vals[8];

// ---- Prototypes used across tabs ----
void startAP();
bool connectSTA(unsigned long timeoutMs=15000);
void loadSettingsFromNVS();
void saveSettingsToNVS();
void wireStaticRoutes();
void wireApiRoutes();
String readBody();
void sendOK();


// ---- Characterize run config/state ----
enum RunMode : uint8_t { RM_IDLE=0, RM_STATIC=1, RM_DYNAMIC=2, RM_CLEAN=3 };

struct RunConfig {
  RunMode mode;
  float   hz;        // dynamic frequency
  float   pw_ms;     // dynamic pulse width (ms)
  uint32_t seconds;  // total run time (s)
  uint8_t inj_mask;  // bitmask of enabled injectors (1=on)
};

extern volatile RunMode runMode;
extern RunConfig runCfg;

// Per-injector accumulated "on" microseconds during the test window
extern volatile uint64_t injOnTimeUs[8];

// Internal run flags
extern volatile bool runActive;
extern volatile bool runStopping;   // graceful stop requested
extern volatile uint8_t seqIndex;   // 0..7 current injector in dynamic mode
extern volatile uint64_t periodUs;  // dynamic period
extern volatile uint32_t pwUs;      // dynamic pulse width (us)
extern volatile uint32_t selectedMask; // cached mask for speed

// Driver API
void drivers_init();
void drivers_allOff();
void drivers_set(uint8_t ch, bool on);
void dynamic_timer_start();
void dynamic_timer_stop();

// Characterize API
void characterize_begin(const RunConfig& cfg);
void characterize_stop_graceful();  // finish current cycle
void characterize_stop_now();       // immediate
void characterize_tick();           // called from loop for time-based stop

// Report building helper
void report_reset();


// ==== Reports ====
void report_reset();
void report_build_json(JsonDocument& doc);

// ==== Calibration ====
void calibration_apply_from_json(const JsonDocument& in);

// ==== Scales Setup ====
void scales_init();
void scales_tare();
void scales_snapshot(float out[8]);

// ==== API wiring (split) ====
void wireApi_Characterize();
void wireApi_SettingsScales();
void wireApi_Reports();
void wireApi_WifiNet();


// ==== Characterize Suite Buffers ====
// We'll capture flows at specific deltas/voltages per injector.
struct CharSuite {
  // P59: Injector Offset (dead time) vs Battery Voltage (ms)
  double p59_offset_ms_v[6][8];
  // Base (nominal) flow cc/min at settings.psi_nom & settings.volt_nom
  double base_cc_min[8];
  // P59: rows for ΔkPa = {0,20,40,60,80}
  double p59_ccmin_kpa[5][8];
  // Holley: rows for battery volts = {9.0,10.5,12.0,13.5,14.0,15.0}
  double holley_ccmin_v[6][8];
};

extern CharSuite gSuite;

enum SuitePhase : uint8_t { SP_NONE=0, SP_BASE=1, SP_HOLLEY=2, SP_P59=3, SP_P59V=4 };

struct SuiteStatus {
  SuitePhase phase;
  int index; // row index within phase
  bool base_filled;
  bool holley_filled[6];
  bool p59_filled[5];
};

extern volatile SuiteStatus gSuiteStatus;

// Suite controls
void suite_clear();
void suite_arm(SuitePhase ph, int idx);
void suite_commit_from_current(); // compute cc/min from current buffers and store into armed slot


void wireApi_Suite();


// ==== Short Pulse Adder (SPA) ====
struct SpaData {
  // PW bins in ms used for SPA sweep (fixed count 6)
  double bins_ms[6] = {0.8, 1.2, 1.6, 2.0, 2.4, 2.8};
  // Computed adder (ms) per bin (averaged across injectors for now)
  double adder_ms[6] = {0,0,0,0,0,0};
  // Estimated minimum injector pulse (ms)
  double min_pw_ms = 0.0;
};
extern SpaData gSpa;


// ==== Characterize Axes (configurable) ====
struct Axes {
  // Default breakpoints; user can override via /api/axes
  double volts[6] = {9.0, 10.5, 12.0, 13.5, 14.0, 15.0};
  int    n_volts = 6;
  double dkp[5]  = {0, 20, 40, 60, 80}; // ΔkPa above nominal
  int    n_dkp = 5;
  double spa_bins[6] = {0.8, 1.2, 1.6, 2.0, 2.4, 2.8};
  int    n_spa = 6;
};
extern Axes gAxes;

// persist/load axes
void axes_load_from_nvs();
void axes_save_to_nvs();

void wireApi_Axes();

void wireApi_SensorsCal();

void wireApi_Clean();

// ==== Fluid selection & calibration ====
struct FluidState { char  name[16] = "Gasoline"; double rho20 = 0.745; double alpha = 0.00110; double rho_custom = 0.744; };
extern FluidState gFluid;
void fluid_load_from_nvs();
void fluid_save_to_nvs();

void wireApi_Fluid();

void wifi_connect_init();
void wireApi_Wifi();

void wireApi_Pump();
void pump_on(); void pump_off(); bool pump_is_on();

void wireApi_Scales();
