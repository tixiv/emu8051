
#include "print_number.h"
#include "keyboard.h"
#include "display.h"

#include <stdlib.h>
#include <string.h>

// "-12345\0"
// "-12.345\0"
// "123"
// "  0.123\0"
static char buffer[8];

void print_number(int16_t value, uint8_t dp) {
    memset(buffer, 0, 8);
    __itoa(value, buffer, 10);

    uint8_t o = 6;
    uint8_t i = 5;
    for (; i != 0xff; i--) {
        if (buffer[i] == '-')
            break;
        if (buffer[i] != 0) {
            buffer[o--]  = buffer[i];
            if (dp == o)
                buffer[o--] = '.';
        }
    }

    while (o > 0) {
        if (dp == o) {
            buffer[o--] = '.';
        }
        else if (o < dp - 1) {
            buffer[o--] = ' ';
        }
        else {
            buffer[o--] = '0';
        }
    }

    if (i == 0xff)
        buffer[0] = ' ';
    else
        buffer[0] = '-';


    display_print(buffer);
    display_put_char(' ');
}

float number_entry() {
    uint8_t pos = 0;

    display_set_cursor(0, 10);

    while (1) {
        update_keyboard();

        if (key_buffer) {
            if ((key_buffer & 0x30) && pos < 6) {
                buffer[pos++] = key_buffer;
                display_put_char(key_buffer);
            }
            if (key_buffer == 2 && pos > 0) {
                pos--;
                buffer[pos] = 0;
                display_set_cursor(0, 10 + pos);
                display_put_char(' ');
                display_set_cursor(0, 10 + pos);
            }
            if (key_buffer == 1) {
                key_buffer = 0;
                buffer[pos] = 0;
                return atof(buffer);
            }

            key_buffer = 0;
        }
    }
}
