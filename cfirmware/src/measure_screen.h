
#include <stdint.h>

typedef struct {
    uint8_t num_ranges;
    struct {
        uint8_t title;
        // __code const char *name;
        int8_t dp;
        uint8_t range;
    } ranges[];
} measure_screen_t;

uint8_t measure_screen_update(void);
void measure_screen_set(measure_screen_t *measure_screen);
