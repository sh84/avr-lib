.equ TWI_PORT = PORTC
.equ TWI_DDR = DDRC
.equ TWI_SCL = 5          // ����� ������ � TWI_PORT ��� SCL
.equ TWI_SDA = 4          // ����� ������ � TWI_PORT ��� SDA
.equ TWI_RETRY = 5        // ���������� �������� ��� ������� (max 64)
.equ TWI_RETRY_DELAY = 10 // ����� ����� ���������, ��

.macro TWI_DEBUG_MACRO
            PUSHY
            LDI YH, High(twiDebugDebugTEXT*2)
            LDI YL, low(twiDebugDebugTEXT*2)
            INR ZH, TWSR
            INR ZL, TWCR
            RCALL error
            POPY
.endm
twiDebugDebugTEXT: .db "TWI", 0

/*
 �������� ��������� twiInitMacro
 USAGE: TwiInitMacro twi_data TWIRATE
 twi_data - 12 ���� � ������ ��� ���������� ����
*/
.macro TwiInitMacro
            .equ TWI_TWBR = (CLOCK_FREQUENCY / @1 - 16) / 2
            .if TWI_TWBR > 255
            .error "TWI rate too low!"
            .endif
            .equ _twi_sla_rw = @0
            .equ _twi_read_addr = @0+1
            .equ _twi_read_size = @0+3
            .equ _twi_read_pos = @0+4
            .equ _twi_write_addr = @0+5
            .equ _twi_write_size = @0+7
            .equ _twi_write_pos = @0+8
            .equ _twi_retries = @0+9    // ������� ��� - ������� ��������� twi
            .equ _twi_callback = @0+10
            RCALL twiInit
.endm

twiInit:
            PUSHF
            OUTI TWBR, TWI_TWBR               // ��������� �������� ��������
            // ����������� ������ SCL � SDA
            CLRBI TWI_DDR, TWI_SCL
            SETBI TWI_PORT, TWI_SCL
            CLRBI TWI_DDR, TWI_SDA
            SETBI TWI_PORT, TWI_SDA
            POPF
            RET

/*
 ���� TWI ���� ����� - ���������� ���� T, ���� - ������������.
*/
twiBusyCheck:
            PUSHF
            CLI
            CLT
            LDS R16, _twi_retries
            SBRC R16, 7
            SET
            POPFT
            RET

/*
 ��������� � TWI ���������� � ������� @0 @1 ���� �� ������ @2,
 ��������� @3 ���� �� ������ @4 � ������� callback @5
 ���� TWI ���� ����� - ���������� ���� T (� ������ �� ��������), ���� - ������������.
 USAGE: twiWriteReadWithCallbackMacro 0xa, 2, write_buff, 2, read_buff, callback
 USAGE: twiWriteReadWithCallbackMacro 0xa, 0, 0, 2, read_buff, callback - ������ ������
 USAGE: twiWriteReadWithCallbackMacro 0xa, 2, write_buff, 0, 0, 0 - ������ ������, ��� callback-�
 TMPR: R16
*/
.macro twiWriteReadWithCallbackMacro
            PUSHF
            CLI
            RCALL twiBusyCheck
            BRTS twiWriteReadWithCallbackEnd
            STSI _twi_sla_rw, @0
            STSI _twi_write_size, @1
            STSI _twi_write_addr, high(@2)
            STSI _twi_write_addr+1, low(@2)
            STSI _twi_read_size, @3
            STSI _twi_read_addr, high(@4)
            STSI _twi_read_addr+1, low(@4)
            STSI _twi_callback, high(@5)
            STSI _twi_callback+1, low(@5)
            STSI _twi_retries, 0x80   // �������� retry ������� � ������ ��� ���������
            CLR R16
            STS _twi_write_pos, R16
            STS _twi_read_pos, R16
            // ���������� START
            OUTI TWCR, 1<<TWINT | 1<<TWSTA | 1<<TWEN | 1 << TWIE
twiWriteReadWithCallbackEnd:
            POPFT
