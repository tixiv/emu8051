
#include "numeric_entry.h"
#include "keyboard.h"
#include "display.h"
#include <stdlib.h>

float numeric_entry_value;

static uint8_t pos = 0;
static char buffer[8];

static const uint8_t offset = 8;

void numeric_entry_init(void) {
    display_set_cursor(0, offset);
    display_put_char('?');

    pos = 0;
}

uint8_t numeric_entry_update(void) {
    if (key_buffer) {
        if ((key_buffer & 0x30) && pos < 6) {
            display_set_cursor(0, offset + pos);
            display_put_char(key_buffer);
            buffer[pos++] = key_buffer;
        }
        if (key_buffer == KEY_ESC) {
            if(pos > 0) {
                pos--;
                buffer[pos] = 0;
                display_set_cursor(0, offset + pos);
                display_put_char(' ');
            }
            else {
                return KEY_ESC;
            }
        }
        if (key_buffer == KEY_ENTER) {
            buffer[pos] = 0;
            numeric_entry_value = atof(buffer);
            return KEY_ENTER;
        }
    }
    return 0;
}
