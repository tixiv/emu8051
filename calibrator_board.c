
#include "char_display.h"
#include "emu8051.h"
#include "curses.h"

#include <stdbool.h>
#include <string.h>


typedef struct {
    int measure_cycle_delay;
    int data_cycle_delay;
    int data_count;
    int cycle;
    uint8_t dat_8000;

    uint8_t digits[5];
} multimeter_t;

// uint8_t digits[] = "81111" "02222" "83333" "04444" "85555" "06666" "87777" "08888";

void multimeter_init(multimeter_t *meter) {
    memset(meter, 0, sizeof(multimeter_t));
    meter->data_count = -1;
}

void multimeter_tick(struct em8051 *aCPU, multimeter_t *meter, float value) {
    if (meter->measure_cycle_delay == 0)
    {
        {
            int v = (value * 10000.0f) + 0.5f;

            bool plus = value > 0.0f;
            bool ov = false;
            if (v > 19999) { ov = true; v = 19999; }
            if (v < -19999) { ov = true; v = -19999; }

            if (v < 0) v = -v;

            bool ur = v <= 1800; 


            meter->digits[0] = 0x30 | (v >= 10000 ? 1:0) | (plus ? 8:0) | (ov ? 4:0) | (ur ? 2:0);
            meter->digits[1] = 0x30 + (v / 1000) % 10;
            meter->digits[2] = 0x30 + (v / 100) % 10;
            meter->digits[3] = 0x30 + (v / 10) % 10;
            meter->digits[4] = 0x30 + v % 10;
        }

        meter->measure_cycle_delay = 87000;
        meter->data_count = 0;
        meter->cycle = (meter->cycle + 1) % 7;
    }
    else
    {
        meter->measure_cycle_delay--;
    }

    if (meter->data_cycle_delay == 0) {
        if (meter->data_count >= 0) {
            meter->data_cycle_delay = 436;

            uint8_t digit = 0;
            switch (meter->data_count) {
                case 0: digit = 0x00; break;
                case 1: digit = 0x80; break;
                case 2: digit = 0x40; break;
                case 3: digit = 0x20; break;
                case 4: digit = 0x10; break;
            }
            
            meter->dat_8000 = (meter->digits[meter->data_count] - 0x30) | digit;
            
            aCPU->mSFR[REG_TCON] |= TCONMASK_IE1;
            
            meter->data_count ++;
            if (meter->data_count == 5)
                meter->data_count = -1;


        }
    } else {
        meter->data_cycle_delay--;
    }
}


struct calibrator_board_t {
    struct display_t display;

    multimeter_t multimeter;
    
    uint8_t display_data;
    uint8_t display_ctrl;
};

static struct calibrator_board_t the_board, *board = &the_board;



float v = -0.01f;

void logicboard_tick(struct em8051 *aCPU)
{
    static bool initialized;
    if(!initialized) {
        initialized = true;
        display_init(&board->display);
        multimeter_init(&board->multimeter);
    }

    uint8_t c = board->display_ctrl;
    uint8_t ctrl = ((c & 0x01) ? 0x80 : 0) | // EN
                   ((c & 0x02) ? 0x20 : 0) | // R/W
                   ((c & 0x04) ? 0x40 : 0);  // RS

    display_tick(&board->display, board->display_data, ctrl);

    multimeter_tick(aCPU, &board->multimeter, v);

    {
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
    
}
