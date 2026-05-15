
#include "char_display.h"
#include "integrator.h"
#include "multimeter.h"
#include "plot.h"
#include "emu8051.h"
#include "curses.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern unsigned int clocks;
extern int opt_clock_hz;

typedef struct {
    float control_voltage;
    float measure_voltage;

} io_board_t;

void io_board_tick(io_board_t *iob, float integrator_voltage, uint8_t range) {
    // Ranges:
    //       M*10 Val GND Batt Temp  Sel Ctrl/10                    Int V  ->  Out   -> measure
    // 0x5f    1   0   1    1    1    1    1      -> Source 20 mA     10 V -> 20 mA  -> 2.00 V
    //                                            -> Source 200 mV    10 V -> 200 mV -> 2.00 V
    // 0x1e    0   0   1    1    1    1    0      -> Source 52 mA    2.6 V -> 52 mA  -> 0.52 V
    //                                            -> Source 2 V       10 V ->  2 V   -> 2.00 V
    // 0x5d    1   0   1    1    1    0    1      -> Source 15 V      7.5V ->  15 V  -> 1.50 V
    //                                            -> R 5 k
    // 0x1f    0   0   1    1    1    1    1      -> Measure 52 mA
    // 0x1d    0   0   1    1    1    0    1      -> Measure 42 V
    // 0x6f    1   1   0    1    1    1    1      -> Measure GND * 10
    // 0x2f    0   1   0    1    1    1    1      -> Measure GND

    float measure_mult = (range & 0x40) ? 10.0f : 1.0f;
    float control_mult = (range & 0x01) ? .02f : .2f;

    iob->control_voltage = integrator_voltage * control_mult;

    float mv = iob->control_voltage;
    switch ((~range) & 0x3C) {
        case 0: mv = iob->control_voltage; break;
        case 0x04: mv = 17.4 * 0.01; break;          // temperature
        case 0x08: mv = 5.26 * 0.1; break;           // battery
        case 0x10: mv = 0.001f; break;               // GND
        case 0x20: mv = iob->control_voltage; break; // measure, not reverse engineered yet
    }

    iob->measure_voltage = mv * measure_mult;
}

struct calibrator_board_t {
    struct display_t display;
    multimeter_t multimeter;
    integrator_t integrator;
    io_board_t io_board;
    plot_t plot;

    uint8_t display_data;

    uint8_t reg_9000;
    uint8_t reg_9002;

    char new_key;
    char current_key;
    int key_timer;

    uint8_t keys_out;

    // mode 0: I Source
    // mode 1: U Source / Thermocouple
    // mode 2: XMTR
    // mode 3: error 15
    // mode 4: I measure
    // mode 5: U measure
    // mode 6: falsch
    // mode 7: error 15
    // mode 8: falsch
    // mode 9: R/PTC messen
    // mode 10: falsch
    // mode 11: error 15
    // mode 12: error 15
    // mode 13: error 15
    // mode 14: error 15
    // mode 15: error 15

    uint8_t mode;

    FILE *logfile;
};

static struct calibrator_board_t the_board, *board = &the_board;

void calibrator_board_init() {
    display_init(&board->display);
    multimeter_init(&board->multimeter);
    plot_init(&board->plot);
    integrator_init(&board->integrator);

    board->mode = 2;

    board->logfile = fopen("log", "w");
}

void keyboard_update() {
    if (board->new_key > 0) {
        board->current_key = board->new_key;
        board->key_timer = 100000;
        board->new_key = 0;
    }

    if (board->key_timer) {
        board->key_timer--;
    } else {
        board->current_key = 0;
    }

    uint8_t keys_out = 0xff;

    if (board->current_key > 0) {
        if ((board->reg_9000 & 0x80) == 0) {
            switch (board->current_key) {
                case '7': keys_out &= (uint8_t)~0x80; break;
                case '8': keys_out &= (uint8_t)~0x20; break;
                case '9': keys_out &= (uint8_t)~0x08; break;
                case '\n': keys_out &= (uint8_t)~0x02; break;
                case '4': keys_out &= (uint8_t)~0x40; break;
                case '5': keys_out &= (uint8_t)~0x10; break;
                case '6': keys_out &= (uint8_t)~0x04; break;
                case '/': keys_out &= (uint8_t)~0x01; break;
            }
        }
        if ((board->reg_9002 & 0x08) == 0) {
            switch (board->current_key) {
                case '1': keys_out &= (uint8_t)~0x80; break;
                case '2': keys_out &= (uint8_t)~0x20; break;
                case '3': keys_out &= (uint8_t)~0x08; break;
                case 'm': keys_out &= (uint8_t)~0x02; break;
                case '#': keys_out &= (uint8_t)~0x40; break;
                case '0': keys_out &= (uint8_t)~0x10; break;
                case ',': keys_out &= (uint8_t)~0x04; break;
                case '*': keys_out &= (uint8_t)~0x01; break;
            }
        }
    }

    board->keys_out = keys_out;
}

