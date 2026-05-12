
#include "char_display.h"
#include "emu8051.h"
#include "curses.h"

#include <string.h>

// 44780 -style character display

extern int opt_clock_hz;

FILE *file;

int busy_37us = 37;

void display_init(struct display_t *disp) {
    memset(disp, 0, sizeof(struct display_t));
    disp->dir = 1;
    disp->dcb = 7;

    file = fopen("output.txt", "w");

    busy_37us = 37 * opt_clock_hz / 12000000;
}


// Takes data and control line states, returns data state for reads
// control.7 = EN
// control.6 = RS
// control.5 = RW;
uint8_t display_tick(struct display_t *disp, uint8_t data, uint8_t control)
{
    uint8_t pout = 0;

    if (disp->busy > 0)
        disp->busy--;

    if (((control & 0x20) == 0x20) && 
            ((disp->old_control & 0x80) == 0) &&
            ((control & 0x80) != 0)) 
    {
        // Read op
        // - E level rises from low to high on read ops			

        fprintf (file, "disp read %x\n", control);
        fflush(file);

        if (control & 0x40)
        {   // P3.6
            // memory IO mode

            if (!disp->busy)
            {
                // memory IO mode
                if (disp->chargen == 0)
                {
                    // read from display
                    disp->data = disp->ram[disp->cp & 0x7f];
                    if (!disp->_4bmode || disp->tick)
                    {
                        disp->cp += disp->dir; 
                        if (disp->shift)
                            disp->ofs += disp->dir;
                        disp->busy = busy_37us;
                    }
                }
                else
                {
                    // read from chargen ram
                    disp->data = disp->cgram[disp->cp & 0x3f];
                    if (!disp->_4bmode || disp->tick)
                    {
                        disp->cp++; // assumed; not clear from data sheet
                        disp->busy = busy_37us;
                    }
                }
            }
        }
        else
        {
            // instruction mode				
            disp->data = disp->cp & 0x7f;
            if (disp->busy)
                disp->data |= 0x80;
            // doesn't cause busy states
        }

        if (disp->_4bmode == 0)
        {
            pout = disp->data;
        }
        else
        {	
            if (disp->tick)
                pout = (disp->data << 4) & 0xf0;
            else
                pout = (disp->data << 0) & 0xf0;
            disp->tick = !disp->tick;
        }
    }

    if (((control & 0x20) != 0x20) && 
            ((disp->old_control & 0x80) != 0) &&
            ((control & 0x80) == 0))
    {	// P3.7
        // Write op
        // - E level drops from high to low on write ops

        fprintf (file, "disp write %x %x\n", control, data);
        fflush(file);

        if (disp->_4bmode == 0)
        {
            disp->data = data;
        }
        else
        {
            if (!disp->tick)
            {
                disp->data = (disp->data & 0xf) | (data & 0xf0);
            }
            else
            {
                disp->data = (disp->data & 0xf0) | ((data & 0xf0) >> 4);
            }
            disp->tick = !disp->tick;
        }

        if (!disp->tick || !disp->_4bmode)
        {
            if (control & 0x40)
            {
                if (!disp->busy)
                {
                    // memory IO mode
                    if (disp->chargen == 0)
                    {
                        // write to display
                        disp->ram[disp->cp & 0x7f] = disp->data;
                        disp->cp += disp->dir; 
                        if (disp->shift)
                            disp->ofs += disp->dir;
                        // busy for 250 microseconds
                        disp->busy = busy_37us;
                    }
                    else
                    {
                        // write to chargen ram
                        disp->cgram[disp->cp & 0x3f] = disp->data;
                        disp->cp++;  // assumed: not clear from data sheet
                        // busy for 250 microseconds
                        disp->busy = busy_37us;
                    }
                }
                else {
                    fprintf (file, "disp busy = %d\n", disp->busy);
                }
            }
            else
            {
                // instruction mode				
                if (disp->busy)
                {
                    fprintf (file, "disp busy = %d\n", disp->busy);
                }
                else
                if (disp->data == 1)
                {
                    // Clear display
                    for (int i = 0; i < 0x80; i++)
                        disp->ram[i] = 0x20;
                    disp->cp = 0;
                    disp->ofs = 0;
                    disp->dir = 1; // based on HD44780U data sheet
                    // busy for 2 milliseconds
                    disp->busy = 2*opt_clock_hz / 12000;
                }
                else
                if ((disp->data & (0xff & ~1)) == 2)
                {
                    // return home
                    disp->cp = 0;
                    disp->ofs = 0;
                    disp->busy = busy_37us;
                }
                else
                if ((disp->data & (0xff & ~3)) == 4)
                {
                    // entry mode set.
                    if (disp->data & 1)
                        disp->shift = 1;
                    else
                        disp->shift = 0;
                    if (disp->data & 2)
                        disp->dir = 1;
                    else
                        disp->dir = -1;
                    disp->busy = busy_37us;
                }
                else
                if ((disp->data & (0xff & ~7)) == 8)
                {
                    // display on/off setting.
                    disp->dcb = disp->data & 0x7;
                    disp->busy = busy_37us;
                }
                else
                if ((disp->data & (0xff & ~0xf)) == 0x10)
                {
                    // cursor or display shift.
                    if (disp->data & 8)
                    {
                        // move cursor
                        if (disp->data & 4)
                            disp->cp++;
                        else
                            disp->cp--;
                    }
                    else
                    {
                        // shift display
                        if (disp->data & 4)
                            disp->ofs++;
                        else
                            disp->ofs--;
                    }
                    disp->busy = busy_37us;
                }
                else
                if ((disp->data & (0xff & ~0x1f)) == 0x20)
                {
                    // function set (4/8 bit interface, font size). 
                    disp->_4bmode = (disp->data & 16) == 0;
                    disp->tick = 0;
                    disp->busy = busy_37us;
                }
                else
                if ((disp->data & (0xff & ~0x3f)) == 0x40)
                {
                    // character gen address set. 
                    disp->chargen = 1;
                    disp->busy = busy_37us;
                }
                else
                if ((disp->data & (0xff & ~0x7f)) == 0x80)
                {
                    // cursor position address set
                    disp->cp = disp->data & 0x7f;
                    disp->chargen = 0;
                    disp->busy = busy_37us;
                }
            }
        }
    }

    if (disp->busy < 0)
        disp->busy = 0;
    
    disp->old_control = control;

    return pout;
}

void display_render(struct display_t *disp)
{
	int i;	
	mvprintw(2, 40, "[");
	for (i = 0; i < 16; i++)
	{
		int c = disp->ram[(i + disp->ofs) & 0x7f];
		if ((disp->dcb & 4) == 0) c = ' ';
		if (c == 0) c = ' ';		
		if (c < 32 || c > 126)
			c = '?';
		printw("%c", c);
	}
	printw("]");

	mvprintw(3, 40, "[");
	for (i = 0; i < 16; i++)
	{
		int c = disp->ram[(i + disp->ofs + 0x40) & 0x7f];
		if ((disp->dcb & 4) == 0) c = ' ';
		if (c == 0) c = ' ';
		if (c < 32 || c > 126)
			c = '?';
		printw("%c", c);
	}
	printw("]");

	
	mvprintw(4, 40, "Display %3s, Cursor %3s", (disp->dcb & 4)?"on":"off", (disp->dcb & 2)?"on":"off");
	mvprintw(5, 40, "Blinking %3s, 4bit %3s", (disp->dcb & 1)?"on":"off", (disp->_4bmode & 1)?"on":"off");
	mvprintw(6, 40, "4b tick:%d Busy:%-7d", disp->tick, disp->busy);

	
}
