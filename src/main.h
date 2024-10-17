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
#define PIN_LED_0 16  // White
#define PIN_LED_1 17  // Orange
#define PIN_LED_2 5   // Red
#define LED_BRIGHTNESS_0 10
#define LED_BRIGHTNESS_1 40
#define LED_BRIGHTNESS_2 40

// UI
#define PIN_KNOB_A 33
#define PIN_KNOB_B 32
#define PIN_BUTTON 35
#define PIN_WIFI_BUTTON 15

// Power
#define PIN_PD_BAD 26  // High when 12 V is likely not present from USB-PD
#define PIN_HVPS_DIS 25


extern char qr_code_str[32];
extern char *qr_code;
extern unsigned qr_code_w;
