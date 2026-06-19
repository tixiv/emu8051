

#include "multimeter.h"

#include "general.h"
#include <stdint.h>
#include <stdlib.h>
#include <8051.h>

float latest_measurement;

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


static char buff[7]; // "-12345\x00"

void read_multimeter_and_convert_result(void) {
	while(multimeter_state != 6);

	char *p = buff;

	if (multimeter_digits[0] & 0x04) { // overload
		if (multimeter_digits[0] & 0x08) {
			multimeter_state = 0; // can't put the reset before the if because it invalidates digits[0]
			latest_measurement = 2.22222f;
			return;
		}			
		else
		{
			multimeter_state = 0; // can't put the reset before the if because it invalidates digits[0]
			latest_measurement = -2.22222f;
			return;
		}
	}

	if ((multimeter_digits[0] & 0x08) == 0)
		*p++ = '-';

	*p++ = (multimeter_digits[0] & 0x01) | 0x30;
	*p++ = (multimeter_digits[1] & 0x0f) | 0x30;
	*p++ = (multimeter_digits[2] & 0x0f) | 0x30;
	*p++ = (multimeter_digits[3] & 0x0f) | 0x30;
	*p++ = (multimeter_digits[4] & 0x0f) | 0x30;
	*p = 0;

	multimeter_state = 0;

	float v = atoi(buff);
	
    latest_measurement = v * (1.1f / 10000.0f);
}
