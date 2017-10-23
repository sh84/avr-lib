/*
 Формат буфера:
 1: read pos
 2: write pos
 3: size
 3-3+size: buff

 Пример объявления:
 buff:   .byte SIZE+3
 ..
 RingBufferInitMacro buff, SIZE
*/


/*
 Иницилизация буфера
 USAGE: RingBufferInitMacro buff, SIZE
 TMPR: Y, R16
*/
.macro RingBufferInitMacro
            LDI YH, high(@0)
            LDI YL, low(@0)
            LDI R16, @1
            STD Y+2, R16
.endm

/*
 Чтение байта из буфера
 Y - буфер, R20 - результат
 Если буфер пуст R20 не меняется, выставляется флаг T
 Если буфер не пуст - флаг T сбрасывается
*/
RingBufferRead:
            PUSHF_17
            CLI
            LD R16, Y    // read pos
            LDD R17, Y+1 // write pos
            // Если read pos == write pos - буфер пуст, выходим
            CP R16, R17
            BREQ ringBufferReadExitWithT
            PUSHY
            // Y <- red pos addr
            CLR R17
            ADD YL, R16
            ADC YH, R17
            ADIW Y, 3
            LD R20, Y // результат
            // read pos ++
            INC R16
            // проверяем что не достигли конец буфера, red pos = 0 если достигли
            POPY
            LDD R17, Y+2  // size
            CP R16, R17
            BRNE ringBufferReadExit
            CLR R16
ringBufferReadExit:
            // Y <- read var addr
            ST Y, R16
            POPF_17
            CLT
            RET
ringBufferReadExitWithT:
            POPF_17
            SET
            RET
/*
 Запись байта в буфер
 Y - буфер, R20 - байт для записи
 Если буфер переполнен, свигается вперед указатель чтения, выставляется флаг T
 Если буфер не переполнен - флаг T сбрасывается
*/
RingBufferWrite:
            PUSHF_17
            PUSHY
            CLI
            LDD R16, Y+1 // write pos
            // write pos addr -> Y
            CLR R17
            ADD YL, R16
            ADC YH, R17
            ADIW Y, 3
            ST Y, R20 // записываем байт
            POPY
            // write pos ++
            INC R16
            // проверяем что не достигли конец буфера, write pos = 0 если достигли
            LDD R17, Y+2 // size
            CP R16, R17
            BRNE ringBufferSetWrite
            CLR R16
ringBufferSetWrite:
            STD Y+1, R16
            LD R17, Y // read pos
            // Если read pos == write pos - буфер заполнен
            CP R16, R17
            BRNE ringBufferWriteExit
            // смещаем вперед red pos
            INC R17
            // проверяем что не достигли конец буфера, red pos = 0 если достигли
            LDD R16, Y+2 // size
            CP R17, R16
            BRNE ringBufferSetReadOverload
            // red pos = 0 если достигли конец буфера
            CLR R17
ringBufferSetReadOverload:
            ST Y, R17
            POPF_17
            SET
            RET
ringBufferWriteExit:
            POPF_17
            CLT
            RET

/*
 Запись строки оканчивающейся на \0 из флеш памяти в буфер
 Y - буфер, Z - адрес строки
 Флаг T выставляется обычным образом
*/
RingBufferLPM:
            PUSHF
            PUSH R20
ringBufferLPMLoop:
            LPM R20, Z+
            CPI R20, 0
            BREQ ringBufferLPMExit
            RCALL RingBufferWrite
            RJMP ringBufferLPMLoop
ringBufferLPMExit:
            POP R20
            POPFT
            RET

/*
 Копируем из одного буфера в другой
 Y - входной буфер, Z - выходной
 TODO: предусмотреть случай нулевого буфера
*/
#ifdef RingBufferCopyLoopProc
RingBufferCopy:
.def in_buff_read_pos   = R16
.def in_buff_write_pos  = R17
.def in_buff_size       = R18
.def out_buff_read_pos  = R19
.def out_buff_write_pos = R20
.def out_buff_size      = R21
.def tmp                = R22
            PUSHF_17_18_19_20_21_22
            PUSHY
            PUSHZ
            CLI
            LD in_buff_read_pos,   Y+
            LD in_buff_write_pos,  Y+
            LD in_buff_size,       Y+
            LD out_buff_read_pos,  Z+
            LD out_buff_write_pos, Z+
            LD out_buff_size,      Z+
            // Y <- in buff red pos addr
            CLR tmp
            ADD YL, in_buff_read_pos
            ADC YH, tmp
            // Z <- out buff write pos addr
            ADD ZL, out_buff_write_pos
            ADC ZH, tmp
