
.org 0xfff0
    mov dptr,#0x8003
    mov  a,#0x0C
    movx @dptr,a      ; bit mod PC6 low
    nop
    nop
    nop
    nop
    ljmp 0xe0c8       ; original firmware start
