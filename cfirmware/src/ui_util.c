
#include "ui_util.h"
#include "display.h"
#include <string.h>

__code const char *language_table = (__code char*)0x6000;

void display_indexed(uint8_t row, uint8_t idx) {
	display_set_cursor(row, 0);
	display_print_16(&language_table[idx * 16]);
}

void display_string(uint8_t row, __code const char *str) {
    if (str < (__code const char *)0x100) {
        display_indexed(row, (uint8_t)str);
    } else {
        display_set_cursor(row, 0);
        display_print(str);
        uint8_t len = strlen(str);
        while(len++ < 16)
            display_put_char(' ');
    }
}
