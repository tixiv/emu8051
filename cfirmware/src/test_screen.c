
#include "test_screen.h"
#include "measure_range.h"
#include "keyboard.h"
#include "general.h"
#include "display.h"
#include "print_number.h"
#include "multimeter.h"
#include "ui_util.h"
#include <8051.h>

static struct  {
    uint8_t range;
    __code const char *name;    
} __code ranges[] = {
    { MR_Batt,       "Range: Battery" },
    { MR_Temp,       "Range: Temperature" },
    { MR_15V,        "Range: 15V" },
    { MR_2V_52mA,    "Range: 2V / 52mA" },
    { MR_200mV_20mA, "Range:200mV/20mA" },
    { MR_Int_x0_2,   "Range: Int * 0.2" },
    { MR_Int_x2,     "Range: Int * 2.0" },
    { MR_Int_x6,     "Range: Int * 6.0" },
    { MR_GND,        "Range: GND" },
    { MR_GND_x10,    "Range: GND * 10" },
    { MR_GND_x30,    "Range: GND * 30" },
};

static uint8_t range_idx;

uint8_t test_screen_update(void) {
    switch (key_buffer) {
        case KEY_MENU:
        case '0':
            range_idx++;
            if (range_idx >= ARRAY_SIZE(ranges))
                range_idx = 0;
            break;
        case '8':
            range_idx--;
            if (range_idx >= ARRAY_SIZE(ranges))
                range_idx = ARRAY_SIZE(ranges) - 1;
            break;
        case KEY_ESC:
            return KEY_ESC;
    }

    set_measure_range(ranges[range_idx].range);

    switch (current_key) {
        case '7': P1 = 0xb7; break;
        case '9': P1 = 0xbb; break;
        case '4': P1 = 0xc7; break;
        case '6': P1 = 0xcb; break;
        case '1': P1 = 0xa7; break;
        case '3': P1 = 0xab; break;
        case '-': P1 = 0x67; break;
        case '.': P1 = 0x6b; break;
        default: P1 = 0x65; break;
    }

    display_set_cursor(1, 0);
    print_number (latest_measurement * 10000.0f, 2);
    display_string(0, ranges[range_idx].name);

    return 0;
}
