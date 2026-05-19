

#include <stdint.h>

#define DAT_EXTMEM(x) *((__xdata volatile uint8_t *)x)

#define REG_CONTROL DAT_EXTMEM(0x9003)
#define REG_DATA DAT_EXTMEM(0x9001)

#define BIT_MOD(b, v) (((b) << 1) | (v))

#define BIT_EN 0
#define BIT_RW 1
#define BIT_RS 2

// delays about 2.4 us per cycle
void delay_16(uint16_t cycles) {
    (void)cycles;
    // clang-format off
   __asm

        ; DPH:DPL contains cycles
    loop:
        nop
        djnz dpl,loop
        mov  a,dph
        jz   done
        dec  dph
        sjmp loop
    done:
        ret
	__endasm;
    // clang-format on
}

#define DELAY_US(x) delay_16((uint16_t)((float)(x)/2.4))

void clock_data_to_display(void) {
    REG_CONTROL = BIT_MOD(BIT_EN, 1);
    REG_CONTROL = BIT_MOD(BIT_EN, 0);
}

void display_init(void) {
    REG_CONTROL = BIT_MOD(BIT_EN, 0);
    REG_CONTROL = BIT_MOD(BIT_RW, 0);
    REG_CONTROL = BIT_MOD(BIT_RS, 0);

    REG_DATA = 0x3c;
    clock_data_to_display();
    DELAY_US(2500);
    clock_data_to_display();
    DELAY_US(200);
    clock_data_to_display();
    DELAY_US(200);
    REG_DATA = 6;
    clock_data_to_display();
    DELAY_US(50);
    REG_DATA = 1; // clear display
    clock_data_to_display();
    DELAY_US(2500);
    REG_DATA = 0xc; // display on, cursor off, blink off
    clock_data_to_display();
    DELAY_US(40);
}

void display_put_char(char c) {
    REG_DATA = c;
    REG_CONTROL = BIT_MOD(BIT_RS, 1);
    clock_data_to_display();
    DELAY_US(40);
}

void display_set_cursor(uint8_t row, uint8_t col) {
    REG_DATA = 0x80 | (uint8_t)(row << 6) | col;
    REG_CONTROL = BIT_MOD(BIT_RS, 0);
    clock_data_to_display();
    DELAY_US(40);
}

float get_float(void) {
    return 16383.0f;
}

float soll = 10000;

void display_print(const char *str) {
    while (1) {
        char c = *str++;
        if (c == 0) break;
        display_put_char(c);
    }
}

void display_print_16(const char *str) {
    uint8_t i = 16;
    while (i--) {
        char c = *str++;
        display_put_char(c);
    }
}
