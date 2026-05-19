
#include "general.h"
#include <stdint.h>

char current_key;
char key_buffer;

static char last_key;

__code const char key_table[] = {
    '7', '4', '8', '5', '9', '6', 1, 2,
    '1', '-', '2', '0', '3', '.', 3, 4
};

static uint8_t get_index_of_clear_bit(uint8_t d) {
    uint8_t i = 0;
    for(;;) {
        if ((d & 0x80) == 0)
            return i;
        d <<= 1;
        i++;
    }
}

void update_keyboard(void) {
    
    // reads both key rows
    uint8_t keys = DAT_EXTMEM(0x8001);

    if (keys != 0xff) // key pressed
    {
        // PC3 = kescan lower high for a short check
        DAT_EXTMEM(0x9003) = BIT_MOD(3,1);
        // Here we will only see the keys in the first row because the second one is high now
        uint8_t keys_1 = DAT_EXTMEM(0x8001);
        DAT_EXTMEM(0x9003) = BIT_MOD(3,0);

        uint8_t key_idx;

        if (keys_1 != 0xff) {
            key_idx = get_index_of_clear_bit(keys_1);
        }
        else {
            key_idx = 8 + get_index_of_clear_bit(keys);
        }

        current_key = key_table[key_idx];
    } else {
        current_key = 0;
    }

    if (current_key != last_key) {
        if (current_key) {
            key_buffer = current_key;
        }
    }

    last_key = current_key;
}