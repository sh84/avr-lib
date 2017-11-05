/*
 Инициализирум входной и выходной бфууфер, вызываем процедуру uartInit
 USAGE: UartInitMscro in_buff, out_buff, SIZE, UARTRATE
 TMPR: Y, R16
*/
.macro UartInitMacro
            .equ _uart_in_buff = @0
            .equ _uart_out_buff = @1
            RingBufferInitMacro _uart_in_buff, @2
            RingBufferInitMacro _uart_out_buff, @2
            .equ UART_UBBR = CLOCK_FREQUENCY / (16 * @3) - 1
            RCALL uartInit
.endm

/*
 Отправляем в выходной буфер строку @0 из флеш памяти
 USAGE: UartWriteTextMacro TEXT
*/
.macro UartWriteTextMacro
            PUSHY
            PUSHZ
            LDI YH, high(_uart_out_buff)
            LDI YL, low(_uart_out_buff)
            LDI ZH, high(@0*2)
            LDI ZL, low(@0*2)
            RCALL RingBufferLPM
            POPZ
            POPY
.endm

/*
 Отправляем в выходной буфер символ из R20
 USAGE: UartWriteMacro 
*/
.macro UartWriteMacro
            PUSHY
            LDI YH, high(_uart_out_buff)
            LDI YL, low(_uart_out_buff)
            RCALL RingBufferWrite
            POPY
.endm

/*
 Отправляем в выходной буфер символ @0
 USAGE: UartWriteMacro "s"
 TMPR: R16
*/
.macro UartWriteMacroI
            LDI R20, @0
            UartWriteMacro
.endm

/*
 Отправляем в uart текущее содержимое выходного буфера
 USAGE: UartSend
 TMPR: R16
*/
.macro UartSendMacro
            OUTI UCSR0B, 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0 | 1<<TXCIE0 | 1<<UDRIE0 // включаем прерывание UDRIE
.endm

/*
 Отправляем в выходной буфер строку TEXT, "\n" и запускает отправку
 USAGE: UartText TEXT
*/
.macro UartText
            PUSH R16
            PUSHY
            PUSHZ
            LDI YH, high(_uart_out_buff)
            LDI YL, low(_uart_out_buff)
            LDI ZH, high(@0*2)
            LDI ZL, low(@0*2)
            RCALL RingBufferLPM
            LDI R20, '\n'
            RCALL RingBufferWrite
            UartSendMacro
            POPZ
            POPY
            POP R16
.endm

uartInit:  
            OUTI UBRR0H, High(UART_UBBR)                  // выставяем скорость передачи
            OUTI UBRR0L, Low(UART_UBBR)
            OUTI UCSR0A, 0
            OUTI UCSR0B, 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0 | 0<<TXCIE0 | 1<<UDRIE0 // включаем выходы TX и RX, прерывание по поступлению байта
            OUTI UCSR0C, 1<<UCSZ00|1<<UCSZ01              // асинхронный режим, четность выкл, 1 стоп бит, 8 бит
            RET

/*
 прерывание UDREaddr
*/
uartDataRegisterEmptyHandler:
            PUSHF
            PUSH R20
            PUSHY
            LDI YH, high(_uart_out_buff)
            LDI YL, low(_uart_out_buff)
            RCALL RingBufferRead
            BRTS uartDataRegisterEmptyStop
            OUTR UDR0, R20
            RJMP uartDataRegisterExit
uartDataRegisterEmptyStop:
            OUTI UCSR0B, 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0 | 0<<TXCIE0 | 0<<UDRIE0 // выключаем прерывание UDRIE
uartDataRegisterExit:
            POPY
            POP R20
            POPF
            RETI

/*
 прерывание URXCaddr 
*/
uartRxCompleteHandler:
            PUSHF
            PUSH R20
            PUSHY
            LDI YH, high(_uart_in_buff)
            LDI YL, low(_uart_in_buff)
            INR R20, UDR0
            // проверяем что пришел символ перевода строки
            CPI R20, 0x0a
            BREQ uartRxCompleteEndLine
            CPI R20, 0x0d
            BREQ uartRxCompleteEndLine
            RCALL RingBufferWrite
            RJMP uartRxCompleteExit
uartRxCompleteEndLine:
            #ifdef UartCommandHandlerUsed
            RCALL uartCommandHandler
            #else
            // копируем из uart_in_buff в uart_out_buff
            LDI YH, high(_uart_in_buff)
            LDI YL, low(_uart_in_buff)
            LDI ZH, high(_uart_out_buff)
            LDI ZL, low(_uart_out_buff)
            RCALL RingBufferCopy
            LDI YH, high(_uart_out_buff)
            LDI YL, low(_uart_out_buff)
            LDI R20, '\n'
            RCALL RingBufferWrite
            UartSendMacro
            #endif
uartRxCompleteExit:
            POPY
            POP R20
            POPF
            RETI

#define __uart_lib
