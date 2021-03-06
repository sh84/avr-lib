/*
 �������� ������ � ������� �� ���� ������
 ���������� ������ ���������� �� ������ ������ ����� �� ������
 Y - ������, Z - �����
 ��������� � ���� T: 1 ���� �����, 0 - �����
 */
stringCmpToText:
.def tmp1 = R16
.def tmp2 = R17
            PUSHF_17
            SET
stringCmpToTextLoop:
            LD tmp1, Y+
            CPI tmp1, 0
            // ���� ��������� 0 - ����� ������, �������
            BREQ stringCmpToTextExit
            LPM tmp2, Z+
            CP tmp1, tmp2
            BREQ stringCmpToTextLoop
            // � ��� �� �������� ������ ���� ������� �� �������
            CLT
stringCmpToTextExit:
            POP R17
            POPFT
            RET
.undef tmp1
.undef tmp2

/*
 �������� ������ (@0) � ������� �� ���� ������ (@1)
 ���������� ������ ���������� �� ������ ������ ����� �� ������
 ��������� � ���� T: 1 ���� �����, 0 - �����
 USAGE: StringCmpToTextMacro string, TEXT
 TMPR: Y, Z
 */
.macro StringCmpToTextMacro
            LDI YH, high(@0)
            LDI YL, low(@0)
            LDI ZH, high(@1*2)
            LDI ZL, low(@1*2)
            RCALL stringCmpToText
.endm

hexToAscii:
            PUSH R16
            PUSH R16
            RCALL hexToAsciiHalf
            MOV R21, R16
            POP R16
            SWAP R16
            RCALL hexToAsciiHalf
            MOV R20, R16
            POP R16
            RET
hexToAsciiHalf:
            ANDI R16, 0x0f
            CPI R16, 10
            BRLO hexToAsciiNumber
            SUBI R16, -0x07
hexToAsciiNumber:
            SUBI R16, -0x30
            RET

#define __strings_lib
