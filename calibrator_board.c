
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
    // Ranges:
    //       M*10 Val GND Batt Temp  Sel Ctrl/10
    // 0x5f    1   0   1    1    1    1    1      -> Source 20mA
    // 0x1e    0   0   1    1    1    1    0      -> Source 52mA

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

    uint8_t reg_9000;
    uint8_t reg_9002;

    char new_key;
    char current_key;
    int key_timer;

    uint8_t keys_out;
};

static struct calibrator_board_t the_board, *board = &the_board;

void calibrator_board_init () {
    display_init(&board->display);
    multimeter_init(&board->multimeter);
    plot_init(&board->plot);
    integrator_init(&board->integrator);
}


void keyboard_update() {
    if (board->new_key > 0) {
        board->current_key = board->new_key;
        board->new_key = 0;
        board->key_timer = 100000;
    }

    if (board->key_timer) {
        board->key_timer--;
    }
    else
    {
        board->current_key = 0;
    }

    uint8_t keys_out = 0xff;

    if (board->current_key > 0) {
        if ((board->reg_9000 & 0x80) == 0) {
            switch (board->current_key) {
                case '7':   keys_out &= (uint8_t)~0x80; break;
                case '8':   keys_out &= (uint8_t)~0x20; break;
                case '9':   keys_out &= (uint8_t)~0x08; break;
                case '\n':  keys_out &= (uint8_t)~0x02; break;
                case '4':   keys_out &= (uint8_t)~0x40; break;
                case '5':   keys_out &= (uint8_t)~0x10; break;
                case '6':   keys_out &= (uint8_t)~0x04; break;
                case '/':   keys_out &= (uint8_t)~0x01; break;
            }
        }
        if ((board->reg_9002 & 0x08) == 0) {
            switch (board->current_key) {
                case '1':   keys_out &= (uint8_t)~0x80; break;
                case '2':   keys_out &= (uint8_t)~0x20; break;
                case '3':   keys_out &= (uint8_t)~0x08; break;
                case 'm':   keys_out &= (uint8_t)~0x02; break;
                case '#':   keys_out &= (uint8_t)~0x40; break;
                case '0':   keys_out &= (uint8_t)~0x10; break;
                case ',':   keys_out &= (uint8_t)~0x04; break;
                case '*':   keys_out &= (uint8_t)~0x01; break;
            }
        }
    }

    board->keys_out = keys_out;
}

void logicboard_tick(struct em8051 *aCPU)
{
    uint8_t c = board->reg_9002;
    uint8_t ctrl = ((c & 0x01) ? 0x80 : 0) | // EN
                   ((c & 0x02) ? 0x20 : 0) | // R/W
                   ((c & 0x04) ? 0x40 : 0);  // RS

    display_tick(&board->display, board->display_data, ctrl);

    integrator_tick(&board->integrator, aCPU);
    float akk = board->integrator.akk;

    io_board_tick(&board->io_board, akk, board->reg_9000);
    float measure_value = board->io_board.measure_voltage;
    //float measure_value = akk / 5.0f;

    double noise = ((double)rand() / RAND_MAX - 0.5) * 0.0002f;

    measure_value += noise;

    // The meter uses a 1.1V reference instead of 1V, so the numbers
    // are going to be off, but that's normal for this device.
    // Probably this decision is due to the 20mA range
    multimeter_tick(aCPU, &board->multimeter, measure_value / 1.1f);

    keyboard_update();

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
    
    case 0x8001: return board->keys_out;

    case 0x9000: return board->reg_9000;
    case 0x9002: return board->reg_9002;
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
        case 0x9000: board->reg_9000 = value; break;
        case 0x9001: board->display_data = value; break;
        case 0x9002: board->reg_9002 = value; break;
        case 0x9003:
            if ((value & 0x80) == 0) {
                // bit mod PORTC
                int bit_num = ((value & 0x0e) >> 1);

                if (value & 0x01)
                    board->reg_9002 |= (1 << bit_num);
                else
                    board->reg_9002 &= ~(1 << bit_num);
            }
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

    mvprintw(18, 0, "range = %02x", board->reg_9000 & 0x7f);

    mvprintw(19, 0, "key = '%c' = %02x  ", (board->current_key > 0 ? board->current_key : '^'), board->current_key);
    mvprintw(20, 0, "key_timer = '%d'  ", board->key_timer);
}

void logicboard_editor_keys(struct em8051 *aCPU, int ch) {
    board->new_key = ch;
}