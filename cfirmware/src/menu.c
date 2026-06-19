
#include "menu.h"

typedef struct {
    const menu_t * menu;
    uint8_t index;
} menu_stack_t;

static menu_stack_t menu_stack[4];
static uint8_t menu_sp;

static const menu_t *current_menu;
static uint8_t current_index;

void redraw_menu(void) {
    display_string(0, current_menu->title);
    display_string(1, current_menu->entries[current_index].name);
}

void menu_init(const menu_t *root_menu) {
    current_menu = root_menu;
    current_index = 0;
    menu_sp = 0;
    redraw_menu();
}

void enter_sub_menu(const menu_t *menu) {
    menu_stack[menu_sp].menu = current_menu;
    menu_stack[menu_sp].index = current_index;
    menu_sp++;
    current_menu = menu;
    current_index = 0;
    redraw_menu();
}

void pop_menu_stack(void) {
    if (menu_sp) {
        menu_sp--;
        current_menu = menu_stack[menu_sp].menu;
        current_index = menu_stack[menu_sp].index;
    }
}

static void exit_sub_menu(void) {
    pop_menu_stack();
    redraw_menu();
}

uint8_t update_menu(void) {
    switch (key_buffer) {
        case KEY_MENU: {
            uint8_t idx = current_index + 1;
            if (idx >= current_menu->num_entries)
                idx = 0;
            current_index = idx;
            redraw_menu();
        } break;
        case KEY_ENTER: {
            current_menu->entries[current_index].handler(current_index);
        } break;
        case KEY_ESC: {
            exit_sub_menu();
            return KEY_ESC;
        } break;
    }
    return 0;
}
