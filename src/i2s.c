#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "soc/i2s_struct.h"
// #include "soc/rtc.h"
#include "soc/io_mux_reg.h"
#include "hal/gpio_hal.h"

#include "i2s.h"
#include "draw.h"
#include "print.h"

// GPIO number definitions
#define PIN_SCK 14
#define PIN_SDO 13
#define PIN_CS_N_A 15
#define PIN_CS_N_B 2
#define PIN_CS_N_B_NAME IO_MUX_GPIO2_REG

// MCP4922 bit definitions
#define A_N 15
#define GA_N 13
#define SHDN_N 12

#define CHUNK_SIZE 4096 * 4

unsigned n_samples = 0;
unsigned n_underflows = 0;

static i2s_chan_handle_t tx_chan;

static IRAM_ATTR bool cb_i2s_tx_undeflow(i2s_chan_handle_t handle, i2s_event_data_t *event, void *user_ctx)
{
	n_underflows++;
    return false;
}

void i2s_init(void)
{
	i2s_chan_config_t tx_chan_cfg = {
		.id = I2S_NUM_1,
		.role = I2S_ROLE_MASTER,
		// Allocate N kB of DMA memory
		.dma_desc_num = 32,  // Number of DMA buffers
		.dma_frame_num = 1023,  // Size of one DMA buffer in 4 byte frames
		.auto_clear = true,
	};
	ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));

	i2s_std_config_t tx_std_cfg = {
		.clk_cfg  = {
			.sample_rate_hz = 200000,  // not relevant
			.clk_src = SOC_MOD_CLK_PLL_F160M,
			// .clk_src = SOC_MOD_CLK_APLL,
			.mclk_multiple = I2S_MCLK_MULTIPLE_128
		},
		.slot_cfg = {
			.data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
			.slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
			.slot_mode = I2S_SLOT_MODE_STEREO,
			.slot_mask = I2S_STD_SLOT_BOTH,
			.ws_width = 16,
			.ws_pol = false,
			.bit_shift = false,
			.msb_right = true
		},
		.gpio_cfg = {
			.mclk = I2S_GPIO_UNUSED,
			.bclk = PIN_SCK,
			.ws   = PIN_CS_N_A,  // This is used as /CS0
			.dout = PIN_SDO,
			.din  = -1,
			.invert_flags = {
				.mclk_inv = false,
				.bclk_inv = false,
				.ws_inv   = false,
			},
		},
	};
	ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));

	i2s_event_callbacks_t cbs = {
		.on_recv = NULL,
		.on_recv_q_ovf = NULL,
		.on_sent = NULL,
		.on_send_q_ovf = cb_i2s_tx_undeflow,
	};
	ESP_ERROR_CHECK(i2s_channel_register_event_callback(tx_chan, &cbs, NULL));

	// rtc_clk_apll_coeff_set(0, 0, 0, 9);  // Fastest possible APLL clock
	// I2S1.clkm_conf.clka_en = 1;  // enable APLL clock

	// This produces a 40 MHz bit-clock
	I2S1.clkm_conf.clk_en = 1;
	I2S1.clkm_conf.clkm_div_num = 2;  // integral divider, min is 2
	I2S1.clkm_conf.clkm_div_a = 1;  // fractional divider numerator
	I2S1.clkm_conf.clkm_div_b = 0;  // fractional divider denominator
	I2S1.sample_rate_conf.tx_bck_div_num = 2;  // bit-clock divider, min is 2
	// I2S1.sample_rate_conf.tx_bits_mod = 16;  // bit length of transmitter channel

	// Output an additional inverted WS on GPIO2
	// This is used as /CS1
	gpio_hal_iomux_func_sel(PIN_CS_N_B_NAME, PIN_FUNC_GPIO);
	gpio_set_direction(PIN_CS_N_B, GPIO_MODE_OUTPUT);
	esp_rom_gpio_connect_out_signal(PIN_CS_N_B, I2S1O_WS_OUT_IDX, true, 0);

	/* (Optional) Preload the data before enabling the TX channel, so that the valid data can be transmitted immediately */
	// while (w_bytes == EXAMPLE_BUFF_SIZE) {
	// 	/* Here we load the target buffer repeatedly, until all the DMA buffers are preloaded */
	// 	ESP_ERROR_CHECK(i2s_channel_preload_data(tx_chan, w_buf, EXAMPLE_BUFF_SIZE, &w_bytes));
	// }

	ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
}

void push_sample(uint16_t val_a, uint16_t val_b, uint16_t val_c, uint16_t val_d)
{
	static uint8_t chunk_buf[CHUNK_SIZE];  // buffer of one chunk of data
	static uint16_t *w_buf = (uint16_t *)chunk_buf;
	static unsigned n_written = 0;
	static unsigned n_underflows_ = 0;

	// print_dec_fix(val_a, FP, 2);
	// print_str(", ");
	// print_dec_fix(val_b, FP, 2);
	// print_str("\n");

	// output the sample-data for 4 channels over the next 64 clocks
	*w_buf++ = (0 << A_N) | (0 << GA_N) | (1 << SHDN_N) | val_a;  // DACA
	*w_buf++ = (0 << A_N) | (0 << GA_N) | (0 << SHDN_N) | val_c;  // DACB
	*w_buf++ = (1 << A_N) | (0 << GA_N) | (1 << SHDN_N) | val_b;  // DACA
	*w_buf++ = (1 << A_N) | (0 << GA_N) | (0 << SHDN_N) | val_d;  // DACB
	n_written += 4 * 2;

	if (n_written >= CHUNK_SIZE) {
		// Should block until CHUNK_SIZE bytes were written to DMA mem.
		esp_err_t ret = i2s_channel_write(tx_chan, chunk_buf, CHUNK_SIZE, NULL, portMAX_DELAY);
		if (ret != ESP_OK) {
			printf("Write Task: i2s write failed with %d\n", ret);
		}
		n_written = 0;
		w_buf = (uint16_t *)chunk_buf;

		if (n_underflows_ != n_underflows) {
			printf("n_underflows: %d\n", n_underflows);
			n_underflows_ = n_underflows;
		}
	}
	n_samples++;
}