.endm

twiHandler:
            PUSHF_17
            PUSHZ
//            TWI_DEBUG_MACRO
// �� ������������
/*INC R1
MOV R16, R1
CPI R16, 33
BRSH twiHandlerExitStop
*/

            LDI ZH, high(twiHandlerVectors)
            LDI ZL, low(twiHandlerVectors)
            INR R16, TWSR
            // ���� TWSR >= 0x60, ������������ �� ���������� 00 - twiHandlerError
            CPI R16, 0x60
            BRSH twiHandlerCall
            LSR R16
            LSR R16
            LSR R16
            ADD ZL, R16
            CLR R16
            ADC ZH, R16
twiHandlerCall:
            ICALL
twiHandlerExit:
            POPZ
            POPF_17
            RETI

/*
twiHandlerExitStop:
            OUTI TWCR, 1<<TWINT | 1<<TWSTO | 1<<TWEN | 1 << TWIE
            POPZ
            POPF
            RETI
*/
twiHandlerVectors:
            // 00 - Bus error due to an illegal START or STOP condition
            RJMP twiHandlerError
            // 08 - A START condition has been transmitted
            RJMP twiHandler08
            // 10 - A repeated START condition has been transmitted
            RJMP twiHandler08
            // 18 - SLA+W has been transmitted; ACK has been received
            RJMP twiHandler18
            // 20 - SLA+W has been transmitted; NOT ACK has been received
            RJMP twiHandlerRetry
            // 28 - Data byte has been transmitted; ACK has been received
            RJMP twiHandler28
            // 30 - Data byte has been transmitted; NOT ACK has been received
            RJMP twiHandlerRetry
            // 38 - Arbitration lost in SLA+W or data bytes, Arbitration lost in SLA+R or NOT ACK bit
            RJMP twiHandlerRetry
            // 40 - SLA+R has been transmitted; ACK has been received
            RJMP twiHandler40
            // 48 - SLA+R has been transmitted; NOT ACK has been received
            RJMP twiHandlerRetry
            // 50 - Data byte has been received; ACK has been returned
            RJMP twiHandler50
            // 58 - Data byte has been received; NOT ACK has been returned
            RJMP twiHandler58

twiHandler08:
            // A START condition has been transmitted
            // A repeated START condition has been transmitted
            LDS R16, _twi_sla_rw
            // ��������� ��� ������ �� ����� ��� ��������
            LDS ZH, _twi_write_size
            LDS ZL, _twi_write_pos
            TST ZH
            BREQ twiHandler08makeRead
            CP ZL, ZH
            BRSH twiHandler08makeRead
            CBR R16, 1 // ���������� 0 ��� ��� ���� ��������
            RJMP twiHandler08end
twiHandler08makeRead:
            SBR R16, 1 // ������ 0 ��� ��� ���� ������
twiHandler08end:
            OUTR TWDR, R16
            OUTI TWCR, 1<<TWINT | 1<<TWEN | 1 << TWIE
            RET

twiHandler18:
            // SLA+W has been transmitted; ACK has been received
            // ��������� � �������� � Z ����� �������� ����� ��� ������
            LDS ZH, _twi_write_addr
            LDS ZL, _twi_write_addr+1
            LDS R16, _twi_write_pos
twiHandler18Send:
            ADD ZL, R16
            CLR R17
            ADC ZH, R17
            // ���������� �������
            INC R16
            STS _twi_write_pos, R16
            // ���������� ����
            LD R16, Z
            OUTR TWDR, R16
            OUTI TWCR, 1<<TWINT | 1<<TWEN | 1 << TWIE
            RET

twiHandler28:
            // Data byte has been transmitted; ACK has been received
            // ��������� �� �������� �� �����, ���� ��� - ���������� ��. ����
            LDS ZH, _twi_write_addr
            LDS ZL, _twi_write_addr+1
            LDS R16, _twi_write_pos
            LDS R17, _twi_write_size
            CP R16, R17
            BRLO twiHandler18Send
            // ��������� �� ���� �� ������ ���������
            LDS R16, _twi_read_size
            TST R16
            BREQ twiHandlerEnd
            // ������ ���� ���� START
            OUTI TWCR, 1<<TWINT | 1<<TWSTA | 1<<TWEN | 1 << TWIE
            RET
