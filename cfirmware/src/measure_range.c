
#include "general.h"

uint8_t current_measure_range;

void set_measure_range(uint8_t range) {
    current_measure_range = range;
    DAT_EXTMEM(0x9000) = range & 0x7f;

    if (range & 0x80)
        DAT_EXTMEM(0x8003) = BIT_MOD(7,1);
    else
        DAT_EXTMEM(0x8003) = BIT_MOD(7,0);
}
