
#include "ui_util.h"
#include "keyboard.h"
#include <stdint.h>

typedef void(*handler_t)(uint8_t) ;

typedef struct {
    __code const char *name;
    handler_t handler;
} menu_entry_t;

typedef struct {
    __code const char *title;
    uint8_t num_entries;
    menu_entry_t entries[];
} menu_t;

void menu_init(const menu_t *root_menu);
void enter_sub_menu(const menu_t *menu);
void pop_menu_stack(void);
void redraw_menu(void);

uint8_t update_menu(void);