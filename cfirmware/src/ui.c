
#include "print_number.h"
#include "keyboard.h"
#include "display.h"
#include "general.h"
#include "measure_range.h"
#include "multimeter.h"

#include <string.h>
#include <stdio.h>
#include <8051.h>

__code const char *language_table = (__code char*)0x6000;


void display_indexed(uint8_t row, uint8_t idx) {
	display_set_cursor(row, 0);
	display_print_16(&language_table[idx * 16]);
}

void display_string(uint8_t row, __code const char *str) {
    if (str < (__code const char *)0x100) {
        display_indexed(row, (uint8_t)str);
    } else {
        display_set_cursor(row, 0);
        display_print(str);
        uint8_t len = strlen(str);
        while(len++ < 16)
            display_put_char(' ');
    }
}


uint8_t current_screen;
enum Screen {
    SCR_MAIN_MENU = 0,
    SCR_SUB_MENU  = 1,
    SCR_TESTS  = 2,
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

menu_t *current_sub_menu;

void menu_entry_volt(void) {

}

void menu_entry_tests(void) {
    current_screen = SCR_TESTS;
}

void original_firmware(void) {
    IE   = 0;
	((void (*)(void))0xffe8)(); // jump to transition asm code
}


menu_t main_menu = {
    (__code const char*)0x8d, 4, 0, {
        {(__code const char*)0x67, menu_entry_volt},
        {(__code const char*)0x68, menu_entry_volt},
        {"Tests", menu_entry_tests},
        {"Original Firmw.", original_firmware}
    }
};


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

void return_to_main_menu() {
    current_screen = SCR_MAIN_MENU;
    draw_menu(&main_menu);
}

struct  {
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

uint8_t range_idx;

void update_tests(void) {
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
            return_to_main_menu();
            return;
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
            update_tests();
        } break;
    }

    key_buffer = 0;
}

void init_ui(void) {
    draw_menu(&main_menu);
}
