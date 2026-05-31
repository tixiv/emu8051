
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
