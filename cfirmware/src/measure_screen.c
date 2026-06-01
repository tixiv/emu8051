
#include "measure_screen.h"
#include "keyboard.h"
#include "measure_range.h"
#include "ui_util.h"
#include "display.h"
#include "multimeter.h"
#include "print_number.h"
#include <stdint.h>

static measure_screen_t *ms;
static uint8_t range;

uint8_t measure_screen_update(void) {
    switch (key_buffer) {
        case KEY_ESC:
            return KEY_ESC;
        case '8':
            if (range > 0) {
                range--;
                set_measure_range(ms->ranges[range].range);
            }
            break;
        case '0':
            if (range < ms->num_ranges - 1) {
                range ++;
                set_measure_range(ms->ranges[range].range);
            }
            break;
    }

    display_indexed(0, ms->ranges[range].title);
    display_set_cursor(1,0);
    display_print("     ");

    float val = latest_measurement * 10000.0f;
    int8_t dp = ms->ranges[range].dp;
    if (dp < 0) {
        dp = -dp;
        val = -val;
    }

    print_number(val, dp);
    display_print("     ");

    return 0;
}

void measure_screen_set(measure_screen_t *measure_screen) {
    ms = measure_screen;
    range = 0;
}
