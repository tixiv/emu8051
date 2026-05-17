; Patched original INT1 handler. not really going to be needed.
.org 0x17de
    MOV        DPTR,#0x8000
    MOVX       A,@DPTR        ; Read data, clear interrupt
    MOV        BANK1_R1,A     ; Save result

    MOV        A,temp_41      ; state 0?
    JNZ        process_byte_isr ; no, already synced

    mov        a,BANK1_R1
    anl        a,#0xf0
    jnz        isr_return

    mov        a,#1
    mov        temp_41,a

process_byte_isr:
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
