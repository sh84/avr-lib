/*
 ������ ������:
 1: read pos
 2: write pos
 3: size
 3-3+size: buff

 ������ ����������:
 buff:   .byte SIZE+3
 ..
 RingBufferInitMacro buff, SIZE
*/


/*
 ������������ ������
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
 ������ ����� �� ������
 Y - �����, R20 - ���������
 ���� ����� ���� R20 �� ��������, ������������ ���� T
 ���� ����� �� ���� - ���� T ������������
*/
RingBufferRead:
            PUSHF_17
            CLI
            LD R16, Y    // read pos
            LDD R17, Y+1 // write pos
            // ���� read pos == write pos - ����� ����, �������
            CP R16, R17
            BREQ ringBufferReadExitWithT
            PUSHY
            // Y <- red pos addr
            CLR R17
            ADD YL, R16
            ADC YH, R17
            ADIW Y, 3
            LD R20, Y // ���������
            // read pos ++
            INC R16
            // ��������� ��� �� �������� ����� ������, red pos = 0 ���� ��������
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
 ������ ����� � �����
 Y - �����, R20 - ���� ��� ������
 ���� ����� ����������, ��������� ������ ��������� ������, ������������ ���� T
 ���� ����� ��� �� �������� - ���� T ������������
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
            ST Y, R20 // ���������� ����
            POPY
            // write pos ++
            INC R16
            // ��������� ��� �� �������� ����� ������, write pos = 0 ���� ��������
            LDD R17, Y+2 // size
            CP R16, R17
            BRNE ringBufferSetWrite
            CLR R16
ringBufferSetWrite:
            STD Y+1, R16
            LD R17, Y // read pos
            // ���� read pos == write pos - ����� ��������
            CP R16, R17
            BRNE ringBufferWriteExit
            // ������� ������ red pos
            INC R17
            // ��������� ��� �� �������� ����� ������, red pos = 0 ���� ��������
            LDD R16, Y+2 // size
            CP R17, R16
            BRNE ringBufferSetReadOverload
            // red pos = 0 ���� �������� ����� ������
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
 ������ ������ �������������� �� \0 �� ���� ������ � �����
 Y - �����, Z - ����� ������
 ���� T ������������ ������� �������
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
 �������� �� ������ ������ � ������
 Y - ������� �����, Z - ��������
 TODO: ������������� ������ �������� ������
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
            // ���� read pos == write pos - ������� ����� ����, �������
            CP in_buff_read_pos, in_buff_write_pos
            BREQ ringBufferCopyExit
            LD tmp, Y+
            ST Z+, tmp
            // pos ++
            INC in_buff_read_pos
            INC out_buff_write_pos
            // ��������� ��� �� �������� ����� ������
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
            // ���� read pos == write pos - ����� ��������
            CP out_buff_write_pos, out_buff_read_pos
            BRNE ringBufferCopyLoop
            // ������� ������ red pos
            INC out_buff_read_pos
            CP out_buff_read_pos, out_buff_size
            BRNE ringBufferCopyLoop
            // red pos = 0 ���� �������� ����� ������
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
 �������� �� ������ ������ � ������
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
 �������� �� ������ � ������
 Y - �����, Z - ������
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
            // ���� read pos == write pos - ����� ����, �������
            CP in_buff_read_pos, in_buff_write_pos
            BREQ RingBufferToStringEnd
            LD tmp, Y+
            ST Z+, tmp
            // pos ++
            INC in_buff_read_pos
            CP in_buff_read_pos, in_buff_size
            BRNE RingBufferToStringLoop
            // pos = 0 ���� �������� ����� ������
            CLR tmp
            SUB YL, in_buff_read_pos
            SBC YH, tmp
            CLR in_buff_read_pos
            RJMP RingBufferToStringLoop
RingBufferToStringEnd:
            POPY
            ST Y, in_buff_read_pos
            // ����������� 0 � ������
            CLR tmp
            ST Z, tmp
            POPZ
            POPF_17_18_19
            RET
.undef in_buff_read_pos
.undef in_buff_write_pos
.undef in_buff_size
.undef tmp

/*
 �������� �� ������ � ������
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
