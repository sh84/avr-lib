/*
 ������������� OUT - ����� �������� � ����
 USAGE: OUTI PORTD, 0xff
 TMPR: R16
*/
.macro OUTI
            LDI R16, @1
        .if @0 < 0x40
            OUT @0, R16
        .else
            STS @0, R16
        .endif
.endm

/*
 ������������� OUT - ����� �������� � ����
 USAGE:  OUTR PORTD, R16
*/
.macro OUTR
        .if @0 < 0x40
            OUT @0, @1
        .else
            STS @0, @1
        .endif
.endm

/*
 ������������� IN- ���� �������� ����� �������
 USAGE: INR R16, PORTD
*/
.macro INR
        .if @1 < 0x40
            IN @0, @1
        .else
            LDS @0, @1
        .endif
.endm

/*
 ������������� SBI - ��������� ���� �����
 USAGE:  SBI PORTD, 7
 TMPR: R16
*/
.macro SETBI
        .if @0 < 0x20
            SBI @0, @1
        .else
            INR R16, @0
            ORI R16, 1<<@1
            OUTR @0, R16
        .endif
.endm

/*
 ������������� CBI - ����� ���� �����
 USAGE: CLRBI PORTD, 7
 TMPR: R16
 SREG: S,V,N,Z
*/
.macro CLRBI
        .if @0 < 0x20
            CBI @0, @1
        .else
            INR R16, @0
            ANDI R16, ~(1<<@1)
            OUTR @0, R16
        .endif
.endm

/*
 �������� ���� �����
 USAGE: INVBI PORTD, 7
 TMPR: R16, R17
 SREG: S,V,N,Z
*/
.macro INVBI
            INR R16, @0
            LDI R17, 1<<@1
            EOR R16, R17
            OUTR @0, R16
.endm

/*
 LDI + STS
 USAGE: STSI addr, VALUE
 TMPR: R16
*/
.macro STSI
            LDI R16, @1
            STS @0, R16
.endm


/*
 PUSH ��� SREG � R16
 USAGE: PUSHF
*/
.macro PUSHF
            PUSH R16
            IN	R16, SREG
            PUSH R16
.endm

/*
 POP ��� SREG � R16
 USAGE: POPF
*/
.macro POPF
            POP R16
            OUT SREG, R16
            POP R16
.endm

/*
 POP ��� SREG � R16, ��������� ��������� ����� T
 USAGE: POPFT
*/
.macro POPFT
            POP R16
            BLD R16, 6
            OUT SREG, R16
            POP R16
.endm

/* ������� �� ������ ���������� ��������� PUSH/POP */

.macro PUSHX
            PUSH XH
            PUSH XL
.endm
.macro POPX
            POP XL
            POP XH
.endm
.macro PUSHY
            PUSH YH
            PUSH YL
.endm
.macro POPY
            POP YL
            POP YH
.endm
.macro PUSHZ
            PUSH ZH
            PUSH ZL
.endm
.macro POPZ
            POP ZL
            POP ZH
.endm

.macro PUSHF_17
            PUSHF
            PUSH R17
.endm
.macro POPF_17
            POP R17
            POPF
.endm
.macro POPFT_17
            POP R17
            POPFT
.endm
.macro PUSHF_17_18
            PUSHF
            PUSH R17
            PUSH R18
.endm
.macro POPF_17_18
            POP R18
            POP R17
            POPF
.endm
.macro POPFT_17_18
            POP R18
            POP R17
            POPFT
.endm
.macro PUSHF_17_18_19
            PUSHF
            PUSH R17
            PUSH R18
            PUSH R19
.endm
.macro POPF_17_18_19
            POP R19
            POP R18
            POP R17
            POPF
.endm
.macro POPFT_17_18_19
            POP R19
            POP R18
            POP R17
            POPFT
.endm
.macro PUSHF_17_18_19_20
            PUSHF
            PUSH R17
            PUSH R18
            PUSH R19
            PUSH R20
.endm
.macro POPF_17_18_19_20
            POP R20
            POP R19
            POP R18
            POP R17
            POPF
.endm
.macro POPFT_17_18_19_20
            POP R20
            POP R19
            POP R18
            POP R17
            POPFT
.endm
.macro PUSHF_17_18_19_20_21
            PUSHF
            PUSH R17
            PUSH R18
            PUSH R19
            PUSH R20
            PUSH R21
.endm
.macro POPF_17_18_19_20_21
            POP R21
            POP R20
            POP R19
            POP R18
            POP R17
            POPF
.endm
.macro POPFT_17_18_19_20_21
            POP R21
            POP R20
            POP R19
            POP R18
            POP R17
            POPFT
.endm
.macro PUSHF_17_18_19_20_21_22
            PUSHF
            PUSH R17
            PUSH R18
            PUSH R19
            PUSH R20
            PUSH R21
            PUSH R22
.endm
.macro POPF_17_18_19_20_21_22
            POP R22
            POP R21
            POP R20
            POP R19
            POP R18
            POP R17
            POPF
.endm
.macro POPFT_17_18_19_20_21_22
            POP R22
            POP R21
            POP R20
            POP R19
            POP R18
            POP R17
            POPFT
.endm
