#pragma once
struct FluidProp { const char* name; double rho20; double alpha; };
static const FluidProp FLUIDS[] = {
  {"Gasoline", 0.745, 0.00110},
  {"E85",      0.790, 0.00085},
  {"Diesel",   0.832, 0.00080},
  {"Kerosene", 0.810, 0.00090},
  {"n-Heptane",0.684, 0.00110},
  {"Water",    0.998, 0.00030},
  {"Custom",   0.744, 0.00090},
};
static inline double fluid_rho_at(double rho20, double alpha, double tempC){ return rho20 * (1.0 - alpha * (tempC - 20.0)); }