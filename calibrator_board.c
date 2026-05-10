
#include "char_display.h"
#include "emu8051.h"
#include <stdbool.h>



struct calibrator_board_t {
    struct display_t display;
    
    uint8_t display_data;
    uint8_t display_ctrl;

};

static struct calibrator_board_t the_board, *board = &the_board;

void logicboard_tick(struct em8051 *aCPU)
{
    static bool initialized;
    if(!initialized) {
        initialized = true;
        display_init(&board->display);
    }

    uint8_t c = board->display_ctrl;
    uint8_t ctrl = ((c & 0x01) ? 0x80 : 0) | // EN
                   ((c & 0x02) ? 0x20 : 0) | // R/W
                   ((c & 0x04) ? 0x40 : 0);  // RS

    display_tick(&board->display, board->display_data, ctrl);
}

void calibrator_board_render (struct em8051 *aCPU)
{
    display_render(&board->display);
}

uint8_t xram[0x8000];

uint8_t calibrator_xread (struct em8051 *aCPU, uint16_t address)
{
    if (address < 0x8000) {
        return xram[address];
    }

    switch (address) {
        case 0x9002: return board->display_ctrl;
    }

    return 0xff;
}

void calibrator_xwrite (struct em8051 *aCPU, uint16_t address, uint8_t value)
{
    if (address < 0x8000) {
        xram[address] = value;
        return;
    }

        switch (address) {
        case 0x9001: board->display_data = value; break;
        case 0x9002: board->display_ctrl = value; break;
    }
}