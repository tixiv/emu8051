
#ifndef CHAR_DISPLAY_H
#define CHAR_DISPLAY_H

#include <stdint.h>

struct display_t {
    // for the 2x16 character display
    unsigned char ram[0x80];
    unsigned char cgram[0x40];
    int cp;
    int ofs;
    int dir;
    int shift;
    int dcb;
    int chargen;
    int data;
    int _4bmode;
    int tick;
    int busy;

    uint8_t old_control;
};

void display_init(struct display_t *disp);

// Takes data and control line states, returns data state for reads
// control.7 = EN
// control.6 = RS
// control.5 = RW;
uint8_t display_tick(struct display_t *disp, uint8_t data, uint8_t control);

void display_render(struct display_t *disp);

#endif
