

.equ temp_41, 0x41
.equ BANK1_R1, 09h

.org 0x0a56
    nop
    nop
    nop

.org 0x17de
    MOV        DPTR,#0x8000
    MOVX       A,@DPTR        ; Read data, clear interrupt
    MOV        BANK1_R1,A     ; Save result

    MOV        A,temp_41      ; state 0?
    JNZ        process_byte   ; no, already synced

    mov        a,BANK1_R1
    anl        a,#0xf0
    jnz        isr_return

    mov        a,#1
    mov        temp_41,a

process_byte:
    MOV        A,temp_41
    CLR        CY
    SUBB       A,#0x6
    JNC        isr_return
    MOV        A,temp_41
    DEC        A
    ADD        A,#0x43
    MOV        R0,A
    mov        a,BANK1_R1
    MOV        @R0,A
    INC        temp_41

isr_return:
    RET





;     MOV        A,BANK1_R1
;     SETB       CY
;     SUBB       A,#0xc8
;     JNC        LAB_CODE_17fa
;     MOV        DPTR,#0x8000
;     MOVX       A,@DPTR
;     MOV        BANK1_R2,A
;     MOV        R0,#0x9
;     MOV        A,#0x1
;     ADD        A,@R0
;     MOV        @R0,A
;     JNC        LAB_CODE_17e5
; LAB_CODE_17fa:
;     MOV        BANK1_R1,#0x1
; LAB_CODE_17fd:
;     MOV        A,BANK1_R1
;     SETB       CY
;     SUBB       A,#0xc8
;     JNC        LAB_CODE_1812
;     MOV        DPTR,#0x8000
;     MOVX       A,@DPTR
;     MOV        BANK1_R2,A
;     MOV        R0,#0x9
;     MOV        A,#0x1
;     ADD        A,@R0
;     MOV        @R0,A
;     JNC        LAB_CODE_17fd
; LAB_CODE_1812:
;     MOV        BANK1_R2,#0x0
;     MOV        temp_41,#0x1
;     SJMP       LAB_CODE_1831








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