void trace_pc(struct em8051 *aCPU);

void logicboard_tick(struct em8051 *aCPU) {
    uint8_t c = board->reg_9002;
    uint8_t ctrl = ((c & 0x01) ? 0x80 : 0) | // EN
                   ((c & 0x02) ? 0x20 : 0) | // R/W
                   ((c & 0x04) ? 0x40 : 0);  // RS

    display_tick(&board->display, board->display_data, ctrl);

    integrator_tick(&board->integrator, aCPU);
    float akk = board->integrator.akk;

    io_board_tick(&board->io_board, akk, board->reg_9000);
    float measure_value = board->io_board.measure_voltage;
    // float measure_value = akk / 5.0f;

    double noise = ((double)rand() / RAND_MAX - 0.5) * 0.0001f;

    measure_value += noise;

    // The meter uses a 1.1V reference instead of 1V, so the numbers
    // are going to be off, but that's normal for this device.
    // Probably this decision is due to the 20mA range
    float multimeter_value = measure_value / 1.1f;

    // if (clocks > 3660000)
    //   multimeter_value = 1.6383;

    multimeter_tick(aCPU, &board->multimeter, multimeter_value);

    keyboard_update();

    plot_update(&board->plot, measure_value);

    trace_pc(aCPU);
}

uint8_t xram[0x8000];

uint8_t write_map[0x8000];
uint8_t read_map[0x8000];

uint8_t calibrator_xread(struct em8051 *aCPU, uint16_t address) {
    if (address < 0x8000) {
        read_map[address] |= 1;
        return xram[address];
    }

    switch (address) {
        case 0x8000:
            aCPU->mSFR[REG_TCON] &= ~TCONMASK_IE1;
            return board->multimeter.dat_8000;

        case 0x8001: return board->keys_out;
        case 0x9000: return board->reg_9000;
        case 0x9002: return (board->reg_9002 & 0x0f) | ((board->mode & 0x0f) << 4);
    }

    return 0xff;
}

void calibrator_xwrite(struct em8051 *aCPU, uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        write_map[address] |= 1;
        xram[address] = value;
        return;
    }

    switch (address) {
        case 0x9000: board->reg_9000 = value; break;
        case 0x9001: board->display_data = value; break;
        case 0x9002: board->reg_9002 = value; break;
        case 0x9003:
            if ((value & 0x80) == 0) {
                // bit mod PORTC
                int bit_num = ((value & 0x0e) >> 1);

                if (value & 0x01)
                    board->reg_9002 |= (1 << bit_num);
                else
                    board->reg_9002 &= ~(1 << bit_num);
            }
    }
}

void calibrator_board_render(struct em8051 *aCPU) {
    display_render(&board->display);

    mvprintw(10, 40, "XRAM 70 = %x", xram[0x70]);
    mvprintw(11, 40, "XRAM 71 = %x", xram[0x71]);
    mvprintw(12, 40, "XRAM A0 = %s                ", &xram[0xa0]);

    uint8_t d = board->multimeter.digits[0];

    char c1 = (d & 4) ? 'o' : (d & 2) ? 'u'
                                      : ' ';

    char c2 = (d & 8) ? ' ' : '-';

    mvprintw(13, 40, "multi = %c %c%c%c%c%c%c", c1, c2,
             board->multimeter.digits[0] & 0x31, board->multimeter.digits[1],
             board->multimeter.digits[2], board->multimeter.digits[3],
             board->multimeter.digits[4]);

    mvprintw(15, 40, "PORT1 = %02x", aCPU->mSFR[REG_P1]);
    mvprintw(16, 40, "inte = %g      ", board->integrator.akk);
    mvprintw(17, 40, "last = %02x", board->integrator.last_pulse_value);
    mvprintw(18, 40, "pc = %04x", board->integrator.written_from);

    mvprintw(18, 0, "range = %02x", board->reg_9000 & 0x7f);

    mvprintw(19, 0, "key = '%c' = %02x  ",
             (board->current_key > 0 ? board->current_key : '^'),
             board->current_key);
    mvprintw(20, 0, "key_timer = '%d'  ", board->key_timer);
    mvprintw(21, 0, "mode = '%x'  ", board->mode);
}

void trace_raw(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(board->logfile, fmt, ap);
    va_end(ap);
}

void trace_msg(const char *fmt, ...) {
    va_list ap;
    
    uint64_t us_total = 1000000ull * clocks / opt_clock_hz;
    
    int us = us_total % 1000000;
    int s = (us_total / 1000000) % 60;
    int m = us_total / 1000000 / 60;

    fprintf(board->logfile, "[%02d:%02d.%06d] ", m, s, us);

    va_start(ap, fmt);
    vfprintf(board->logfile, fmt, ap);
    va_end(ap);
    fflush(board->logfile);
}

void trace_flush() {
    fflush(board->logfile);
}

uint8_t xram_copy[32768];

