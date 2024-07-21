#ifndef I2S_H
#define I2S_H

#include <stdint.h>

// init and start the DMA output
void i2s_init(void);

// Add a single sample to the DMA buffer
void push_sample(uint16_t val_a, uint16_t val_b, uint16_t val_c, uint16_t val_d);

#endif
