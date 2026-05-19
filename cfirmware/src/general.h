
#define DAT_EXTMEM(x) *((__xdata volatile uint8_t *)x)
#define BIT_MOD(b,v) (((b)<<1)|(v))

#define TRACE(x) DAT_EXTMEM(0xffff)=(x)
// #define TRACE(x)
