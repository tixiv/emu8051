
#include <stdint.h>

void pulse_integrator_exact(uint16_t cycles, uint8_t value);
void do_integrator_calibration(void);

extern float integrator_calibration[4];
