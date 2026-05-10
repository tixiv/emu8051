
#ifndef CALIBRATOR_BOARD_H
#define CALIBRATOR_BOARD_H

#include <stdint.h>

uint8_t calibrator_xread (struct em8051 *aCPU, uint16_t address);

void calibrator_xwrite (struct em8051 *aCPU, uint16_t address, uint8_t value);

#endif
