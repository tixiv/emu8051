
#include "ui.h"
#include "print_number.h"
#include "keyboard.h"
#include "display.h"
#include "general.h"
#include "measure_range.h"
#include "multimeter.h"
#include "numeric_entry.h"
#include "ui_util.h"
#include "test_screen.h"
#include "source.h"
#include "menu.h"
#include "measure_screen.h"
#include "thermocouple.h"

#include <string.h>
#include <stdio.h>
#include <8051.h>

uint8_t current_mode;
uint8_t current_screen;

uint8_t ui_current_range;
uint8_t allow_measure_screen_exit;

void return_to_menu(void) {
    current_screen = SCR_MENU;
    redraw_menu();
}

typedef struct {
    uint8_t str_title;
    uint8_t str_entry_0;
    uint8_t str_entry_1;
    uint8_t range;
    float   multiplier;
    uint8_t dp;
} ui_range_t;

__code measure_screen_t measure_screen_mA = {
    2, {
        {0x70, -4, MR_2V_52mA},
        {0x6f, -3, MR_200mV_20mA},
    }
};

__code measure_screen_t measure_screen_V = {
    4, {
        {0x13, 4, MR_42V},
        {0x9b, 3, MR_15V},
        {0x9a, 2, MR_2V_52mA},
        {0x99, 4, MR_200mV_20mA},
    }
};

__code ui_range_t ranges[] = {
    {0xa5, 0x38, 0x39, MR_15V,        10.0f,  3},
    {0xa7, 0x3e, 0x3f, MR_2V_52mA,    1.0f,   2},
    {0xa9, 0x44, 0x45, MR_200mV_20mA, 100.0f, 4},
    {0x74, 0x26, 0x27, MR_2V_52mA,    100.0f, 4},
    {0x76, 0x2c, 0x2d, MR_200mV_20mA, 10.0f,  3},
};

float val_to_hw;

static void exit_source(void) {
    set_measure_range(MR_Int_x0_2);
    source_target = 0.0f;
}


static void numeric_entry_done(float value, uint8_t status) __reentrant {
    if (status == NES_OKAY) {
        set_measure_range(ranges[ui_current_range].range);
        source_target = value * val_to_hw;
        source_start();
        current_screen = SCR_SOURCE;
    }
    else {
        exit_source();
        return_to_menu();
    }
}

static void enter_source_numeric_entry(void) {
    display_indexed(0, ranges[ui_current_range].str_entry_0);
    display_indexed(1, ranges[ui_current_range].str_entry_1);
    numeric_entry_init(numeric_entry_done);
}

void enter_source(void) {
    val_to_hw = 1.0f / ranges[ui_current_range].multiplier;

    enter_source_numeric_entry();
}

uint8_t update_source_screen(void) {
    switch (key_buffer) {
        case KEY_ESC :
            exit_source();
            return KEY_ESC;
        case 0: break;
    
        default:
            enter_source_numeric_entry();
            return 0;
    }
    
    display_indexed(0, ranges[ui_current_range].str_title);

    display_set_cursor(1, 0);
    display_print(" D   ");
    print_number(latest_measurement * 10000.0f, ranges[ui_current_range].dp);
    display_print("     ");

    return 0;
}

void enter_volt_source(uint8_t entry) {
    ui_current_range = entry;
    enter_source();
}

void enter_current_source(uint8_t entry) {
    ui_current_range = entry + 3;
    enter_source();
}

__code menu_t sub_menu_volt_source = {
    INT_STRING(0x91), 3, {
        {INT_STRING(0x9b), enter_volt_source},
        {INT_STRING(0x9a), enter_volt_source},
        {INT_STRING(0x99), enter_volt_source},
    }
};

void menu_entry_volt_source(uint8_t entry) {
    (void) entry;
    enter_sub_menu(&sub_menu_volt_source);
}

void menu_entry_volt_measure(uint8_t entry) {
    (void) entry;
    measure_screen_set(&measure_screen_V);
    allow_measure_screen_exit = 1;
    current_screen = SCR_MEASURE;
}

void menu_entry_test(uint8_t entry) {
    switch (entry) {
        case 0:
            current_screen = SCR_TESTS;
            display_clear_row(0);
            display_clear_row(1);
            break;
        case 1: 
            current_screen = SCR_MULTIMETER_TEST;
            multimeter_test_init();
        break;
    }
}

__code menu_t sub_menu_tests = {
    "Tests", 2, {
        {"Range/Integrator", menu_entry_test},
        {"Meter timing", menu_entry_test},
    }
};


void menu_entry_tests(uint8_t entry) {
    (void) entry;
    enter_sub_menu(&sub_menu_tests);
}

void original_firmware(uint8_t entry) {
    (void) entry;
    IE   = 0;
	((void (*)(void))0xffe8)(); // jump to transition asm code
}

__code const char str_Tests[] = "Tests";
__code const char str_OrigFimrware[] = "Original Firmw.";

__code menu_t main_menu_volt_source = {
    INT_STRING(0x8d), 4, {
        {INT_STRING(0x67), menu_entry_volt_source},
        {INT_STRING(0x68), menu_entry_tc_source},
        {str_Tests, menu_entry_tests},
        {str_OrigFimrware, original_firmware}
    }
};

__code menu_t main_menu_current_source = {
    INT_STRING(0x69), 4, {
        {INT_STRING(0x70), enter_current_source},
        {INT_STRING(0x6f), enter_current_source},
        {str_Tests, menu_entry_tests},
        {str_OrigFimrware, original_firmware}
    }
};

__code menu_t main_menu_volt_measure = {
    INT_STRING(0xe2), 4, {
        {INT_STRING(0x67), menu_entry_volt_measure},
        {INT_STRING(0x68), menu_entry_tc_measure},
        {str_Tests, menu_entry_tests},
        {str_OrigFimrware, original_firmware}
    }
};

uint8_t read_switch_posistion(void) {
    return DAT_EXTMEM(0x9002) >> 4;
}

void update_ui(void) {
    if (current_mode != read_switch_posistion()) {
        source_stop();
        init_ui();
    }
    update_keyboard();

    switch (current_screen) {
        case SCR_MENU: {
            update_menu();
        } break;
        case SCR_TESTS: {
            uint8_t res = test_screen_update();
            if (res)
                return_to_menu();
        } break;
        case SCR_MULTIMETER_TEST: {
            uint8_t res = multimeter_test_screen_update();
            if (res)
                return_to_menu();
        } break;
        case SCR_NUMERIC_ENTRY: {
            numeric_entry_update();            
        } break;
        case SCR_SOURCE: {
             uint8_t res = update_source_screen();     
             if (res == KEY_ESC)
                return_to_menu();
        } break;
        case SCR_MEASURE: {
            uint8_t res = measure_screen_update();
            if (res && allow_measure_screen_exit)
                return_to_menu();
        } break;
        case SCR_MEASURE_TC: {
            uint8_t res = measure_tc_screen_update();
            if (res)
                return_to_menu();
        } break;
    }

    key_buffer = 0;
}

void init_ui(void) {

    current_mode = read_switch_posistion();
    switch (current_mode) {
        default:
        case 0:
            menu_init(&main_menu_current_source);
            current_screen = SCR_MENU;
            break;
        case 1:
            menu_init(&main_menu_volt_source);
            current_screen = SCR_MENU;
            break;
        case 4:
            measure_screen_set(&measure_screen_mA);
            allow_measure_screen_exit = 0;
            current_screen = SCR_MEASURE;
            break;
        case 5:
            menu_init(&main_menu_volt_measure);
            current_screen = SCR_MENU;
            break;
    }
}
