/*
 Стартовая инциализация - очистка памяти и регистров
 USAGE: ClearAllMacro
*/
.macro ClearAllMacro
            // очищаем память
            LDI ZL, low(SRAM_START)     // адрес начала ОЗУ
            LDI ZH, high(SRAM_START)
            CLR R16                     // заполняем нулями
ClearAllMacroFlush:
            ST Z+, R16
            CPI ZH, high(RAMEND+1)
            BRNE ClearAllMacroFlush
            CPI ZL, low(RAMEND+1)
            BRNE ClearAllMacroFlush

            // очищаем регистры
            LDI ZL, 30                  // адрес старшего регистра
            CLR ZH
ClearAllMacroFlushReg:
            DEC ZL
            ST Z, ZH
            BRNE ClearAllMacroFlushReg
            CLR ZL
.endm

/*
 Инициализация стека в конце памяти
 Usage: InitStackMacro
*/
.macro InitStackMacro
            LDI R16, low(RAMEND)
            OUT SPL, R16
            LDI R16, high(RAMEND)
            OUT SPH, R16
.endm
