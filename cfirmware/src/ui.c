
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

#include <string.h>
#include <stdio.h>
#include <8051.h>

uint8_t current_screen;
enum Screen {
    SCR_MENU = 0,
    SCR_SOURCE = 1,
    SCR_TESTS  = 2,
    SCR_NUMERIC_ENTRY = 3,
    SCR_MEASURE = 4,
};

uint8_t ui_current_range;

typedef struct {
    uint8_t str_title;
    uint8_t str_entry_0;
    uint8_t str_entry_1;
    uint8_t range;
    float   multiplier;
    uint8_t dp;
} ui_range_t;

__code ui_range_t ranges[] = {
    {0xa5, 0x38, 0x39, MR_15V,        10.0f,  3},
    {0xa7, 0x3e, 0x3f, MR_2V_52mA,    1.0f,   2},
    {0xa9, 0x44, 0x45, MR_200mV_20mA, 100.0f, 4},
    {0x74, 0x26, 0x27, MR_2V_52mA,    100.0f, 4},
    {0x76, 0x2c, 0x2d, MR_200mV_20mA, 10.0f,  3},
};

void enter_source_numeric_entry(void) {
    display_indexed(0, ranges[ui_current_range].str_entry_0);
    display_indexed(1, ranges[ui_current_range].str_entry_1);
    current_screen = SCR_NUMERIC_ENTRY;
    numeric_entry_init();
}

float val_to_hw;

void enter_source(void) {
    val_to_hw = 1.0f / ranges[ui_current_range].multiplier;

    enter_source_numeric_entry();
}

void numeric_entry_enter_pressed(void) {
    set_measure_range(ranges[ui_current_range].range);
    source_target = numeric_entry_value * val_to_hw;
    source_start();
    current_screen = SCR_SOURCE;
}

uint8_t update_source_screen(void) {
    switch (key_buffer) {
        case KEY_ESC :
            set_measure_range(MR_Int_x0_2);
            source_target = 0.0f;
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

void enter_volt_source_15V(void) {
    ui_current_range = 0;
    enter_source();
}

void enter_volt_source_2V(void) {
    ui_current_range = 1;
    enter_source();
}

void enter_volt_source_200mV(void) {
    ui_current_range = 2;
    enter_source();
}

void enter_current_source_52ma(void) {
    ui_current_range = 3;
    enter_source();
}

void enter_current_source_20ma(void) {
    ui_current_range = 4;
    enter_source();
}

__code menu_t sub_menu_volt_source = {
    INT_STRING(0x91), 3, {
        {INT_STRING(0x9b), enter_volt_source_15V},
        {INT_STRING(0x9a), enter_volt_source_2V},
        {INT_STRING(0x99), enter_volt_source_200mV},
    }
};

void menu_entry_volt_source(void) {
    enter_sub_menu(&sub_menu_volt_source);
}

void menu_entry_tc_source(void) {

}

void menu_entry_tests(void) {
    current_screen = SCR_TESTS;
}

void original_firmware(void) {
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
        {INT_STRING(0x70), enter_current_source_52ma},
        {INT_STRING(0x6f), enter_current_source_20ma},
        {str_Tests, menu_entry_tests},
        {str_OrigFimrware, original_firmware}
    }
};

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

void return_to_menu(void) {
    current_screen = SCR_MENU;
    redraw_menu();
}

void update_ui(void) {
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
        case SCR_NUMERIC_ENTRY: {
            uint8_t res = numeric_entry_update();
            if (res == KEY_ESC)
                return_to_menu();
            if (res == KEY_ENTER)
                numeric_entry_enter_pressed();
            
        } break;
        case SCR_SOURCE: {
             uint8_t res = update_source_screen();     
             if (res == KEY_ESC)
                return_to_menu();       
        } break;
        case SCR_MEASURE: {
            measure_screen_update();     
        } break;
    }

    key_buffer = 0;
}

uint8_t switch_posistion;

void update_switch_posistion(void) {
    switch_posistion = DAT_EXTMEM(0x9002) >> 4;
}

void init_ui(void) {
    update_switch_posistion();
    switch (switch_posistion) {
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
            current_screen = SCR_MEASURE;
            break;
        case 5:
            measure_screen_set(&measure_screen_V);
            current_screen = SCR_MEASURE;
            break;
    }
}
