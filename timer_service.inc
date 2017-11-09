/*
 »спользуетс€ (и не должен больше никем мен€тьс€) регистр R9
 “ак же необходимо 32 байта пам€ти, адрес которой передаетс€ в TimerServiceInitMacro
*/

/*
 »нициализирум сервис таймеров
 USAGE: TimerServiceInitMacro handlers_data
 TMPR: R16
*/
.macro TimerServiceInitMacro
            .equ _timer_service_data = @0
            OUTI TIMSK0, 1<<OCIE0A       // включаем прерывани€ по совпадению с OSR0A
            OUTI TCCR0A, 1<<WGM01        // сброс по совпадению с OSR0A
            .if CLOCK_FREQUENCY <= 2000000
            OUTI OCR0A, CLOCK_FREQUENCY / 8 / 1000
            OUTI TCCR0B, 1<<CS01             // делитель 1/8
            .elif CLOCK_FREQUENCY <= 16000000
            OUTI OCR0A, CLOCK_FREQUENCY / 64 / 1000
            OUTI TCCR0B, 1<<CS01 | 1<<CS00   // делитель 1/64
            .else
            OUTI OCR0A, CLOCK_FREQUENCY / 256 / 1000
            OUTI TCCR0B, 1<<CS02             // делитель 1/256
            .endif
            
.endm

/*
 USAGE: TimerServiceAddTaskMacro task_addr TIMEOUT
 TIMEOUT в мс, не более 65535
 TMPR: Y, Z
*/
.macro TimerServiceAddTaskMacro
            .if @1 > 65535
            .error "TIMEOUT max value is 65535!"
            .endif
            LDI YH, high(@1)
            LDI YL, low(@1)
            LDI ZH, high(@0)
            LDI ZL, low(@0)
            RCALL timerServiceAddTask
.endm

timerServiceAddTask:
            PUSHF
            // провер€ем что есть свободный слот, иначе - логгируем ошибку, или рестартимс€
            MOV R16, R9
            CPI R16, 0xff
            BREQ timerServiceAddTaskError
            // ищем свободный слот дл€ обработчика
            PUSH R17
            PUSHX
            // загружаем в X адрес 0 слота
            LDI XH, high(_timer_service_data)
            LDI XL, low(_timer_service_data)
            LDI R17, 1 // битова€ маска€ дл€ проверки - 1 бит в провер€емом разр€де
timerServiceAddTaskFindLoop:
            MOV R16, R17
            AND R16, R9
            BREQ timerServiceAddTaskFindFinish
            LSL R17
            ADIW X, 4
            RJMP timerServiceAddTaskFindLoop
timerServiceAddTaskFindFinish:
            // записываем в R9 что найденный слот зан€т
            OR R9, R17
            // по адресу X и X+1 пишем значение счетчика
            ST X+, YH
            ST X+, YL
            // по адресу X+2 и X+3 пишем адресс обработчика
            ST X+, ZH
            ST X+, ZL
            POPX
            POP R17
            POPF
            RET
timerServiceAddTaskError:
            LDI YH, High(timerServiceAddTaskErrorTEXT*2)
            LDI YL, low(timerServiceAddTaskErrorTEXT*2)
            RCALL error
            POPF
            RET
timerServiceAddTaskErrorTEXT: .db "timer add no slots", 0, 0

// обработчик прерывани€ таймера
timerServiceCompareA:
            PUSHF
            // провер€ем что есть хоть 1 зан€тый слот, иначе - выходим
            TST R9
            BREQ timerServiceCompareAExit
            // ищем заполненный слот дл€ обработчика
            PUSH R17
            PUSHY
            PUSHZ
            // загружаем в Y адрес 0 слота
            LDI YH, high(_timer_service_data)
            LDI YL, low(_timer_service_data)
            LDI R17, 1 // битова€ маска€ дл€ проверки - 1 бит в провер€емом разр€де
timerServiceCompareAFindLoop:
            MOV R16, R17
            AND R16, R9
            BREQ timerServiceCompareANotFound
            // достаем счетчик
            LD ZH, Y
            LDD ZL, Y+1
            // декрементим его и сохран€ем
            SBIW Z, 1
            ST Y, ZH
            STD Y+1, ZL
            BRNE timerServiceCompareANotFound
            // тут счетчик == 0
            // сбрасываем соотв. бит в R9
            MOV R16, R17
            COM R16
            AND R9, R16
            // грузим адресс и переходим по нему
            LDD ZH, Y+2
            LDD ZL, Y+3
            PUSH R17
            PUSHY
            ICALL
            POPY
            POP R17
timerServiceCompareANotFound:
            // смещаемс€ к нужному слоту
            ADIW Y, 4
            LSL R17
            // проверка на конец цикла - бит сдвига перейдет в флаг —
            BRCC timerServiceCompareAFindLoop
            POPZ
            POPY
            POP R17
timerServiceCompareAExit:
            POPF
            RETI

#define __timer_service_lib
