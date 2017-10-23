/*
 ��������� ������������ - ������� ������ � ���������
 USAGE: ClearAllMacro
*/
.macro ClearAllMacro
            // ������� ������
            LDI ZL, low(SRAM_START)     // ����� ������ ���
            LDI ZH, high(SRAM_START)
            CLR R16                     // ��������� ������
ClearAllMacroFlush:
            ST Z+, R16
            CPI ZH, high(RAMEND+1)
            BRNE ClearAllMacroFlush
            CPI ZL, low(RAMEND+1)
            BRNE ClearAllMacroFlush

            // ������� ��������
            LDI ZL, 30                  // ����� �������� ��������
            CLR ZH
ClearAllMacroFlushReg:
            DEC ZL
            ST Z, ZH
            BRNE ClearAllMacroFlushReg
            CLR ZL
.endm

/*
 ������������� ����� � ����� ������
 Usage: InitStackMacro
*/
.macro InitStackMacro
            LDI R16, low(RAMEND)
            OUT SPL, R16
            LDI R16, high(RAMEND)
            OUT SPH, R16
.endm