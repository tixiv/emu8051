
#include "general.h"
#include "display.h"
#include "keyboard.h"
#include "multimeter.h"
#include "integrator.h"
#include "print_number.h"
#include "ui.h"
#include "measure_range.h"

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
    DAT_EXTMEM(0X9000) = 0x37; // keyscan upper low, measure range
    DAT_EXTMEM(0X8003) = BIT_MOD(4, 0); // disable multimeter interrupt
    DAT_EXTMEM(0x9002) = 0;    // keyscan lower low
}

float soll;

void source_loop(void) {
	while(1) {
		read_multimeter_and_convert_result();
    	float v = latest_measurement;
		if (!break_cycle) {
			float diff = soll - v;
			do_integrator(diff * 5.0f);
			display_set_cursor(1,0);
			print_number(diff * 10000.0f,0);
		} else {
			break_cycle--;
		}

		display_set_cursor(0,0);
		print_number(v * 10000.0f, 3);

		update_keyboard();

		if (key_buffer) {
			if (key_buffer == 3) {
				IE   = 0;
				P1 = 0x65;
				((void (*)(void))0xe0c8)(); // jump to original firmware start
			}
			else {
				key_buffer = 0;
				return;
			}
		}
	}
}

int main(void) {
    io_init();
    display_init();

	display_indexed(0, 0x00);
	display_indexed(1, 0x0b);

 	DAT_EXTMEM(0X8003) = 0x09; // Set PC4 = interrupt enable

	TCON = 0x00;  // level triggered INT1
	IE   = 0x84;   // enable INT1 + global

	integrator_calib();

	init_ui();

	set_measure_range(MR_15V);

	while (1) {
		read_multimeter_and_convert_result();
		update_ui();
	}

	while (1) {
		display_indexed(0, 0x38);
		display_indexed(1, 0x39);

		soll = number_entry() * 0.1f;

		read_multimeter_and_convert_result();
		read_multimeter_and_convert_result();

		source_loop();
	}

}
