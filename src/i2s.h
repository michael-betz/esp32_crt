#ifndef I2S_H
#define I2S_H

typedef struct {
    int16_t x;
    int16_t y;
    int16_t brightness;
} draw_list_t;

void i2s_init(void);
void i2s_write_chunk(void);

#endif
