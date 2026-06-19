
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

static uint8_t tc_type;
static uint8_t tc_char;
static uint8_t tc_ref;

static void init_tc_measurement(void);

void enter_thermocouple_ref(uint8_t entry) {
    tc_ref = entry;
    pop_menu_stack(); // ref menu
    // TODO: entry 2: input custom tmperature
    init_tc_measurement();
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


static struct {
    float measure_akk;
    float ref_temp_akk;
    float ref_temp;
    float gnd_akk;
    float cold_junction_mV;
} tc_state;

static uint8_t channel_idx;
static uint8_t measure_init_countdown;

static const float alpha = 0.1;
static const float alpha_aux = 0.2;

static void update_tc_measurement(void) {
    
    // rotate ranges: GND, TC, TC, TC LM35c, TC, TC, TC (repeat)
    switch (channel_idx) {
        case 0: set_measure_range(MR_GND_x30); break;
        case 4: set_measure_range(MR_Temp); break;
        default: set_measure_range(MR_66mV_6_6mA); break;
    }

    if (measure_init_countdown) {
        switch (channel_idx) {
            case 1: tc_state.gnd_akk = latest_measurement; break;
            case 2: tc_state.measure_akk = latest_measurement; break;
            case 5: tc_state.ref_temp_akk = latest_measurement; break;                
        }
        measure_init_countdown--;
    }

    switch (channel_idx) {
        case 1:
            tc_state.gnd_akk += alpha_aux * (latest_measurement - tc_state.gnd_akk);
            break;

        case 5:
            tc_state.ref_temp_akk += alpha_aux * (latest_measurement - tc_state.ref_temp_akk);
            switch (tc_ref){
                case 0: tc_state.ref_temp = tc_state.ref_temp_akk; break;
                case 1: tc_state.ref_temp = 0.0f; break;
                case 2: tc_state.ref_temp = 0.5f; break;
            }
            tc_state.cold_junction_mV = calc_voltage_from_temperature(current_table,  tc_state.ref_temp * 100.0f);
            break;

        default:
            tc_state.measure_akk += alpha * (latest_measurement - tc_state.measure_akk);
            break;
    }

    if (measure_init_countdown < 3) {
        if (channel_idx == 5) {

            display_set_cursor(0,10);
            print_number_cropped(tc_state.ref_temp * 1000.0f, 5, 2, 0);
            display_print("\xdf" "C");
        }
        if (channel_idx == 0 || channel_idx == 4) {
            display_set_cursor(1,0);
            display_print("     ");

            float millivolts = tc_state.measure_akk * (100.0f/3.0f);
            millivolts += tc_state.cold_junction_mV;
            float t = calc_temperature_from_voltage(current_table, millivolts);

            print_number_cropped(t * 10.0f, 5, 0, 1);
            display_print("\xdf" "C         ");
        }
    } else {
            display_set_cursor(0,11);
            display_print("--.-" "\xdf" "C");

            display_set_cursor(1,0);
            display_print("      --.-" "\xdf" "C         ");
    }

    channel_idx++;
    channel_idx &= 0x07;
}

static void init_tc_measurement(void) {
    set_measure_range(MR_GND_x30);

    current_screen = SCR_MEASURE_TC;
    display_indexed(0, 0xae);
    display_set_cursor(0,4);
    display_put_char(tc_char);

    channel_idx = 0;
    measure_init_countdown = 8;
}


uint8_t measure_tc_screen_update(void) {
    update_tc_measurement();

    switch (key_buffer) {
        case KEY_ESC:
            return KEY_ESC;
    }

    return 0;
}
