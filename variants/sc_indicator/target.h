#pragma once

#define RADIOLIB_STATIC_ONLY 1
#include <RadioLib.h>
#include <helpers/radiolib/RadioLibWrappers.h>
#include <helpers/ESP32Board.h>
#include <helpers/radiolib/CustomSX1262Wrapper.h>
#include <helpers/AutoDiscoverRTCClock.h>
#include <helpers/sensors/EnvironmentSensorManager.h>
#ifdef DISPLAY_CLASS
  #include "SCIndicatorDisplay.h"
//  #include <helpers/ui/NullDisplayDriver.h>
  #include <helpers/ui/MomentaryButton.h>
#endif

// Defined here because of IO_EXPANDER ...
//#define P_LORA_DIO_1 (IO_EXPANDER|3)
//#define P_LORA_NSS (IO_EXPANDER|0)
//#define P_LORA_RESET (IO_EXPANDER|1)
//#define P_LORA_BUSY (IO_EXPANDER|2)
// #define P_LORA_SCLK 41
// #define P_LORA_MISO 47
// #define P_LORA_MOSI 48
// #define SPI_SCLK 41
// #define SPI_MISO 47
// #define SPI_MOSI 48

extern ESP32Board board;
extern WRAPPER_CLASS radio_driver;
extern ESP32RTCClock rtc_clock;
extern EnvironmentSensorManager sensors;

#ifdef DISPLAY_CLASS
  extern DISPLAY_CLASS display;
  extern MomentaryButton user_btn;
#endif

bool radio_init();
uint32_t radio_get_rng_seed();
void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr);
void radio_set_tx_power(uint8_t dbm);
mesh::LocalIdentity radio_new_identity();