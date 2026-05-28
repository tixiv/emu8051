
#ifndef _8255_H
#define _8255_H

// Implements a 8255 simulation.
// The simulation is not so accurate. Only the strobed input mode (mode 1)
// is implemented For port A. The strobed output mode, mode 2
// and port B are not implemented.
//
// Usage:
// Thee user should write IO input levels to in_x in the struct. Output
// levels to the simulated hardware can be read from out_x at any time.
// The write and read functions should be called from the simulated CPU
// when it accesses the registers of this 8255.

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t in_a;
    uint8_t in_b;
    uint8_t in_c;

    uint8_t out_a;
    uint8_t out_b;
    uint8_t out_c;

    /* private, haha */

    uint8_t out_c_latch;

    uint8_t in_a_latch;

    uint8_t control; // Layout:
    // 6:5 = Mode_A,  4 = InOut_A,  3 = InOut_CL, 2 = Mode_B, 1 = InOut_B, 0 = InOut_CL
    // 1 = In, 0 = Out

    bool IBFA;
} _8255_t;

void _8255_init(_8255_t *s) ;
uint8_t _8255_read(_8255_t *s, uint16_t reg);
void _8255_write(_8255_t *s, uint16_t reg, uint8_t value);
void _8255_porta_strobed_input(_8255_t *s, uint8_t value);


#endif
