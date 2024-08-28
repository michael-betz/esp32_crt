#pragma once

void init_encoder(void);

// Clear absolute encoder value
void clear_encoder();

// Returns difference in encoder ticks since last call
//
// if btns is given, gives the state of up to 8 buttons
// the lowest 8 bits are high (and stay high) when the corresponding button is pressed
// the next 8 bits are set for one cycle on the rising edge (on pushed)
// the next 8 bits are set for one cycle on the falling edge (on released)
//
// If encoder_absolute is given, the raw, absolute, full resolution encoder value is written there
int get_encoder(unsigned *btns, int *encoder_absolute);