void snapshot(struct em8051 *aCPU) {
    memcpy(xram_copy, xram, sizeof(xram_copy));
    memset(write_map, 0, sizeof(write_map));
    memset(read_map, 0, sizeof(read_map));
    trace_msg("Snapshot taken at PC: %04x, Time: %14.3f ms\n",
            aCPU->mPC, clocks / 1000.0f);
    
}

void hexdump(uint8_t *data, int len) {
    for (int i = 0; i < len; i++) {
        trace_raw("%02X ", data[i]);
    }

    trace_raw(" | ");

    for (int i = 0; i < len; i++) {
        trace_raw("%c", isprint(data[i]) ? data[i] : '.');
    }

    if (len == 4) {
        trace_raw(" | %f", (double)*(float *)data);
    }

    trace_raw("\n");
}

void print_diff(int diff_start, int diff_end, const char *desc) {
    int size = diff_end - diff_start;
    trace_raw("---- %s at %04x size %02x -----\n", desc, diff_start, size);
    trace_raw("from: ");
    hexdump(&xram_copy[diff_start], size);
    trace_raw("to:   ");
    hexdump(&xram[diff_start], size);
}

void find_chains(uint8_t *map, const char *desc) {
    const int size = 32768;
    int i;
    int diff_start = -1;
    for (i = 0; i < size; i++) {
        if (map[i]) {
            if (diff_start == -1) {
                diff_start = i;
            }
        }
        if (!map[i] || i == size - 1) {
            if (diff_start != -1) {
                print_diff(diff_start, i, desc);
                diff_start = -1;
            }
        }
    }
}

void diff(struct em8051 *aCPU) {
    trace_msg("Diff at PC: %04x, Time: %14.3f ms\n", aCPU->mPC,
            clocks / 1000.0f);
    find_chains(read_map, "Reads");
    find_chains(write_map, "Writes");

    trace_flush();
}

void logicboard_editor_keys(struct em8051 *aCPU, int ch) {
    switch (ch) {
        case 's':
            snapshot(aCPU);
            break;
        case 'd':
            diff(aCPU);
            break;
        case 'M':
            board->mode = (board->mode + 1) % 16;
            break;
        case '>':
            board->integrator.akk += 0.5f;
            break;
        case '<':
            board->integrator.akk -= 0.5f;
            break;
        default:
            board->new_key = ch;
    }
}

float read_fac(struct em8051 *aCPU) {
    uint8_t raw[4];

    raw[0] = aCPU->mLowerData[0x3B];
    raw[1] = aCPU->mLowerData[0x3C];

    uint8_t cy = aCPU->mLowerData[0x20] & 0x80;
    raw[3] = (aCPU->mLowerData[0x36] >> 1) | cy;
    raw[2] = aCPU->mLowerData[0x3D];

    if (aCPU->mLowerData[0x36] & 1)
        raw[2] |= 0x80;
    else
        raw[2] &= 0x7F;

    float f;
    memcpy(&f, raw, 4);
    return f;
}

uint16_t get_caller(struct em8051 *aCPU) {
    uint8_t sp = aCPU->mSFR[REG_SP];
    uint8_t *d = aCPU->mLowerData;
    return d[sp] << 8 | d[sp - 1];
}

void trace_multimeter_read(struct em8051 *aCPU) {
    uint16_t caller = get_caller(aCPU);
    if (caller == 0x679) // Don't trcae calls from read and convert
        return;

    trace_msg("Multimeter read from %04x '%s' range %02x\n",
            caller, &xram[0xa0], board->reg_9000 & 0x7f);
}

void trace_multimeter_read_and_convert(struct em8051 *aCPU) {

    trace_msg("Multimeter read and convert from %04x %f range %02x\n",
            get_caller(aCPU), *(float *)&xram[0x187c], board->reg_9000 & 0x7f);
}

void trace_extmem_70() {
    trace_msg("Extmem 70 = %02x %02x\n", xram[0x70], xram[0x71]);
}

void trace_3f_pointer(struct em8051 *aCPU) {
    uint16_t p = aCPU->mLowerData[0x3f] << 8 | aCPU->mLowerData[0x40];
    float v = *(float *)&xram[p];

    trace_msg("trace math op - fac = %f, argument = %f\n", read_fac(aCPU), v);
}

void trace_fun762(struct em8051 *aCPU) {
    uint16_t caller = get_caller(aCPU);
    trace_msg("fun762 from %04x\n", caller);
}

void trace_pc(struct em8051 *aCPU) {
    uint16_t pc = aCPU->mPC;

    switch (pc) {
        case 0xddca: trace_multimeter_read(aCPU); break;
        case 0x06e8: trace_multimeter_read_and_convert(aCPU); break;
        // case 0x785:  trace_extmem_70(); break;
        case 0xdffd: trace_3f_pointer(aCPU); break;
        case 0xe00a: trace_extmem_70(); break;
        case 0x07c5: trace_fun762(aCPU); break;
    }
}
