/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hub75.pio.h"

//#include "mountains_128x64_rgb565.h"

#define DATA_BASE_PIN 0
#define DATA_N_PINS 6
#define ROWSEL_BASE_PIN 6
#define ROWSEL_N_PINS 4
#define CLK_PIN 10
#define STROBE_PIN 11
#define OEN_PIN 12

#define WIDTH 64
#define HEIGHT 32

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 17
#define UART_RX_PIN 16

int main() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	// overclocking makes the clock too fast for the screens, 
	// so it'd require additional delays
    stdio_uart_init();

    PIO pio = pio0;
    uint sm_data = 0;
    uint sm_row = 1;

    uint data_prog_offs = pio_add_program(pio, &hub75_data_rgb888_program);
    uint row_prog_offs = pio_add_program(pio, &hub75_row_program);
    hub75_data_rgb888_program_init(pio, sm_data, data_prog_offs, DATA_BASE_PIN, CLK_PIN);
    hub75_row_program_init(pio, sm_row, row_prog_offs, ROWSEL_BASE_PIN, ROWSEL_N_PINS, STROBE_PIN);

    //static uint32_t gc_plane[WIDTH*HEIGHT/8]; // 4 pix per entry; for DMA later
	//though it'd mean an interrupt every <3.2ms

    while (1) {
		absolute_time_t start = get_absolute_time();
		for (int bit = 0; bit < 8; ++bit) {
			for (int rowsel = 0; rowsel < (1 << ROWSEL_N_PINS); ++rowsel) {
				for (int x = 0; x < WIDTH; x+=4) {

					uint32_t color0 = 0b111111;
					uint32_t color1 = 0b100100;
					uint32_t color2 = 0b010010;
					uint32_t color3 = 0b001001;
					uint32_t pix =  (((x+0)*4) <<  0) | // RGBRGB ((x+0, y), (x+0, y+16))
									(((x+1)*4) <<  6) | // RGBRGB ((x+1, y), (x+1, y+16))
									(((x+2)*4) << 12) | // RGBRGB ((x+2, y), (x+2, y+16))
									(((x+3)*4) << 18);  // RGBRGB ((x+3, y), (x+3, y+16))
													    // 8 bit padding, bits 24-32
					// BGRBGRBGRBGRBGRBGR
					pix = 0b001001010010100100111111 << 0;
					pio_sm_put_blocking(pio, sm_data, pix);
				}
                // SM is finished when it stalls on empty TX FIFO
                hub75_wait_tx_stall(pio, sm_data);
                // Also check that previous OEn pulse is finished, else things can get out of sequence
                hub75_wait_tx_stall(pio, sm_row);

                // Latch row data, pulse output enable for new row.
				// this brings latch down and OE up for 2^n cycles --
				// does the BCM in PIO but processor is busy feeding it
				// also sets brightness per row; 100 =~ 100us; 10 =~10us
                pio_sm_put_blocking(pio, sm_row, rowsel | (10u * (1u << bit) << 5));
            }
        }

		absolute_time_t end = get_absolute_time();
		int64_t delta = absolute_time_diff_us(start, end);
		printf("Time per frame: %lldμs\n", delta);
		/*
		Original example:
		Time per frame: 3757μs

		Pre-shuffled-bits:
		Time per frame: 3240μs
		*/
    }

}
