
#include "integrator.h"
#include <stdint.h>

void integrator_init (integrator_t *integrator) {
    integrator->akk = 0.0;
}

#define k *1000
#define M *1000000

static const float capacitor = 0.0000022;
static const float delta_time = 0.000001;

void integrator_tick(integrator_t *integrator, struct em8051 *aCPU) {
    uint8_t port1 = aCPU->mSFR[REG_P1];

    bool up = (port1 & 0x04) == 0;
    bool down = (port1 & 0x08) == 0;

    if (up || down) {
        float resistor = (port1 & 0x01) == 0 ? 10 k  : 
                        (port1 & 0x20) == 0 ? 100 k :
                        (port1 & 0x40) == 0 ? 1 M   :
                        (port1 & 0x80) == 0 ? 10 M  : 0.0f;

        if (resistor != 0.0f) {
            float voltage = (port1 & 0x12) == 0x12 ? 2.86f :
                            (port1 & 0x12) == 0x02 ? 0.037f :
                                0.0037f; // unsure with this one
            
            double diff = (voltage / resistor / capacitor) * delta_time;

            if (up) integrator->akk += diff;
            if (down) integrator->akk -= diff;

            if (integrator->last_port != port1) {
                integrator->last_pulse_value = port1;
                integrator->written_from = aCPU->mPC;
            }
        }
    }

    // float error_current = -0.000000036f;
    // integrator->akk += error_current / capacitor;

    if (integrator->akk > 16.5f) integrator->akk = 16.5f;
    if (integrator->akk < -6.5f) integrator->akk = -6.5f;

    integrator->last_port = port1;
}
