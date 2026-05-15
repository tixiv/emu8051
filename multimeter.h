
#ifndef MULTIMETER_H
#define MULTIMETER_H

#include <stdint.h>
#include "emu8051.h"

typedef struct {
    int measure_cycle;
    int data_cycle_delay;
    int data_count;
    uint8_t dat_8000;

    uint8_t digits[5];
} multimeter_t;

void multimeter_init(multimeter_t *meter);
void multimeter_tick(struct em8051 *aCPU, multimeter_t *meter, float value);

#endif
