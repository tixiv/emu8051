
#include "char_display.h"
#include "integrator.h"
#include "multimeter.h"
#include "plot.h"
#include "emu8051.h"
#include "curses.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    float control_voltage;
    float measure_voltage;

} io_board_t;

void io_board_tick (io_board_t* iob, float integrator_voltage, uint8_t range){
    float measure_mult = (range & 0x40) ? 10.0f : 1.0f;
    float control_mult = (range & 0x01) ? .02f : .2f;
    
    iob->control_voltage = integrator_voltage * control_mult;

    float mv = iob->control_voltage;
    switch ((~range) & 0x3C) {
        case 0: mv = iob->control_voltage; break;
        case 0x04: mv = 17.4 * 0.01; break; // temperature
        case 0x08: mv = 5.26 * 0.1; break; // battery
        case 0x10: mv = 0.0f; break; // GND
        case 0x20: mv = iob->control_voltage; break;// measure, not reverse engineered yet
    }

    iob->measure_voltage = mv * measure_mult;
}

struct calibrator_board_t {
    struct display_t display;
    multimeter_t multimeter;
    integrator_t integrator;
    io_board_t io_board;
    plot_t plot;
    
    uint8_t display_data;
    uint8_t display_ctrl;

    uint8_t range;

};

static struct calibrator_board_t the_board, *board = &the_board;

void calibrator_board_init () {
    display_init(&board->display);
    multimeter_init(&board->multimeter);
    plot_init(&board->plot);
    integrator_init(&board->integrator);
}


void logicboard_tick(struct em8051 *aCPU)
{
    uint8_t c = board->display_ctrl;
    uint8_t ctrl = ((c & 0x01) ? 0x80 : 0) | // EN
                   ((c & 0x02) ? 0x20 : 0) | // R/W
                   ((c & 0x04) ? 0x40 : 0);  // RS

    display_tick(&board->display, board->display_data, ctrl);

    integrator_tick(&board->integrator, aCPU);
    float akk = board->integrator.akk;

    io_board_tick(&board->io_board, akk, board->range);
    float measure_value = board->io_board.measure_voltage;
    //float measure_value = akk / 5.0f;

    double noise = ((double)rand() / RAND_MAX - 0.5) * 0.0005f;

    measure_value += noise;

    multimeter_tick(aCPU, &board->multimeter, measure_value);

    plot_update(&board->plot, measure_value);
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

    case 0x9000: return board->range;
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
        case 0x9000: board->range = value; break;
        case 0x9001: board->display_data = value; break;
        case 0x9002: board->display_ctrl = value; break;

    }
}

void calibrator_board_render (struct em8051 *aCPU)
{
    display_render(&board->display);

    mvprintw(10, 40, "XRAM 70 = %x", xram[0x70]);
    mvprintw(11, 40, "XRAM 71 = %x", xram[0x71]);
    mvprintw(12, 40, "XRAM A0 = %s                ", &xram[0xa0]);

    uint8_t d =board->multimeter.digits[0];

    char c1 = (d & 4) ? 'o' :
              (d & 2) ? 'u' : ' ';
    
    char c2 = (d & 8) ? ' ' : '-';

    mvprintw(13, 40, "multi = %c %c%c%c%c%c%c",
        c1, c2,
        board->multimeter.digits[0] & 0x31,
        board->multimeter.digits[1],
        board->multimeter.digits[2],
        board->multimeter.digits[3],
        board->multimeter.digits[4]);

    mvprintw(15, 40, "PORT1 = %02x", aCPU->mSFR[REG_P1]);
    mvprintw(16, 40, "inte = %g      ", board->integrator.akk);
    mvprintw(17, 40, "last = %02x", board->integrator.last_pulse_value);
    mvprintw(18, 40, "pc = %04x", board->integrator.written_from);

    mvprintw(18, 0, "range = %02x", board->range & 0x7f);

}
