
#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <8051.h>



int putchar(int c) {
    display_put_char((char)c);
    return c;
}

#define DAT_EXTMEM(x) *((__xdata volatile uint8_t *)x)

#define TRACE(x) DAT_EXTMEM(0xffff)=(x)
// #define TRACE(x)

void io_init(void) {
    P1 = 0x65;
    DAT_EXTMEM(0X9003) = 0x88;
    DAT_EXTMEM(0X8003) = 0xb2;
    DAT_EXTMEM(0X9000) = 0xb7;
    DAT_EXTMEM(0x9002) = 8;
    DAT_EXTMEM(0X8003) = 8;
}

extern float get_float(void);



volatile uint8_t multimeter_state;

uint8_t multimeter_digits[5];

void int1_isr(void) __interrupt (2)
{
	uint8_t v = DAT_EXTMEM(0X8000);
	uint8_t s = multimeter_state;
	if (s == 0) {
		// sync with MSD D5, it will have 0 in the upper nibble
		if (v & 0xf0)
			return;

		s = 1;
	}

	if (s < 6) {
		multimeter_digits[s-1] = v;
		multimeter_state = s + 1;
	}
}

float read_multimeter_and_convert_result(void) {
	while(multimeter_state != 6);

	char buff[10];
	char *p = buff;

	if ((multimeter_digits[0] & 0x08) == 0)
		*p++ = '-';

	*p++ = (multimeter_digits[0] & 0x01) | 0x30;
	*p++ = (multimeter_digits[1] & 0x0f) | 0x30;
	*p++ = (multimeter_digits[2] & 0x0f) | 0x30;
	*p++ = (multimeter_digits[3] & 0x0f) | 0x30;
	*p++ = (multimeter_digits[4] & 0x0f) | 0x30;

	multimeter_state = 0;

	TRACE(1);

	float v = atoi(buff);

	TRACE(2);

	return v;
}

extern float soll;

extern void delay_16(uint16_t cycles);

void pulse_integrator(uint8_t cycles, uint8_t value) {
	TRACE(27);
	P1 = value;
	delay_16(cycles);
	P1 = 0x65;
	TRACE(28);
}

uint8_t break_cycle;

void pulse_integrator_big(float diff, uint8_t value){
	TRACE(29);
	uint16_t d = diff / 5.0f;
	P1 = value;
	delay_16(d);
	break_cycle = 2;
	P1 = 0x65;
	TRACE(30);
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

int main(void) {
    io_init();
    display_init();

	display_set_cursor(0,0);
	display_print("Hallo");

	DAT_EXTMEM(0X9000) = 0x5d | 0x80; // measure range 15V, keyb scan high

 	DAT_EXTMEM(0X8003) = 0x09; // Set PC4 = interrupt enable

	TCON = 0x00;  // level triggered INT1
	IE   = 0x84;   // enable INT1 + global

	while(1) {
		TRACE(0);
		float v = read_multimeter_and_convert_result();


		if (!break_cycle) {
			float diff = soll - v;
			do_integrator(diff);
		} else {
			break_cycle--;
		}

		char buffer[10];

		TRACE(3);
		int iv = v;
		TRACE(4);
	    __itoa(iv, buffer, 10);
        TRACE(5);

		display_set_cursor(0,0);
		display_print(buffer);
		display_print("      ");

		TRACE(6);
        v = v * 1.1f;
		TRACE(7);
		iv = v;
		TRACE(8);
	    __itoa((int)v, buffer, 10);
		TRACE(9);
        
		display_set_cursor(1,0);
		display_print(buffer);
		display_print("      ");
	}

	/*
	char buffer[10];

	sprintf(buffer, "A %d A", 1234);

	display_set_cursor(0,0);
	display_print(buffer);

	while(1){}
	*/
}
