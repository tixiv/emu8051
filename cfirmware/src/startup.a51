
.module startup

.globl __sdcc_gsinit_startup

.area HOME (ABS,CODE)

.org 0xfffd
    ljmp __sdcc_gsinit_startup
