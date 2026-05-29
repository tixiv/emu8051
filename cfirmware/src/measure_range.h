
#include <stdint.h>

#define MR_x3 0x80
#define MR_x10 0x40
#define MR_Ctrl_div_10  0x01
#define MR_N_div_100    0x02
#define MR_select_Int   0x3c
#define MR_select_Value 0x1c
#define MR_select_GND   0x2c
#define MR_select_Batt  0x34
#define MR_select_Temp  0x38

#define MR_15V        (MR_select_Value | MR_x10 | MR_Ctrl_div_10)
#define MR_2V_52mA    (MR_select_Value | MR_N_div_100)
#define MR_200mV_20mA (MR_select_Value | MR_N_div_100 | MR_x10 | MR_Ctrl_div_10)
#define MR_66mV_6_6mA (MR_select_Value | MR_N_div_100 | MR_x10 | MR_Ctrl_div_10 | MR_x3)

#define MR_Int_x0_2  (MR_select_Int   | MR_N_div_100)
#define MR_Int_x2    (MR_select_Int   | MR_N_div_100 | MR_x10)
#define MR_Int_x6    (MR_select_Int   | MR_N_div_100 | MR_x10 | MR_x3)

#define MR_GND       (MR_select_GND   | MR_N_div_100)
#define MR_GND_x10   (MR_select_GND   | MR_N_div_100 | MR_x10)
#define MR_GND_x30   (MR_select_GND   | MR_N_div_100 | MR_x10 | MR_x3)

#define MR_Batt      (MR_select_Batt  | MR_N_div_100)
#define MR_Temp      (MR_select_Temp  | MR_N_div_100)

void set_measure_range(uint8_t range);

extern uint8_t current_measure_range;
