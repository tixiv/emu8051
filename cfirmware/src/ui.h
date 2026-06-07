
#include <stdint.h>

void display_indexed(uint8_t row, uint8_t idx);

void return_to_menu(void);

void update_ui(void);
void init_ui(void);



extern uint8_t current_mode;
enum Mode {
    MODE_SOURCE_I  = 0,
    MODE_SOURCE_U  = 1,
    MODE_XMTR      = 2,
    MODE_MEASURE_I = 4,
    MODE_MEASURE_U = 5,
};

extern uint8_t current_screen;
enum Screen {
    SCR_MENU = 0,
    SCR_SOURCE = 1,
    SCR_TESTS  = 2,
    SCR_NUMERIC_ENTRY = 3,
    SCR_MEASURE = 4,
    SCR_MEASURE_TC = 5,
};