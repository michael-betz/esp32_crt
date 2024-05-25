#pragma once

// -------------------------
//  GPIO number definitions
// -------------------------
// DACs
#define PIN_SCK 12
#define PIN_SDO 14
#define PIN_CS_N_A 27
#define PIN_CS_N_B 4
#define PIN_CS_N_B_NAME IO_MUX_GPIO4_REG

// LEDs
#define PIN_LED_R 16  // D1
#define PIN_LED_G 17  // D2
#define PIN_LED_B 5   // D3

// UI
#define PIN_KNOB_A 33
#define PIN_KNOB_B 32
#define PIN_BUTTON 35

// Power
#define PIN_PD_BAD 26  // High when 12 V is likely not present from USB-PD
#define PIN_HVPS_DIS 25
