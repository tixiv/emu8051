
#include "char_display.h"
#include "integrator.h"
#include "multimeter.h"
#include "plot.h"
#include "emu8051.h"
#include "curses.h"

#include <stdbool.h>
#include <string.h>

struct calibrator_board_t {
    struct display_t display;
    multimeter_t multimeter;
    integrator_t integrator;
    plot_t plot;
    
    uint8_t display_data;
    uint8_t display_ctrl;
};

static struct calibrator_board_t the_board, *board = &the_board;

void calibrator_board_init () {
    display_init(&board->display);
    multimeter_init(&board->multimeter);
    plot_init(&board->plot);
    integrator_init(&board->integrator);
}

static float v = -0.01f;

void logicboard_tick(struct em8051 *aCPU)
{
    uint8_t c = board->display_ctrl;
    uint8_t ctrl = ((c & 0x01) ? 0x80 : 0) | // EN
                   ((c & 0x02) ? 0x20 : 0) | // R/W
                   ((c & 0x04) ? 0x40 : 0);  // RS

    display_tick(&board->display, board->display_data, ctrl);

    integrator_tick(&board->integrator, aCPU);

    float akk = board->integrator.akk;

    float measure_value = akk / 5.0f;

    multimeter_tick(aCPU, &board->multimeter, measure_value);

    plot_update(&board->plot, measure_value);

    if(1){
        static int delay;
        if ((delay++ & 0xfffff) == 0)
            v += 0.0001f;
    }
}

uint8_t xram[0x8000];

uint8_t calibrator_xread (struct em8051 *aCPU, uint16_t address)
{
    if (address < 0x8000) {
        return xram[address];
    }

    switch (address) {
    case 0x8000:
        aCPU->mSFR[REG_TCON] &= ~TCONMASK_IE1;
        return board->multimeter.dat_8000;

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

void calibrator_board_render (struct em8051 *aCPU)
{
    display_render(&board->display);

    mvprintw(10, 40, "XRAM 70 = %x", xram[0x70]);
    mvprintw(11, 40, "XRAM 71 = %x", xram[0x71]);
    mvprintw(12, 40, "XRAM A0 = %s", &xram[0xa0]);

    mvprintw(14, 40, "PORT1 = %x", aCPU->mSFR[REG_P1]);
    mvprintw(15, 40, "inte = %g", board->integrator.akk);
    mvprintw(16, 40, "last = %x", board->integrator.last_pulse_value);
    mvprintw(17, 40, "pc = %x", board->integrator.written_from);
}
