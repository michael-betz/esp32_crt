# ESP32 CRT clock
## DACs
  * Use I2S peripheral to generate continuous sample stream
  * Drive 2x MCP4922 at 40 MHz, sharing the SCLK and DATA pins
  * Use the GPIO matrix to output an inverted copy of WS to serve as /CS for the second DAC
  * 4 channels, 12 bit, 625 kS/s

![logic-analyzer capture of the DAC SPI waveform](pics/ksnip_20231012-164406.png)

## SDL2 simulator
simulates the CRT. Used to develop the drawing algorithm.

![test-pattern on the CRT simulator](pics/ksnip_20231017-010520.png)
