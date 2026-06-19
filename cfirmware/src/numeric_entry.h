
#include <stdint.h>

enum NumericEntryStatus {
    NES_OKAY,
    NES_CANCEL
};

typedef void (*numeric_entry_handler_t) (float result, uint8_t status) __reentrant;

void numeric_entry_init(numeric_entry_handler_t handler);
void numeric_entry_update(void);
