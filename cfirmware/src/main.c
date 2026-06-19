
#include "general.h"
#include "display.h"
#include "keyboard.h"
#include "multimeter.h"
#include "integrator.h"
#include "print_number.h"
#include "ui.h"
#include "measure_range.h"
#include "source.h"

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

int main(void) {
    io_init();
    display_init();

	display_indexed(0, 0x00);
	display_indexed(1, 0x0b);

 	DAT_EXTMEM(0X8003) = 0x09; // Set PC4 = interrupt enable

	TCON = 0x00;  // level triggered INT1
	IE   = 0x84;   // enable INT1 + global

	do_integrator_calibration();

	set_measure_range(MR_Int_x0_2);

	init_ui();

	while (1) {
		TRACE(1);
		read_multimeter_and_convert_result();
		TRACE(2);
		source_update();
		TRACE(3);
		update_ui();
	}
}
