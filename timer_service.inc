/*
 ������������ (� �� ������ ������ ����� ��������) ������� R9
 ��� �� ���������� 32 ����� ������, ����� ������� ���������� � TimerServiceInitMacro
*/

/*
 ������������� ������ ��������
 USAGE: TimerServiceInitMacro handlers_data
 TMPR: R16
*/
.macro TimerServiceInitMacro
            .equ _timer_service_data = @0
            OUTI TIMSK0, 1<<OCIE0A       // �������� ���������� �� ���������� � OSR0A
            OUTI TCCR0A, 1<<WGM01        // ����� �� ���������� � OSR0A
            .if CLOCK_FREQUENCY <= 2000000
            OUTI OCR0A, CLOCK_FREQUENCY / 8 / 1000
            OUTI TCCR0B, 1<<CS01             // �������� 1/8
            .elif CLOCK_FREQUENCY <= 16000000
            OUTI OCR0A, CLOCK_FREQUENCY / 64 / 1000
            OUTI TCCR0B, 1<<CS01 | 1<<CS00   // �������� 1/64
            .else
            OUTI OCR0A, CLOCK_FREQUENCY / 256 / 1000
            OUTI TCCR0B, 1<<CS02             // �������� 1/256
            .endif
            
.endm

/*
 USAGE: TimerServiceAddTaskMacro task_addr TIMEOUT
 TIMEOUT � ��, �� ����� 65535
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
            // ��������� ��� ���� ��������� ����, ����� - ��������� ������, ��� �����������
            MOV R16, R9
            CPI R16, 0xff
            BREQ timerServiceAddTaskError
            // ���� ��������� ���� ��� �����������
            PUSH R17
            PUSHX
            // ��������� � X ����� 0 �����
            LDI XH, high(_timer_service_data)
            LDI XL, low(_timer_service_data)
            LDI R17, 1 // ������� ������ ��� �������� - 1 ��� � ����������� �������
timerServiceAddTaskFindLoop:
            MOV R16, R17
            AND R16, R9
            BREQ timerServiceAddTaskFindFinish
            LSL R17
            ADIW X, 4
            RJMP timerServiceAddTaskFindLoop
timerServiceAddTaskFindFinish:
            // ���������� � R9 ��� ��������� ���� �����
            OR R9, R17
            // �� ������ X � X+1 ����� �������� ��������
            ST X+, YH
            ST X+, YL
            // �� ������ X+2 � X+3 ����� ������ �����������
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

// ���������� ���������� �������
timerServiceCompareA:
            PUSHF
            // ��������� ��� ���� ���� 1 ������� ����, ����� - �������
            TST R9
            BREQ timerServiceCompareAExit
            // ���� ����������� ���� ��� �����������
            PUSH R17
            PUSHY
            PUSHZ
            // ��������� � Y ����� 0 �����
            LDI YH, high(_timer_service_data)
            LDI YL, low(_timer_service_data)
            LDI R17, 1 // ������� ������ ��� �������� - 1 ��� � ����������� �������
timerServiceCompareAFindLoop:
            MOV R16, R17
            AND R16, R9
            BREQ timerServiceCompareANotFound
            // ������� �������
            LD ZH, Y
            LDD ZL, Y+1
            // ����������� ��� � ���������
            SBIW Z, 1
            ST Y, ZH
            STD Y+1, ZL
            BRNE timerServiceCompareANotFound
            // ��� ������� == 0
            // ���������� �����. ��� � R9
            MOV R16, R17
            COM R16
            AND R9, R16
            // ������ ������ � ��������� �� ����
            LDD ZH, Y+2
            LDD ZL, Y+3
            PUSH R17
            PUSHY
            ICALL
            POPY
            POP R17
timerServiceCompareANotFound:
            // ��������� � ������� �����
            ADIW Y, 4
            LSL R17
            // �������� �� ����� ����� - ��� ������ �������� � ���� �
            BRCC timerServiceCompareAFindLoop
            POPZ
            POPY
            POP R17
timerServiceCompareAExit:
            POPF
            RETI

#define __timer_service_lib
