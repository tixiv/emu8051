
#include "thermocouple.h"
#include "menu.h"
#include "ui.h"
#include "display.h"
#include "multimeter.h"
#include "print_number.h"
#include "measure_range.h"


typedef struct {
    float coefficients[2][4][4];
    float range_limits[2][5];
} thermocouple_table_t;

const __code thermocouple_table_t * const thermocouple_tables = (const __code thermocouple_table_t *)0x5662;

__code const thermocouple_table_t *current_table;

int8_t find_range(__code const float *range_limits, float value) {
    if (value < range_limits[0])
        return -1;

    for(int i = 1; i < 5; i++) {
        if (value < range_limits[i])
            return i-1;
    }
    return 4;
}

float calculate_polynom(__code const float *coefficients, float x) {
    float akk = 0.0f;
    float x_n = 1.0f;

    for (int i = 0; i < 4; i++) {
        akk += x_n * coefficients[i];
        x_n *= x;
    }
    return akk;
}

float table_calc(__code const thermocouple_table_t *table, uint8_t direction, float v) {
    int8_t rng = find_range(table->range_limits[direction], v);

    if (rng < 0) {
        rng = 0;
        v = table->range_limits[direction][0];
    }
    else if (rng > 3) {
        rng = 3;
        v = table->range_limits[direction][4];
    }

    return calculate_polynom(table->coefficients[direction][rng], v);
}

float calc_voltage_from_temperature(__code const thermocouple_table_t *table, float t) {
    return table_calc(table, 1, t);
}

float calc_temperature_from_voltage(__code const thermocouple_table_t *table, float u) {
    return table_calc(table, 0, u);
}

uint8_t tc_type;
uint8_t tc_char;
uint8_t tc_ref;


void enter_thermocouple_ref(uint8_t entry) {
    tc_ref = entry;
    current_screen = SCR_MEASURE_TC;
    display_indexed(0, 0xae);
    display_set_cursor(0,4);
    display_put_char(tc_char);
    set_measure_range(MR_66mV_6_6mA);
}

menu_t sub_menu_thermocouple_ref = {
    INT_STRING(0x9c), 3, {
        {INT_STRING(0xab), enter_thermocouple_ref},
        {INT_STRING(0xac), enter_thermocouple_ref},
        {INT_STRING(0xad), enter_thermocouple_ref},
    }
};

void enter_thermocouple_type(uint8_t entry) {
    tc_type = entry;
    tc_char = entry["JLTUKESRB"];
    current_table = &thermocouple_tables[tc_type];
    sub_menu_thermocouple_ref.title = INT_STRING(0x9c + entry);
    enter_sub_menu(&sub_menu_thermocouple_ref);
}

__code menu_t sub_menu_thermocouple_type = {
    INT_STRING(0xe6), 9, {
        {INT_STRING(0x9c), enter_thermocouple_type},
        {INT_STRING(0x9d), enter_thermocouple_type},
        {INT_STRING(0x9e), enter_thermocouple_type},
        {INT_STRING(0x9f), enter_thermocouple_type},
        {INT_STRING(0xa0), enter_thermocouple_type},
        {INT_STRING(0xa1), enter_thermocouple_type},
        {INT_STRING(0xa2), enter_thermocouple_type},
        {INT_STRING(0xa3), enter_thermocouple_type},
        {INT_STRING(0xa4), enter_thermocouple_type},
    }
};

void menu_entry_tc_source(uint8_t entry) {
    (void) entry;
    enter_sub_menu(&sub_menu_thermocouple_type);
}

void menu_entry_tc_measure(uint8_t entry) {
    (void) entry;
    enter_sub_menu(&sub_menu_thermocouple_type);
}

uint8_t measure_tc_screen_update(void) {
    switch (key_buffer) {
        case KEY_ESC:
            return KEY_ESC;
    }

    display_set_cursor(0,10);
    print_number_cropped(173, 5, 2, 0);
    display_print("\xdf" "C");

    display_set_cursor(1,0);
    display_print("     ");

    float millivolts = latest_measurement * (100.0f/3.0f);
    millivolts += calc_voltage_from_temperature(current_table,  17.3f);
    float t = calc_temperature_from_voltage(current_table, millivolts);

    print_number_cropped(t * 10.0f, 5, 0, 1);
    display_print("\xdf" "C         ");

    return 0;
}
