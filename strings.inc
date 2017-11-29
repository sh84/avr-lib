/*
 Сравнить строку с текстом во флеш памяти
 фактически строка сраниается сс куском текста такой же длинны
 Y - строка, Z - текст
 Результат в бите T: 1 если равны, 0 - иначе
 */
stringCmpToText:
.def tmp1 = R16
.def tmp2 = R17
            PUSHF_17
            SET
stringCmpToTextLoop:
            LD tmp1, Y+
            CPI tmp1, 0
            // если прочитали 0 - конец строки, выходим
            BREQ stringCmpToTextExit
            LPM tmp2, Z+
            CP tmp1, tmp2
            BREQ stringCmpToTextLoop
            // а тут мы окажемся только если символы не свопали
            CLT
stringCmpToTextExit:
            POP R17
            POPFT
            RET
.undef tmp1
.undef tmp2

/*
 Сравнить строку (@0) с текстом во флеш памяти (@1)
 фактически строка сраниается сс куском текста такой же длинны
 Результат в бите T: 1 если равны, 0 - иначе
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
