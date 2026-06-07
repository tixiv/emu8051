
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

void print_number_cropped(int16_t value, uint8_t dp, uint8_t crop, uint8_t minus) {
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

    while (o > crop) {
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
        buffer[crop] = ' ';
    else
        buffer[crop] = '-';


    display_print(buffer + crop + (minus ? 0:1));
}

void print_number(int16_t value, uint8_t dp) {
    print_number_cropped(value, dp, 0, 1);
}
