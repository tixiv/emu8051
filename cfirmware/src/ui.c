
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

#include <string.h>
#include <stdio.h>
#include <8051.h>

uint8_t current_screen;
enum Screen {
    SCR_MAIN_MENU = 0,
    SCR_SUB_MENU  = 1,
    SCR_SOURCE = 2,
    SCR_TESTS  = 3,
    SCR_NUMERIC_ENTRY = 4,
};

typedef void(*handler_t)(void) ;

typedef struct {
    __code const char *name;
    handler_t handler;
} menu_entry_t;

typedef struct {
    __code const char *title;
    uint8_t num_entries;
    uint8_t idx;
    menu_entry_t entries[];
} menu_t;

void draw_menu(menu_t *menu) {
    display_string(0, menu->title);
    display_string(1, menu->entries[menu->idx].name);
}

uint8_t update_menu(menu_t *menu) {
    switch (key_buffer) {
        case KEY_MENU: {
            uint8_t idx = menu->idx + 1;
            if (idx >= menu->num_entries)
                idx = 0;
            menu->idx = idx;
            draw_menu(menu);
        } break;
        case KEY_ENTER: {
            menu->entries[menu->idx].handler();
        } break;
        case KEY_ESC: {
            return KEY_ESC;
        } break;
    }
    return 0;
}

menu_t *current_sub_menu;
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

menu_t sub_menu_volt_source = {
    INT_STRING(0x91), 3, 0, {
        {INT_STRING(0x9b), enter_volt_source_15V},
        {INT_STRING(0x9a), enter_volt_source_2V},
        {INT_STRING(0x99), enter_volt_source_200mV},
    }
};


void menu_entry_volt_source(void) {
    current_screen = SCR_SUB_MENU;
    current_sub_menu = &sub_menu_volt_source;
    draw_menu(current_sub_menu);
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


menu_t main_menu = {
    INT_STRING(0x8d), 4, 0, {
        {INT_STRING(0x67), menu_entry_volt_source},
        {INT_STRING(0x68), menu_entry_tc_source},
        {"Tests", menu_entry_tests},
        {"Original Firmw.", original_firmware}
    }
};

void return_to_main_menu(void) {
    current_screen = SCR_MAIN_MENU;
    draw_menu(&main_menu);
}

void return_to_sub_menu(void) {
    current_screen = SCR_SUB_MENU;
    draw_menu(current_sub_menu);
}

void update_ui(void) {
    update_keyboard();

    switch (current_screen) {
        case SCR_MAIN_MENU: {
            uint8_t res = update_menu(&main_menu);
            if (res) {}
        } break;
        case SCR_SUB_MENU: {
            uint8_t res = update_menu(current_sub_menu);
            if (res) {
                current_screen = SCR_MAIN_MENU;
                draw_menu(&main_menu);
            }
        } break;
        case SCR_TESTS: {
            uint8_t res = test_screen_update();
            if (res)
                return_to_main_menu();
        } break;
        case SCR_NUMERIC_ENTRY: {
            uint8_t res = numeric_entry_update();
            if (res == KEY_ESC)
                return_to_sub_menu();
            if (res == KEY_ENTER)
                numeric_entry_enter_pressed();
            
        } break;
        case SCR_SOURCE: {
             uint8_t res = update_source_screen();     
             if (res == KEY_ESC)
                return_to_sub_menu();       
        } break;
    }

    key_buffer = 0;
}

void init_ui(void) {
    draw_menu(&main_menu);
}
