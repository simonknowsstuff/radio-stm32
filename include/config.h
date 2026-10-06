#pragma once
#include <Arduino.h>

// I2C Pins
#define I2C_SCL PB6
#define I2C_SDA PB7

// Potentiometers
#define PIN_POT_TUNE PA6 // A6 - Radio tuning
#define PIN_POT_VOL  PA7 // A7 - Volume

// Push Buttons (Active Low / Pull-up)
#define PIN_BTN_BACK   PA0 // A0 - Back button
#define PIN_BTN_SELECT PA1 // A1 - Select button
#define PIN_BTN_DOWN   PA2 // A2 - Down button
#define PIN_BTN_UP     PA3 // A3 - Up button
#define PIN_BTN_FM_DEC PA4 // A4 - FM value decrease 0.1 MHz
#define PIN_BTN_FM_INC PA5 // A5 - FM value increase 0.1 MHz

// System Limits
#define FM_MIN_FREQ 8700  // 87.0 MHz
#define FM_MAX_FREQ 10800 // 108.0 MHz
#define VOL_MIN 0
#define VOL_MAX 15
#define NUM_PRESETS 3