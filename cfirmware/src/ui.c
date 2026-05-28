
#include "print_number.h"
#include "keyboard.h"
#include "display.h"
#include "general.h"

#include <string.h>
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
    display_string(0, "range: ");
}

void original_firmware(void) {
    IE   = 0;
	P1 = 0x65;
	((void (*)(void))0xfff0)(); // jump to transition asm code
    // DAT_EXTMEM(0x8003) = BIT_MOD(6,0);
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

void update_tests() {

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
