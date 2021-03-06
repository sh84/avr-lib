/*
 ������������� ������� � �������� �������, �������� ��������� uartInit
 USAGE: UartInitMscro in_buff, out_buff, SIZE, UARTRATE
 TMPR: Y, R16
*/
.macro UartInitMacro
            .equ _uart_in_buff = @0
            .equ _uart_out_buff = @1
            RingBufferInitMacro _uart_in_buff, @2
            RingBufferInitMacro _uart_out_buff, @2
            .equ UART_UBBR = CLOCK_FREQUENCY / (8 * @3) - 1
            RCALL uartInit
.endm

/*
 ���������� � �������� ����� ������ @0 �� ���� ������
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
 ���������� � �������� ����� ������ �� R20
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
 ���������� � �������� ����� ������ @0
 USAGE: UartWriteMacro "s"
 TMPR: R16
*/
.macro UartWriteMacroI
            LDI R20, @0
            UartWriteMacro
.endm

/*
 ���������� � uart ������� ���������� ��������� ������
 USAGE: UartSend
 TMPR: R16
*/
.macro UartSendMacro
            OUTI UCSR0B, 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0 | 1<<TXCIE0 | 1<<UDRIE0 // �������� ���������� UDRIE
.endm

/*
 ���������� � �������� ����� ������ TEXT, "\n" � ��������� ��������
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
            OUTI UBRR0H, High(UART_UBBR)                  // ��������� �������� ��������
            OUTI UBRR0L, Low(UART_UBBR)
            OUTI UCSR0A, 1<<U2X0                          // �������� ��������
            OUTI UCSR0B, 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0 | 0<<TXCIE0 | 1<<UDRIE0 // �������� ������ TX � RX, ���������� �� ����������� �����
            OUTI UCSR0C, 1<<UCSZ00|1<<UCSZ01              // ����������� �����, �������� ����, 1 ���� ���, 8 ���
            RET

/*
 ���������� UDREaddr
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
            OUTI UCSR0B, 1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0 | 0<<TXCIE0 | 0<<UDRIE0 // ��������� ���������� UDRIE
uartDataRegisterExit:
            POPY
            POP R20
            POPF
            RETI

/*
 ���������� URXCaddr 
*/
uartRxCompleteHandler:
            PUSHF
            PUSH R20
            PUSHY
            LDI YH, high(_uart_in_buff)
            LDI YL, low(_uart_in_buff)
            INR R20, UDR0
            // ��������� ��� ������ ������ �������� ������
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
            // �������� �� uart_in_buff � uart_out_buff
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

/*
  �������� ������
  � Y ����� ������ � ���� ������ � �������
  � Z 2 ����� �������������� ���������� �� ������
*/
uartSendError:
            PUSH R20
            PUSH R21
            PUSH ZL
            PUSH ZH
            MOVW Z, Y
            LDI YH, high(_uart_out_buff)
            LDI YL, low(_uart_out_buff)
            RCALL RingBufferLPM
            LDI ZH, high(UART_ERROR_POSTFIX_TEXT*2)
            LDI ZL, low(UART_ERROR_POSTFIX_TEXT*2)
            RCALL RingBufferLPM
            POP R16
            RCALL hexToAscii
            RCALL RingBufferWrite
            MOV R20, R21
            RCALL RingBufferWrite
            POP R16
            RCALL hexToAscii
            RCALL RingBufferWrite
            MOV R20, R21
            RCALL RingBufferWrite
            LDI R20, '\n'
            RCALL RingBufferWrite
            UartSendMacro
            POP R21
            POP R20
            RET

UART_ERROR_POSTFIX_TEXT: .db ", code: ", 0, 0


#define __uart_lib