twiHandlerEnd:
            // ����������� ��������: ���� STOP
            OUTI TWCR, 1<<TWINT | 1<<TWSTO | 1<<TWEN | 1 << TWIE
            // �������� ������, ���� �� ����
            LDS ZH, _twi_callback
            LDS ZL, _twi_callback+1
            TST ZH
            BRNE twiHandlerEndWithCallback
            TST ZL
            BRNE twiHandlerEndWithCallback
            RJMP twiHandlerEndExit
twiHandlerEndWithCallback:
            ICALL
twiHandlerEndExit:
            CLR R16
            STS _twi_retries, R16 //���������� ���� ���������
            RET

twiHandler40:
            // SLA+R has been transmitted; ACK has been received
            // ��������� �� ���������� �� ����� ��� ������ �����
            LDS ZH, _twi_read_pos
            LDS ZL, _twi_read_size
            INC ZH
            CP ZH, ZL
            BRSH twiHandler40Last
            OUTI TWCR, 1<<TWINT | 1<<TWEA | 1<<TWEN | 1 << TWIE
            RET
twiHandler40Last:
            OUTI TWCR, 1<<TWINT | 1<<TWEN | 1 << TWIE
            RET

twiHandler50:
            // Data byte has been received; ACK has been returned
            RCALL twiHandlerRead
            RJMP twiHandler40

twiHandler58:
            // Data byte has been received; NOT ACK has been returned
            RCALL twiHandlerRead
            RJMP twiHandlerEnd

twiHandlerRead:
            // ��������� ����� ������
            LDS ZH, _twi_read_addr
            LDS ZL, _twi_read_addr+1
            LDS R16, _twi_read_pos
            CLR R17
            ADD ZL, R16
            ADC ZH, R17
            // �������� � ���������� ����
            INR R17, TWDR
            ST Z, R17
            // ����������� ������� ������
            INC R16
            STS _twi_read_pos, R16
            RET

twiHandlerError:
            // ����� ������ ���������
            PUSHY
            LDI YH, High(twiHandlerErrorTEXT*2)
            LDI YL, low(twiHandlerErrorTEXT*2)
            INR ZH, TWSR
            INR ZL, TWCR
            RCALL error
            POPY
            RJMP twiHandlerRetry
twiHandlerErrorTEXT: .db "TWI erorr", 0

twiHandlerRetry:
            PUSHY
            INR ZH, TWSR
            INR ZL, TWCR
            // ���� ����
            OUTI TWCR, 1<<TWINT | 1<<TWSTO | 1<<TWEN | 1 << TWIE
            // ����������� retry �������
            LDS R16, _twi_retries
            INC R16
            // ��������� �� ���������� �������
            CPI R16, TWI_RETRY
            BRSH twiHandlerRetryError
            // ��������� �������
            STS _twi_retries, R16
            // ������� ��� ��� ����� TWI_RETRY_DELAY ��
            TimerServiceAddTaskMacro twiHandlerRetryHandler, TWI_RETRY_DELAY
            POPY
            RET
twiHandlerRetryError:
            LDI YH, High(twiHandlerRetryErrorTEXT*2)
            LDI YL, low(twiHandlerRetryErrorTEXT*2)
            RCALL error
            POPY
            RET
twiHandlerRetryHandler: // ����� ������������ Y, Z, R16, R17
            CLR R16
            STS _twi_write_pos, R16
            STS _twi_read_pos, R16
            // ���������� START
            OUTI TWCR, 1<<TWINT | 1<<TWSTA | 1<<TWEN | 1 << TWIE
            RET
twiHandlerRetryErrorTEXT: .db "TWI retry error", 0

#define __twi_lib
