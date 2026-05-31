
#include <stdint.h>

void display_indexed(uint8_t row, uint8_t idx);
void display_string(uint8_t row, __code const char *str);

#define INT_STRING(x) ((__code const char*)(x))
