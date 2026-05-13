
#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include "emu8051.h"

typedef struct {
    double akk;
    uint8_t last_port;
    uint8_t last_pulse_value;
    uint16_t written_from;
} integrator_t;

void integrator_init (integrator_t *integrator);
void integrator_tick(integrator_t *integrator, struct em8051 *aCPU);

#endif
