

.equ temp_41, 0x41
.equ BANK1_R1, 09h
.equ BANK1_R2, 0ah

.equ EXTMEM_digit_0, 0x1a28
.equ EXTMEM_digit_1, 0x1a29
.equ EXTMEM_digit_2, 0x1a2a
.equ EXTMEM_digit_3, 0x1a2b
.equ EXTMEM_digit_4, 0x1a2c


; This patch removes the wait from the source loop.
; The integrator badly messes up on large diffs if not one
; measurement cycle is skipped, so this patch can't really be enabled.
;.org 0x0a56
;    nop
;    nop
;    nop

; Patch out some waits in main startup
.org 0x19aa
    nop
    nop
    nop

.org 0x1b64
    nop
    nop
    nop

.org 0x1c5c
    nop
    nop
    nop

; Patch read multimeter to not use interrupt
.org 0xdb75
    MOV        DPTR,#0x8003
    mov        a,#0x09        ; bitmod: set 8255 interrupt enable
    movx       @dptr,a
    
    clr        a
    mov        temp_41,a
    
    MOV        DPTR,#0x8000
    MOVX       A,@DPTR        ; clear any pending interrupts
    

wait_P3_3:
    JB         P3.3, wait_P3_3 ; wait for interrupt to be asserted by 8255
    
    MOVX       A,@DPTR        ; Read data, clear interrupt
    MOV        BANK1_R1,A     ; Save result

    MOV        A,temp_41      ; state 0?
    JNZ        process_byte   ; no, already synced

    mov        a,BANK1_R1
    anl        a,#0xf0        ; Digit D5 (MSB) ?
    jnz        wait_P3_3      ; No, wait for it

    mov        a,#1
    mov        temp_41,a

process_byte:
    MOV        A,temp_41
    ADD        A,#0x42
    MOV        R0,A
    mov        a,BANK1_R1
    MOV        @R0,A
    INC        temp_41;
    MOV        A,temp_41
    CLR        CY
    SUBB       A,#0x6       ; // are we done ?
    JC         wait_P3_3    ; // No: next byte;
 
 ; dba8:
    mov        dptr,#EXTMEM_digit_0
    mov        a,0x43
    anl        a,#0x0f
    movx       @dptr,a;

    inc        dptr
    mov        a,0x44
    anl        a,#0x0f
    orl        a,#0x30
    movx       @dptr,a;

    inc        dptr
    mov        a,0x45
    anl        a,#0x0f
    orl        a,#0x30
    movx       @dptr,a;

    inc        dptr
    mov        a,0x46
    anl        a,#0x0f
    orl        a,#0x30
    movx       @dptr,a

    inc        dptr
    mov        a,0x47
    anl        a,#0x0f
    orl        a,#0x30
    movx       @dptr,a

    mov        BANK1_R2,#0 ; keep stupid code happy
    sjmp       0xdc3f
