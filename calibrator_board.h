
#ifndef CALIBRATOR_BOARD_H
#define CALIBRATOR_BOARD_H

#include "emu8051.h"
#include <stdint.h>

void calibrator_board_init ();

uint8_t calibrator_xread (struct em8051 *aCPU, uint16_t address);
void calibrator_xwrite (struct em8051 *aCPU, uint16_t address, uint8_t value);

void calibrator_board_render (struct em8051 *aCPU);

#endif
