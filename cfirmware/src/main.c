
#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include <8051.h>



int putchar(int c) {
    display_put_char((char)c);
    return c;
}

#define DAT_EXTMEM(x) *((__xdata volatile uint8_t *)x)

void io_init(void) {
    // P1 = 0x65;
    DAT_EXTMEM(0X9003) = 0x88;
    DAT_EXTMEM(0X8003) = 0xb2;
    DAT_EXTMEM(0X9000) = 0xb7;
    DAT_EXTMEM(0x9002) = 8;
    DAT_EXTMEM(0X8003) = 8;
}

extern float get_float(void);


void test1(){
	float x = get_float();

    for(int i=0; i<2; i++) {

		int my_number = x;
    	char buffer[10];

	    __itoa(my_number, buffer, 10);
        
		display_set_cursor(i,0);
		display_print(buffer);

        x = x * 1.1f;

    }
}

int main(void) {
    io_init();
    display_init();

	test1();

	while(1);

	char buffer[10];

	sprintf(buffer, "A %d A", 1234);

	display_set_cursor(0,0);
	display_print(buffer);

	while(1){}
}
