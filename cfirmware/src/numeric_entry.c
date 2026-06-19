
#include "numeric_entry.h"
#include "keyboard.h"
#include "display.h"
#include "ui.h"
#include <stdlib.h>

static uint8_t pos = 0;
static char buffer[8];

static const uint8_t offset = 8;

static numeric_entry_handler_t current_handler;

void numeric_entry_init(numeric_entry_handler_t handler) {
    current_handler = handler;
    current_screen = SCR_NUMERIC_ENTRY;
    display_set_cursor(0, offset);
    display_put_char('?');

    pos = 0;
}

void numeric_entry_update(void) {
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
                current_handler(0.0f, NES_CANCEL);
            }
        }
        if (key_buffer == KEY_ENTER) {
            buffer[pos] = 0;
            float value = atof(buffer);
            current_handler(value, NES_OKAY);
        }
    }
}