ringBufferCopyLoop:
            // Если read pos == write pos - входной буфер пуст, выходим
            CP in_buff_read_pos, in_buff_write_pos
            BREQ ringBufferCopyExit
            LD tmp, Y+
            ST Z+, tmp
            // pos ++
            INC in_buff_read_pos
            INC out_buff_write_pos
            // проверяем что не достигли конец буфера
            CP in_buff_read_pos, in_buff_size
            BRNE ringBufferCopyLoop1
            CLR tmp
            SUB YL, in_buff_read_pos
            SBC YH, tmp
            CLR in_buff_read_pos
ringBufferCopyLoop1:
            CP out_buff_write_pos, out_buff_size
            BRNE ringBufferCopyLoop2
            CLR tmp
            SUB ZL, out_buff_write_pos
            SBC ZH, tmp
            CLR out_buff_write_pos
ringBufferCopyLoop2:
            // Если read pos == write pos - буфер заполнен
            CP out_buff_write_pos, out_buff_read_pos
            BRNE ringBufferCopyLoop
            // смещаем вперед red pos
            INC out_buff_read_pos
            CP out_buff_read_pos, out_buff_size
            BRNE ringBufferCopyLoop
            // red pos = 0 если достигли конец буфера
            CLR tmp
            SUB YL, in_buff_read_pos
            SBC YH, tmp
            CLR in_buff_read_pos
            RJMP ringBufferCopyLoop
ringBufferCopyExit:
            POPZ
            POPY
            ST Y, in_buff_read_pos
            ST Z, out_buff_read_pos
            STD Z+1, out_buff_write_pos
            POPF_17_18_19_20_21_22
            RET
.undef in_buff_read_pos
.undef in_buff_write_pos
.undef in_buff_size
.undef out_buff_read_pos
.undef out_buff_write_pos
.undef out_buff_size
.undef tmp

/*
 Копируем из одного буфера в другой
 USAGE: RingBufferCopyMacro in_buff, out_buff
 TMPR: Y, Z
*/
.macro RingBufferCopyMacro
            LDI YH, high(@0)
            LDI YL, low(@0)
            LDI ZH, high(@1)
            LDI ZL, low(@1)
            RCALL RingBufferCopy
.endm
#endif

/* 
 Копируем из буфера в строку
 Y - буфер, Z - строка
*/
#ifdef __strings_lib
RingBufferToString:
.def in_buff_read_pos   = R16
.def in_buff_write_pos  = R17
.def in_buff_size       = R18
.def tmp                = R19
            PUSHF_17_18_19
            PUSHZ
            PUSHY
            CLI
            LD in_buff_read_pos,   Y+
            LD in_buff_write_pos,  Y+
            LD in_buff_size,       Y+
            // Y <- in buff red pos addr
            CLR tmp
            ADD YL, in_buff_read_pos
            ADC YH, tmp
RingBufferToStringLoop:
            // Если read pos == write pos - буфер пуст, выходим
            CP in_buff_read_pos, in_buff_write_pos
            BREQ RingBufferToStringEnd
            LD tmp, Y+
            ST Z+, tmp
            // pos ++
            INC in_buff_read_pos
            CP in_buff_read_pos, in_buff_size
            BRNE RingBufferToStringLoop
            // pos = 0 если достигли конец буфера
            CLR tmp
            SUB YL, in_buff_read_pos
            SBC YH, tmp
            CLR in_buff_read_pos
            RJMP RingBufferToStringLoop
RingBufferToStringEnd:
            POPY
            ST Y, in_buff_read_pos
            // завершающий 0 в строке
            CLR tmp
            ST Z, tmp
            POPZ
            POPF_17_18_19
            RET

/*
 Копируем из буфера в строку
 USAGE: RingBufferToStringMacro buff, string
 TMPR: Y, Z
*/
.macro RingBufferToStringMacro
            LDI YH, high(@0)
            LDI YL, low(@0)
            LDI ZH, high(@1)
            LDI ZL, low(@1)
            RCALL RingBufferToString
.endm
#endif

#define __ring_buffer_lib
