
#include "general.h"
#include "display.h"
#include "keyboard.h"
#include "multimeter.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <8051.h>


int putchar(int c) {
    display_put_char((char)c);
    return c;
}

void io_init(void) {
    P1 = 0x65;
    DAT_EXTMEM(0X9003) = 0x88;
    DAT_EXTMEM(0X8003) = 0xb2;
    DAT_EXTMEM(0X9000) = 0x37; // keyscan upper low, measure range
    DAT_EXTMEM(0X8003) = 8;    // disable multimeter ineterrupt
    DAT_EXTMEM(0x9002) = 0;    // keyscan lower low
}

#define TRACE_MAIN(x) TRACE(x)

extern void delay_16(uint16_t cycles);

void pulse_integrator(uint8_t cycles, uint8_t value) {
	TRACE_MAIN(27);
	P1 = value;
	delay_16(cycles);
	P1 = 0x65;
	TRACE_MAIN(28);
}

uint8_t break_cycle;

void pulse_integrator_big(float diff, uint8_t value){
	TRACE_MAIN(29);
	uint16_t d = diff / 5.0f;
	P1 = value;
	delay_16(d);
	break_cycle = 2;
	P1 = 0x65;
	TRACE_MAIN(30);
}

void do_integrator(float diff) {
	diff *= 8;
	uint8_t negative = 0;
	if (diff < 0.0f) {
		diff = -diff;
		negative = 1;
	}

	if (diff < 255.0f) {
		volatile uint8_t cycles = diff;
		if (negative)
			pulse_integrator(cycles, 0xe6);
		else
			pulse_integrator(cycles, 0xea);
	} else {
		if (negative)
			pulse_integrator_big(diff, 0xf6);
		else
			pulse_integrator_big(diff, 0xfa);
	}

}

__code const char *language_table = (__code char*)0x6000;

void display_i_string(uint8_t row, uint8_t idx) {
	display_set_cursor(row, 0);
	display_print_16(&language_table[idx * 16]);
}

void pulse_integrator_test(uint16_t cycles);

void integrator_calib(void);

uint8_t cycles = 100;

int main(void) {
    io_init();
    display_init();

	display_i_string(0, 0x00);
	display_i_string(1, 0x0b);

	DAT_EXTMEM(0X9000) = 0x5d; // measure range 15V, keyb scan low

 	DAT_EXTMEM(0X8003) = 0x09; // Set PC4 = interrupt enable

	TCON = 0x00;  // level triggered INT1
	IE   = 0x84;   // enable INT1 + global



	// integrator_calib();

	// while(1);


	while(1) {
		TRACE_MAIN(0);
		float v = read_multimeter_and_convert_result();


		/*
		if (!break_cycle) {
			float diff = soll - v;
			do_integrator(diff);
		} else {
			break_cycle--;
		}

		*/


		if (cycles) {
			TRACE(100 + cycles);
			pulse_integrator_test(cycles);
			cycles --;
		}

		char buffer[10];

		TRACE_MAIN(3);
		int iv = v;
		TRACE(4);
	    __itoa(iv, buffer, 10);
        TRACE_MAIN(5);

		display_set_cursor(0,0);
		display_print(buffer);
		display_print("      ");

		TRACE_MAIN(6);
        v = v * 1.1f;
		TRACE_MAIN(7);
		iv = v;
		TRACE_MAIN(8);
	    __itoa((int)v, buffer, 10);
		TRACE_MAIN(9);
        
		display_set_cursor(1,0);
		display_print(buffer);
		display_print("      ");

		update_keyboard();

		if (key_buffer) {
			if (key_buffer == 1) {
				// Haha, this crashes sdcc: '((void *(void))0x1234)();'

				IE   = 0;
				P1 = 0x65;
				((void (*)(void))0xe0c8)(); // jump to original firmware start
			}
			
			display_set_cursor(1,15);
			display_put_char(key_buffer);
			key_buffer = 0;

		}


	}
}
