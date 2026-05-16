

#include <stdint.h>

#define DAT_EXTMEM(x) *((__xdata volatile uint8_t *)x)

#define REG_CONTROL DAT_EXTMEM(0x9003)
#define REG_DATA DAT_EXTMEM(0x9001)

#define BIT_MOD(b, v) (((b) << 1) | (v))

#define BIT_EN 0
#define BIT_RW 1
#define BIT_RS 2

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
    delay_16(50); // 13.931ms, 16.696ms
    clock_data_to_display();
    delay_16(5);
    clock_data_to_display();
    delay_16(5);
    REG_DATA = 6;
    clock_data_to_display();
    delay_16(5);
    REG_DATA = 1;
    clock_data_to_display();
    delay_16(0x3c);
    REG_DATA = 0xc;
    clock_data_to_display();
    delay_16(5);
}

void display_put_char(char c) {
    REG_DATA = c;
    REG_CONTROL = BIT_MOD(BIT_RS, 1);
    clock_data_to_display();
    delay_16(10);
}

void display_set_cursor(uint8_t row, uint8_t col) {
    REG_DATA = 0x80 | (uint8_t)(row << 6) | col;
    REG_CONTROL = BIT_MOD(BIT_RS, 0);
    clock_data_to_display();
    delay_16(10);
}

float get_float(void) {
    return 16383.0f;
}

void display_print(char *str) {
    while (1) {
        char c = *str++;
        if (c == 0) break;
        display_put_char(c);
    }
}