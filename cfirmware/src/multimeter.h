// This prototype needs to be visible when sdcc compiles
// the unit that includes the main() function. Otherwise no 
// jump to the ISR will be put into the vector table.
void int1_isr(void) __interrupt (2);

float read_multimeter_and_convert_result(void);
