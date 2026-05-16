
#include <stdint.h>

void display_init(void);
void display_put_char(char c);
void display_set_cursor(uint8_t row, uint8_t col);
void display_print(char *str);
