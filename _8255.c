
#include "_8255.h"

extern void trace_msg(const char *fmt, ...);

static void update_port_c(_8255_t *s) {
    uint8_t new_out_c = s->out_c_latch;

    if ((s->control & 0x70) == 0x30) { // Port A mode 1 input
        new_out_c &= 0xC7; // clear bits 5:3 which are now driven by the input logic

        if (s->IBFA) {
            new_out_c |= 0x20;            // PC5: IBFA signal
            if (s->out_c_latch & 0x10) {  // INTEA ?
                new_out_c |= 0x08;        // PC3: INTRA signal
            }
        }
    }

    s->out_c = new_out_c;
}

uint8_t _8255_read(_8255_t *s, uint16_t reg) {
    switch (reg % 4) {
        case 0:
            s->IBFA = 0; // clear input buffer full flag
            update_port_c(s);
            
            if ((s->control & 0x70) == 0x30) { // Port A mode 1 input
                trace_msg("8255 Sync data read = %x", s->in_a_latch);
                return s->in_a_latch;
            } else {
                return s->control & 0x10 ? s->in_a : s->out_a;
            }

        case 1:
            return s->control & 0x02 ? s->in_b : s->out_b;
        
        case 2: 
            return ((s->control & 0x08 ? s->in_c : s->out_c) & 0xf0) |
                   ((s->control & 0x01 ? s->in_c : s->out_c) & 0x0f);
        case 3:
            return s->control;
    }
    return 0;
}

void _8255_write(_8255_t *s, uint16_t reg, uint8_t value) {
    switch (reg % 4) {
        case 0: s->out_a = value; break;
        case 1: s->out_b = value; break;
        case 2: s->out_c_latch = value; update_port_c(s) ; break;
        case 3:
            if (value & 0x80) {
                s->control = value & 0x7f;
            } else {
                // bit mod PORTC
                int bit_num = ((value & 0x0e) >> 1);

                if (value & 0x01)
                    s->out_c_latch |= (1 << bit_num);
                else
                    s->out_c_latch &= ~(1 << bit_num);
            }

            update_port_c(s);
            break;
    }
}

void _8255_porta_strobed_input(_8255_t *s, uint8_t value) {
    s->IBFA = true;
    s->in_a_latch = value;
    update_port_c(s);
}
