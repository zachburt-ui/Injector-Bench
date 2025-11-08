# Injector Bench Firmware V5

This repository contains the Arduino sketch that ships on the Injector Bench V5
controller (ESP32).  The project is organised as Arduino "tabs" inside the
`firmware/` directory so it can be opened directly in the Arduino IDE.

## Building
1. Install the ESP32 board support package in the Arduino IDE.
2. Install the required libraries: `ArduinoJson`, `ESPmDNS`, and `Preferences`
   (bundled with the ESP32 core).
3. Open `firmware/InjectorBench_PureFS_Tabs.ino` in the IDE and select the
   appropriate ESP32 target.
4. Build and upload to the controller.

LittleFS is used for the on-device web UI.  The single-page app lives at
`firmware/index.html` and is served directly from program memory.

## Firmware layout
The sketch is split into focused modules:

- `net_wifi.ino`, `wifi_connect.ino` – Wi-Fi station/AP management and MDNS.
- `api_wifi_net.ino`, `api_settings_scales.ino`, `api_fluid.ino`,
  `api_clean.ino`, `api_axes.ino`, `api_sensors_cal.ino`, `api_suite.ino`,
  `api_reports.ino`, `api_characterize.ino` – HTTP JSON APIs consumed by the
  front-end.
- `drivers_injectors.ino`, `pump_control.ino`, `ads1256_dual.ino`,
  `max31865_pt100.ino` – hardware interfaces for injectors, pump, scales, and
  temperature measurement.
- `characterize*.ino`, `reports.ino`, `math_flow.ino`, `wizard_state.ino` – run
  control, data capture, and reporting helpers.
- `storage.ino`, `axes_store.ino`, `fluid_store.ino` – persistence of user
  settings in NVS.

## REST API
The firmware exposes JSON endpoints on port 80.  Key routes include:

- `GET /api/telemetry` – latest voltage, pressure, temperature, and scale data.
- `GET/POST /api/settings` – bench configuration (flow target, psi, voltage,
  fluid selection, scale enable).
- `GET/POST /api/scales` plus helper actions for tare/span mapping.
- `POST /api/wifi` and `GET /api/net` – Wi-Fi credential updates and status.
- `POST /api/characterize/*` – start/stop injector characterization runs.
- `GET/POST /api/clean` – cleaning cycle controls.
- `GET/POST /api/fluid` – fluid density coefficients and presets.
- `GET/POST /api/axes` – suite axis breakpoints for pressure/voltage grids.
- `GET/POST /api/sensors` – telemetry sensor calibration offsets/gains.
- `GET /api/reports` – formatted results assembled from the most recent run.
- `/api/suite/*` – capture and fit characterization suites, including SPA data.
- `/api/pump` – pump enable/disable.

The API is consumed by the bundled `index.html` single-page application.  All
routes return JSON responses suitable for scripting or manual testing with
`curl`.
