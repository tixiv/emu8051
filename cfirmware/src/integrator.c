
#include "general.h"
#include "display.h"

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <8051.h>
#include "multimeter.h"


void pulse_exact(uint16_t cycles, uint8_t value) {
    (void)cycles;
    (void)value;
    // clang-format off
   __asm
        ; DPH:DPL contains cycles
    CLR TR0
    CLR TF0

    MOV TMOD,#0x01     ; Timer0 mode 1 (16-bit)

    MOV TH0,dph
    MOV TL0,dpl

    mov P1,a          ; pulse start
    SETB TR0          ; timer start

    mov a,#0x65

pulse_exact_wait:
    JNB TF0,pulse_exact_wait

    mov P1,a

    CLR TR0
    CLR TF0

    __endasm;
    // clang-format on
}

void pulse_integrator_exact(uint16_t cycles, uint8_t value) {
    if (cycles < 4)
        return;
    pulse_exact(2 - cycles, value);
}

void pulse_integrator_test(uint16_t cycles) {
    pulse_integrator_exact(cycles, 0xea);
}

float integrator_reset_crude(void)
{
    DAT_EXTMEM(0X9000) = 0x3e;
    
    float v = 0;

    for(uint8_t i = 0; i < 5; i++) {
        read_multimeter_and_convert_result();
        v = read_multimeter_and_convert_result();

        uint8_t negative = 0;
        if (v < 0) {
            negative = 1;
            v = -v;
        }

        if (v < 100.0f) break;

        float cycles = 37000.0f / 10000.0f * v;

        if (negative) {
            pulse_integrator_exact(cycles, 0xfa);
            v = -v;
        } else {
            pulse_integrator_exact(cycles, 0xf6);
        }
    }

    return v;
}

static char buffer[10];

void print_number(int16_t value) {
    __itoa(value, buffer, 10);
    display_print(buffer);
}

void integrator_calib(void) {
    float v1 = integrator_reset_crude();
    
    pulse_integrator_exact(27648, 0xfa);

    read_multimeter_and_convert_result();
    float v2 = read_multimeter_and_convert_result();

    float v_s = (v2 - v1) * 5.0f * 100.0f / 0.03f;

    display_set_cursor(0, 0);
    print_number(v_s);

}
