
#include "integrator.h"
#include "multimeter.h"

#include <stdint.h>


// in 2V = fullscale for all modes
float source_target;

static uint8_t source_mode;
static uint8_t break_cycle;

void source_start(void) {
    source_mode = 1;
    break_cycle = 2;
}

void source_stop(void) {
    source_mode = 0;
}

static void do_integrator(float diff) {
	uint8_t negative = 0;
	if (diff < 0.0f) {
		diff = -diff;
		negative = 1;
	}

	if (diff < 0.1) {
		if (negative)
			pulse_integrator_exact(integrator_calibration[3] * diff * 0.5f, 0xe6);
		else
			pulse_integrator_exact(integrator_calibration[2] * diff * 0.5f, 0xea);
	} else {
        break_cycle = 2;
		if (negative)
			pulse_integrator_exact(integrator_calibration[1] * diff, 0xf6);
		else
			pulse_integrator_exact(integrator_calibration[0] * diff, 0xfa);
	}
}

void source_update(void) {
    if (source_mode) {
		if (!break_cycle) {
            float v = latest_measurement;
			float diff = source_target - v;
			do_integrator(diff * 5.0f);
		} else {
			break_cycle--;
		}
    }
}
