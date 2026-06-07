
#include "general.h"
#include "display.h"
#include "print_number.h"
#include "measure_range.h"

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

void integrator_reset_crude(void)
{
    set_measure_range(MR_Int_x0_2);
    read_multimeter_and_convert_result();

    float v = 0;

    for(uint8_t i = 0; i < 6; i++) {
        read_multimeter_and_convert_result();
        read_multimeter_and_convert_result();
        v = latest_measurement;
        
        uint8_t negative = 0;
        if (v < 0.0f) {
            negative = 1;
            v = -v;
        }

        if (v < 0.0020f) break;

        float cycles = 30000.0f * v;

        if (negative) {
            pulse_integrator_exact(cycles, 0xfa);
        } else {
            pulse_integrator_exact(cycles, 0xf6);
        }
    }
}

// In pulses per integrator volt
float integrator_calibration[4];

void do_integrator_calibration(void) {
    integrator_reset_crude();

    read_multimeter_and_convert_result();
    read_multimeter_and_convert_result();
    float v1 = latest_measurement;
     
    pulse_integrator_exact(27648, 0xfa); // 30 ms

    read_multimeter_and_convert_result();
    read_multimeter_and_convert_result();
    float v2 = latest_measurement;
    
    pulse_integrator_exact(27648, 0xf6); // 30 ms

    float v_s = (v2 - v1) * 5.0f / 0.03f;
    integrator_calibration[0] = 921600.0f / v_s;
    display_set_cursor(0, 0);
    print_number(v_s * 100.0f, 4);

    
    read_multimeter_and_convert_result();
    read_multimeter_and_convert_result();
    v1 = latest_measurement;

    MEASURE_RANGE = 0x7e;

    v_s = (v1 - v2) * 5.0f / 0.03f;
    integrator_calibration[1] = -921600.0f / v_s;
    display_set_cursor(1, 0);
    print_number(v_s * 100.0f, 4);


    for (uint8_t x=0; x<4; x++) {
        pulse_integrator_exact(54253, 0x65);
    }


    read_multimeter_and_convert_result();
    read_multimeter_and_convert_result();
    v1 = latest_measurement;

    pulse_integrator_exact(27648, 0xea);

    read_multimeter_and_convert_result();
    read_multimeter_and_convert_result();
    v2 = latest_measurement;

    pulse_integrator_exact(27648, 0xe6);

    v_s = (v2 - v1) * 0.5f / 0.03f;
    integrator_calibration[2] = 921600.0f / v_s;
    display_set_cursor(0, 0);
    print_number(v_s * 1000.0f, 3);

    read_multimeter_and_convert_result();
    read_multimeter_and_convert_result();
    v1 = latest_measurement;

    v_s = (v1 - v2) * 0.5f / 0.03f;
    integrator_calibration[3] = -921600.0f / v_s;
    display_set_cursor(1, 0);
    print_number(v_s * 1000.0f, 3);

    for (uint8_t x=0; x<4; x++) {
        pulse_integrator_exact(54253, 0x65);
    }
}
