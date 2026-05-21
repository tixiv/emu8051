
#include <stdint.h>

void pulse_integrator_exact(uint16_t cycles, uint8_t value);
void integrator_calib(void);

extern uint8_t break_cycle;

void do_integrator(float diff);
