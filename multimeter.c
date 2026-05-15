
#include "multimeter.h"
#include "emu8051.h"
#include <string.h>
#include <stdbool.h>

// uint8_t digits[] = "81111" "02222" "83333" "04444" "85555" "06666" "87777" "08888";

extern void trace_raw(const char *fmt, ...);
extern void trace_msg(const char *fmt, ...) ;

void multimeter_init(multimeter_t *meter) {
    memset(meter, 0, sizeof(multimeter_t));
    meter->data_count = -1;
}

void multimeter_tick(struct em8051 *aCPU, multimeter_t *meter, float value) {
    if (meter->measure_cycle == 24000) {
        trace_msg("Multimeter sample %f\n", value);

        // 2V full range
        int v = (value * 10000.0f) + 0.5f;

        bool plus = value > 0.0f;
        bool ov = false;
        if (v > 19999) {
            ov = true;
            v = 19999;
        }
        if (v < -19999) {
            ov = true;
            v = -19999;
        }

        if (v < 0) v = -v;

        bool ur = v <= 1800;

        if (!ov) {
            meter->digits[0] = 0x30 | (v >= 10000 ? 1 : 0) | (plus ? 8 : 0) | (ov ? 4 : 0) | (ur ? 2 : 0);
            meter->digits[1] = 0x30 + (v / 1000) % 10;
            meter->digits[2] = 0x30 + (v / 100) % 10;
            meter->digits[3] = 0x30 + (v / 10) % 10;
            meter->digits[4] = 0x30 + v % 10;
        } else {
            meter->digits[0] = 0x30 | (plus ? 8 : 0) | (ov ? 4 : 0);
            meter->digits[1] = 0x30;
            meter->digits[2] = 0x30;
            meter->digits[3] = 0x30;
            meter->digits[4] = 0x30;
        }
    }

    meter->measure_cycle++;

    if (meter->measure_cycle == 96000) {
        trace_msg("Multimeter ready\n");
        meter->measure_cycle = 0;
        meter->data_count = 0;
    }

    if (meter->data_cycle_delay == 0) {
        if (meter->data_count >= 0) {
            meter->data_cycle_delay = 480;

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

            meter->data_count++;
            if (meter->data_count == 5)
                meter->data_count = -1;
        }
    } else {
        meter->data_cycle_delay--;
    }
}
