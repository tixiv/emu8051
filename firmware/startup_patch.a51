
.org 0x0000
    ljmp startup

.org 0xffe8
    mov dptr,#0x8003
    mov a,#0x0C
    movx @dptr,a      ; bit mod PC6 low
    ljmp 0xe0c8       ; original firmware start

startup:
    mov dptr,#0x8003
    mov a,#0xb2
    movx @dptr,a      ; configure control word (PC6 bank switching pin goes low now)
    mov a,#0x0D       
    movx @dptr,a      ; bit mod PC6 back high
