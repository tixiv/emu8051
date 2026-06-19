
#include "test_screen.h"
#include "measure_range.h"
#include "keyboard.h"
#include "general.h"
#include "display.h"
#include "print_number.h"
#include "multimeter.h"
#include "ui_util.h"
#include "source.h"
#include "integrator.h"
#include "display.h"
#include <8051.h>

static struct  {
    uint8_t range;
    __code const char *name;    
} __code ranges[] = {
    { MR_Batt,          "Range: Battery" },
    { MR_Temp,          "Range: Temperatu" },
    { MR_42V,           "Range: 42V" },
    { MR_15V,           "Range: 15V" },
    { MR_2V_52mA,       "Range: 2V / 52mA" },
    { MR_200mV_20mA,    "Range:200mV/20mA" },
    { MR_42V | MR_x3,   "Range: 42V * 3" },
    { MR_15V | MR_x3,   "Range: 15V * 3" },
    { MR_2V_52mA|MR_x3, "Range: 2V/52mA*3" },
    { MR_66mV_6_6mA,    "Range: 66mV/6mA" },
    { MR_GND,           "Range: GND" },
    { MR_GND | MR_x3,   "Range: GND * 3" },
    { MR_GND_x10,       "Range: GND * 10" },
    { MR_GND_x30,       "Range: GND * 30" },
    { MR_Int_x0_2,      "Range: Int * 0.2" },
    { MR_Int_x2,        "Range: Int * 2.0" },
    { MR_Int_x6,        "Range: Int * 6.0" },
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

enum MultimeterTestState {
    MTS_INIT,
    MTS_RUNNING,

};

static uint8_t multimeter_test_state;
static uint16_t multimeter_test_delay;
static uint8_t toggle;

void multimeter_test_init(void) {
    multimeter_test_state = MTS_INIT;
    multimeter_test_delay = 10;
    set_measure_range(MR_Int_x0_2);
    source_target = 2.0f;
    source_start();

    display_clear_row(0);
    display_clear_row(1);
}

uint8_t multimeter_test_screen_update(void)
{
    if (multimeter_test_state == MTS_RUNNING) {
        pulse_integrator_exact(multimeter_test_delay, 0x65);
        if (toggle) {
            set_measure_range(MR_Int_x0_2);
        } else {
            set_measure_range(MR_GND);
        }
    } else {
        if (multimeter_test_delay) {
            multimeter_test_delay--;
            return 0;
        }
        else {
            multimeter_test_state = MTS_RUNNING;
            source_stop();
            toggle = 0;
            return 0;
        }
    }
    
    switch (key_buffer) {
        case KEY_ESC:
            return KEY_ESC;
    }

    switch (current_key) {
        case '7': if (multimeter_test_delay >= 1000) multimeter_test_delay -= 1000; break;
        case '9': if (multimeter_test_delay <= (65535 -1000)) multimeter_test_delay += 1000; break;
        case '4': if (multimeter_test_delay >= 100) multimeter_test_delay -= 100; break;
        case '6': if (multimeter_test_delay <= (65535 -100)) multimeter_test_delay += 100; break;
        case '1': if (multimeter_test_delay >= 10) multimeter_test_delay -= 10; break;
        case '3': if (multimeter_test_delay <= (65535 -10)) multimeter_test_delay += 10; break;
        case '-': if (multimeter_test_delay >= 1) multimeter_test_delay -= 1; break;
        case '.': if (multimeter_test_delay <= (65535 -1)) multimeter_test_delay += 1; break;
        default: break;
    }

    if (toggle) {
        display_set_cursor(0, 8);
    } else {
        display_set_cursor(0, 0);
    }
    print_number(latest_measurement * 10000.0f, 3);

    display_set_cursor(1, 0);

    print_number(multimeter_test_delay, 8);

    toggle ^= 1;

    return 0;
}