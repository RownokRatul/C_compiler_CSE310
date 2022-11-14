PRINTLN PROC
    ;CLEAR DX
    PUSH BP
    MOV BP, SP

    XOR DX, DX
    ;MOV AX, TEMP
    MOV AX, [BP+4]
    ADD SP, -2
    MOV CX, 10D
    MOV [BP-2], WORD PTR 0
    ;IF NEGATIVE
        CMP AX, 0
        JGE _WHILE_OUT
        ;PRUNT MINUS SIGN
        PUSH AX
        MOV DL, 02DH
        MOV AH, 2
        INT 21H
        POP AX
        NEG AX
    _WHILE_OUT:
        ;DX:AX -> NUM / 10
        XOR DX, DX
        DIV CX
        INC [BP-2]
        ;CONVERT ASCII AND PRINT
        OR DL, 030H
        PUSH DX
        ;IF QUOTIENT IS ZERO, EXIT LOOP
            CMP AX, 0
            JE END_WHILE_OUT
        JMP _WHILE_OUT

    END_WHILE_OUT:
    ;PRINT REVERSE FROM STACK
    MOV CX, [BP-2]
    REVERSE_PRINT:
        MOV AH, 2
        POP DX
        INT 21H
        LOOP REVERSE_PRINT
    ;POP AND RETURN
    MOV AH, 2
    MOV DL, 0AH
    INT 21H
    MOV DL, 0DH
    INT 21H
    MOV SP, BP
    POP BP
    RET 2
PRINTLN ENDP
