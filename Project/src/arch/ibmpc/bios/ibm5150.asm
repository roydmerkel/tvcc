LOC OBJ                     LINE    SOURCE

                               1    $TITLE(BIOS FOR IBM PERSONAL COMPUTER)
                               2
                               3    ;---------------------------------------------------------------
                               4    ;       THE  BIOS ROUTINES ARE MEANT TO BE ACCESSED THROUGH    :
                               5    ;       SOFTWARE INTERRUPTS ONLY.  ANY ADDRESSES PRESENT IN    :
                               6    ;       THE LISTINGS  ARE INCLUDED  ONLY FOR  COMPLETENESS,    :
                               7    ;       NOT FOR REFERENCE.    APPLICATIONS WHICH  REFERENCE    :
                               8    ;       ABSOLUTE   ADDRESSES   WITHIN   THE  CODE   SEGMENT    :
                               9    ;       VIOLATE THE STRUCTURE AND DESIGN OF BIOS.              :
                              10    ;---------------------------------------------------------------
                              11
                              12    ;----------------------------------------
                              13    ;               EQUATES                 :
                              14    ;----------------------------------------
  0060                        15    PORT_A          EQU     60H             ; 8255 PORT A ADDR
  0061                        16    PORT_B          EQU     61H             ; 8255 PORT B ADDR
  0062                        17    PORT_C          EQU     62H             ; 8255 PORT C ADDR
  0063                        18    CMD_PORT        EQU     63H
  0020                        19    INTA00          EQU     20H             ; 8259 PORT
  0021                        20    INTA01          EQU     21H             ; 8259 PORT
  0020                        21    EOI             EQU     20H
  0040                        22    TIMER           EQU     40H
  0043                        23    TIM_CTL         EQU     43H             ; 8253 TIMER CONTROL PORT ADDR
  0040                        24    TIMER0          EQU     40H             ; 8253 TIMER/CNTER 0 PORT ADDR
  0001                        25    TMINT           EQU     01              ; TIMER 0 INTR RECVD MASK
  0008                        26    DMA08           EQU     08              ; DMA STATUS REG PORT ADDR
  0000                        27    DMA             EQU     00              ; DMA CHANNEL 0 ADDR REG PORT ADDR
  0540                        28    MAX_PERIOD      EQU     540H
  0410                        29    MIN_PERIOD      EQU     410H
  0060                        30    KBD_IN          EQU     60H             ; KEYBOARD DATA IN ADDR PORT
  0002                        31    KBDINT          EQU     02              ; KEYBOARD INTR MASK
  0060                        32    KB_DATA         EQU     60H             ; KEYBOARD SCAN CODE PORT
  0061                        33    KB_CTL          EQU     61H             ; CONTROL BITS FOR KB SENSE DATA
                              34    ;----------------------------------------
                              35    ;       8088 INTERRUPT LOCATIONS        :
                              36    ;----------------------------------------
  ----                        37    ABS0            SEGMENT AT 0
  0000                        38    STG_LOC0        LABEL   BYTE
  0008                        39                    ORG     2*4
  0008                        40    NMI_PTR         LABEL    WORD
  0014                        41                    ORG     5*4
  0014                        42    INT5_PTR        LABEL   WORD
  0020                        43                    ORG     8*4
  0020                        44    INT_ADDR        LABEL   WORD
  0020                        45    INT_PTR         LABEL   DWORD
  0040                        46                    ORG     10H*4
  0040                        47    VIDEO_INT       LABEL   WORD
  0074                        48                    ORG     1DH*4
  0074                        49    PARM_PTR        LABEL   DWORD           ; POINTER TO VIDEO PARMS
  0060                        50                    ORG     18H*4
  0060                        51    BASIC_PTR       LABEL   WORD            ; ENTRY POINT FOR CASSETTE BASIC
  0078                        52                    ORG     01EH*4          ; INTERRUPT 1EH
  0078                        53    DISK_POINTER    LABEL   DWORD
  007C                        54                    ORG     01FH*4          ; LOCATION OF POINTER
  007C                        55    EXT_PTR LABEL   DWORD                   ; POINTER TO EXTENSION
  0100                        56                    ORG     040H*4          ; ROUTINE
  0100 ????                   57    IO_ROM_INIT     DW      ?               ;
  0102 ????                   58    IO_ROM_SEG      DW      ?               ; OPTIONAL ROM SEGMENT
  0400                        59                    ORG     400H
  0400                        60    DATA_AREA       LABEL   BYTE            ; ABSOLUTE LOCATION OF DATA SEGMENT
  0400                        61    DATA_WORD       LABEL   WORD
  7C00                        62                    ORG     7C00H
  7C00                        63    BOOT_LOCN       LABEL   FAR
  ----                        64    ABS0            ENDS
                              65
                              66    ;--------------------------------------------------------
                              67    ;       STACK -- USED DURING INITIALIZATION ONLY        :
                              68    ;--------------------------------------------------------
  ----                        69    STACK           SEGMENT AT 30H
  0000 (128                   70                    DW      128 DUP(?)
       ????
       )
  0100                        71    TOS             LABEL   WORD
  ----                        72    STACK           ENDS
                              73
                              74    ;----------------------------------------
                              75    ;        ROM BIOS DATA AREAS            :
                              76    ;----------------------------------------
  ----                        77    DATA            SEGMENT AT 40H
  0000 (4                     78    RS232_BASE      DW      4 DUP (?)       ; ADDRESSES OF RS232 ADAPTERS
       ????
       )
  0008 (4                     79    PRINTER_BASE    DW      4 DUP(?)        ; ADDRESSES OF PRINTERS
       ????
       )
  0010 ????                   80    EQUIP_FLAG      DW      ?               ; INSTALLED HARDWARE
  0012 ??                     81    MFG_TST         DB      ?               ; INITIALIZATION FLAG
  0013 ????                   82    MEMORY_SIZE     DW      ?               ; MEMORY SIZE IN K BYTES
  0015 ????                   83    IO_RAM_SIZE     DW      ?               ; MEMORY IN I/O CHANNEL
                              84    ;----------------------------------------
                              85    ;          KEYBOARD DATA AREAS          :
                              86    ;----------------------------------------
  0017 ??                     87    KB_FLAG         DB      ?
                              88
                              89    ;----- SHIFT FLAG EQUATES WITHIN KB FLAG
                              90
    0080                      91    INS_STATE       EQU     80H             ; INSRT STATE IS ACTIVE
    0040                      92    CAPS_STATE      EQU     40H             ; CAPS LOCK STATE HAS BEEN TOGGLED
    0020                      93    NUM_STATE       EQU     20H             ; NUM LOCK STATE HAS BEEN TOGGLED
    0010                      94    SCROLL_STATE    EQU     10H             ; SCROLL LOCK STATE HAS BEEN TOGGLED
    0008                      95    ALT_SHIFT       EQU     08H             ; ALTERNATE SHIFT KEY DEPRESSED
    0004                      96    CLT_SHIFT       EQU     04H             ; CONTROL SHIFT KEY DEPRESSED
    0002                      97    LEFT_SHIFT      EQU     02H             ; LEFT SHIFT KEY DEPRESSED
    0001                      98    RIGHT_SHIFT     EQU     01H             ; RIGHT SHIFT KEY DEPRESSED
                              99
  0018 ??                    100    KB_FLAG_1       DB      ?               ; SECOND BYTE OF KEYBOARD
                             101
    0080                     102    INS_SHIFT       EQU     80H             ; INSERT KEY IS DEPRESSED
    0040                     103    CAPS_SHIFT      EQU     40H             ; CAPS LOCK KEY IS DEPRESSED
    0020                     104    NUM_SHIFT       EQU     20H             ; NUM LOCK KEY IS DEPRESSED
    0010                     105    SCROLL_SHIFT    EQU     10H             ; SCROLL LOCK KEY IS DEPRESSED
    0008                     106    HOLD_STATE      EQU     08H             ; SUSPEND KEY HAS BEEN TOGGLED
                             107
  0019 ??                    108    ALT_INPUT       DB      ?               ; STORAGE FOR ALTERNATE KEYPAD ENTRY
  001A ????                  109    BUFFER_HEAD     DW      ?               ; POINTER TO HEAD OF KEYBOARD BUFFER
  001C ????                  110    BUFFER_TAIL     DW      ?               ; POINTER TO TAIL OF KEYBOARD BUFFER
  001E (16                   111    KB_BUFFER       DW      16 DUP(?)       ; ROOM FOR 15 ENTRIES
       ????
       )
  003E                       112    KB_BUFFER_END   LABEL   WORD
                             113
                             114    ;----- HEAD = TAIL INDICATES THAT THE BUFFER IS EMPTY
                             115
    0045                     116    NUM_KEY         EQU     69              ; SCAN CODE FOR NUMBER LOCK
    0046                     117    SCROLL_KEY      EQU     70              ; SCROLL LOCK KEY
    0038                     118    ALT_KEY         EQU     56              ; ALTERNATE SHIFT KEY SCAN CODE
    001D                     119    CTL_KEY         EQU     29              ; SCAN CODE FOR CONTROL KEY
    003A                     120    CAPS_KEY        EQU     58              ; SCAN CODE FOR SHIFT KEY
    002A                     121    LEFT_KEY        EQU     42              ; SCAN CODE FOR LEFT SHIFT
    0036                     122    RIGHT_KEY       EQU     54              ; SCAN CODE FOR RIGHT SHIFT
    0052                     123    INS_KEY         EQU     82              ; SCAN CODE FOR INSERT KEY
    0053                     124    DEL_KEY         EQU     83              ; SCAN CODE FOR DELETE KEY
                             125
                             126    ;----------------------------------------
                             127    ;       DISKETTE DATA AREAS             ;
                             128    ;----------------------------------------
  003E ??                    129    SEEK_STATUS     DB      ?               ; DRIVE RECALIBRATION STATUS
                             130    ;                               BIT 3-0 = DRIVE 3-0 NEEDS RECAL BEFORE
                             131    ;                                         NEXT SEEK IF BIT IS = 0
    0080                     132    INT_FLAG        EQU     080H            ; INTERRUPT OCCURENCE FLAG
  003F ??                    133    MOTOR_STATUS    DB      ?               ; MOTOR STATUS
                             134    ;                               BIT 3-0 = DRIVE 3-0 IS CURRENTLY RUNNING
                             135    ;                               BIT 7   = CURRENT OP IS A WRITE, REQUIRES DELAY
  0040 ??                    136    MOTOR_COUNT     DB      ?               ; TIME OUT COUNTER FOR DRIVE TURN OFF
    0025                     137    MOTOR_WAIT      EQU     37              ; TWO SEC OF COUNT FOR MOTOR TURN OFF
                             138
  0041 ??                    139    DISKETTE_STATUS DB      ?               ; BYTE OF RETURN CODE INFO FOR STATUS
    0080                     140    TIME_OUT        EQU     80H             ; ATTACHMENT FAILED TO RESPOND
    0040                     141    BAD_SEEK        EQU     40H             ; SEEK OPERATION FAILED
    0020                     142    BAD_NEC         EQU     20H             ; NEC CONTROLLER HAS FAILED
    0010                     143    BAD_CRC         EQU     10H             ; BAD CRC ON DISKETTE READ
    0009                     144    DMA_BOUNDARY    EQU     09H             ; ATTEMPT TO DMA ACROSS 64K BOUNDARY
    0008                     145    BAD_DMA         EQU     08H             ; DMA OVERRUN ON OPERATION
    0004                     146    RECORD_NOT_FND  EQU     04H             ; REQUESTED SECTOR NOT FOUND
    0003                     147    WRITE_PROTECT   EQU     03H             ; WRITE ATTEMPTED ON WRITE PROT DISK
    0002                     148    BAD_ADDR_MARK   EQU     02H             ; ADDRESS MARK NOT FOUND
    0001                     149    BAD_CMD         EQU     01H             ; BAD COMMAND PASSED TO DISKETTE I/O
                             150
  0042 (?                    151    NEC_STATUS      DB      7 DUP(?)        ; STATUS BYTES FROM NEC
       ??
       )
                             152
                             153    ;----------------------------------------
                             154    ;       VIDEO DISPLAY AREA              :
                             155    ;----------------------------------------
  0049 ??                    156    CRT_MODE        DB      ?               ; CURRENT CRT MODE
  004A ????                  157    CRT_COLS        DW      ?               ; NUMBER OF COLUMNS ON SCREEN
  004C ????                  158    CRT_LEN         DW      ?               ; LENGTH OF REGEN IN BYTES
  004E ????                  159    CRT_START       DW      ?               ; STARTING ADDRESS IN REGEN BUFFER
  0050 (8                    160    CURSOR_POSN     DW      8 DUP(?)        ; CURSOR FOR EACH OF UP TO 8 PAGES
       ????
       )
  0060 ????                  161    CURSOR_MODE     DW      ?               ; CURRENT CURSOR MOD SETTING
  0062 ??                    162    ACTIVE_PAGE     DB      ?               ; CURRENT PAGE BEING DISPLAYED
  0063 ????                  163    ADDR_6845       DW      ?               ; BASE ADDRESS FOR ACTIVE DISPLAY CARD
  0065 ??                    164    CRT_MODE_SET    DB      ?               ; CURRENT SETTING OF THE 3X8 REGISTER
  0066 ??                    165    CRT_PALETTE     DB      ?               ; CURRENT PALETTE SETTING COLOR CARD
                             166
                             167    ;----------------------------------------
                             168    ;          CASSETTE DATA AREA           :
                             169    ;----------------------------------------
  0067 ????                  170    EDGE_CNT        DW      ?               ; TIME COUNT AT DATA EDGE
  0069 ????                  171    CRC_REG         DW      ?               ; CRC REGISTER
  006B ??                    172    LAST_VAL        DB      ?               ; LAST INPUT VALUE
                             173
                             174    ;----------------------------------------
                             175    ;           TIMER DATA AREA             :
                             176    ;----------------------------------------
  006C ????                  177    TIMER_LOW       DW      ?               ; LOW WORD OF TIMER COUNT
  006E ????                  178    TIMER_HIGH      DW      ?               ; HIGH WORD OF TIMER COUNT
  0070 ??                    179    TIMER_OFL       DB      ?               ; TIMER HAS ROLLED OVER SINCE LAST READ
                             180    ;COUNTS_SEC     EQU     18
                             181    ;COUNTS_MIN     EQU     1092
                             182    ;COUNTS_HOUR    EQU     65543
                             183    ;COUNTS_DAY     EQU     1573040 = 1800B0H
                             184
                             185    ;----------------------------------------
                             186    ;          SYSTEM DATA AREA             :
                             187    ;----------------------------------------
  0071 ??                    188    BIOS_BREAK      DB      ?               ; BIT 7 = 1 IF BREAK KEY WAS DEPRESSED
  0072 ????                  189    RESET_FLAG      DW      ?               ; WORD = 1234H IF KB RESET UNDERWAY
                             190    ;----------------------------------------
                             191    ;         FIXED DISK DATA AREA          :
                             192    ;----------------------------------------
  0074 ????                  193                    DW      ?               ;
  0076 ????                  194                    DW      ?               ;
                             195    ;----------------------------------------
                             196    ;       PRINTER AND RS232 TIMEOUT CTRS  :
                             197    ;----------------------------------------
  0078 (4                    198    PRINT_TIM_OUT   DB      4 DUP(?)        ; PRINTER TIME OUT COUNTER
       ??
       )
  007C (4                    199    RS232_TIM_OUT   DB      4 DUP(?)        ; RS232 TIME OUT COUNTER
       ??
       )
                             200    ;----------------------------------------
                             201    ;       EXTRA KEYBOARD DATA AREA        :
                             202    ;----------------------------------------
  0080 ????                  203    BUFFER_START    DW      ?
  0082 ????                  204    BUFFER_END      DW      ?
  ----                       205    DATA    ENDS
                             206    ;----------------------------------------
                             207    ;          EXTRA DATA AREA              :
                             208    ;----------------------------------------
  ----                       209    XXDATA          SEGMENT AT 50H
  0000 ??                    210    STATUS_BYTE     DB      ?
  ----                       211    XXDATA          ENDS
                             212
                             213    ;----------------------------------------
                             214    ;       VIDEO DISPLAY BUFFER            :
                             215    ;----------------------------------------
  ----                       216    VIDEO_RAM       SEGMENT AT 0B800H
  0000                       217    REGEN           LABEL   BYTE
  0000                       218    REGENW          LABEL   WORD
  0000 (16385                219                    DB      16384 DUP(?)
       ??
       )
  ----                       220    VIDEO_RAM       ENDS
                             221    ;----------------------------------------
                             222    ;         ROM RESIDENT CODE             :
                             223    ;----------------------------------------
  ----                       224    CODE            SEGMENT AT 0F000H
  0000 (57344                225                    DB      57344 DUP(?)                    ; FILL LOWEST 56K
       ??
       )
                             226
  E000 31353031343736        227                    DB      '1501476 COPR. IBM 1981'        ; COPYRIGHT NOTICE
       20434F50522E20
       49424D20313938
       32
                             228
                             229    ;----------------------------------------------------------------
                             230    ;       INITIAL RELIABILITY TESTS -- PHASE 1                    :
                             231    ;----------------------------------------------------------------
                             232                    ASSUME    CS:CODE,SS:CODE,ES:ABS0,DS:DATA
                             233    ;----------------------------------------
                             234    ;       DATA DEFINITIONS                :
                             235    ;----------------------------------------
  E016 D1E0                  236    C1      DW      C11             ; RETURN ADDRESS
                             237
                             238    ;------------------------------------------------------------------------
                             239    ;       THIS SUBROUTINE PERFORMS A READ/WRITE STORAGE TEST ON           :
                             240    ;       A 16K BLOCK OF STORAGE.                                         :
                             241    ; ENTRY REQUIREMENTS:                                                   :
                             242    ;       ES = ADDRESS OF STORAGE SEGMENT BEING TESTED                    :
                             243    ;       DS = ADDRESS OF STORAGE SEGMENT BEING TESTED                    :
                             244    ;       WHEN ENTERING AT STGTST_CNT, CX MUST BE LOADED WITH             :
                             245    ;       THE BYTE COUNT.                                                 :
                             246    ; EXIT PARAMETERS:                                                      :
                             247    ;       ZERO FLAG = 0 IF STORAGE ERROR (DATA COMPARE OR PARITY CHECK.   :
                             248    ;              AL = 0 DENOTES A PARITY CHECK. ELSE AL=XOR'ED BIT        :
                             249    ;                     PATTERN OF THE EXPECTED DATA PATTERN VS THE       :
                             250    ;                     ACTUAL DATA READ.                                 :
                             251    ;       AX,BX,CX,DX,DI, AND SI ARE ALL DESTROYED.                       :
                             252    ;------------------------------------------------------------------------
                             253
  E018                       254    STGTST  PROC    NEAR
  E018 B90040                255            MOV     CX,4000H               ; SETUP CNT TO TEST A 16K BLK
  E01B                       256    STGTST_CNT:
  E01B FC                    257            CLD                            ; SET DIR FLAG TO INCREMENT
  E01C 88D9                  258            MOV     BX,CX                  ; SAVE BYTE CNT (4K FOR VIDEO OR 16K)
  E01E B8AAAA                259            MOV     AX,0AAAAH              ; GET DATA PATTERN TO WRITE
  E021 BA55FF                260            MOV     DX,0FF55H              ; SETUP OTHER DATA PATTERNS TO USE
  E024 2BFF                  261            SUB     DI,DI                  ; DI = OFFSET 0 RELATIVE TO ES REG
  E026 F3                    262            REP     STOSB                  ; WRITE STORAGE LOCATIONS
  E027 AA
  E028                       263    C3:                                    ;  STG01
  E028 4F                    264            DEC     DI                     ; POINT TO LAST BYTE JUST WRITTEN
  E029 FD                    265            STD                            ; SET DIR FLAG TO GO BACKWARDS
  E02A                       266    C4:
  E02A 8BF7                  267            MOV     SI,DI
  E02C 8BCB                  268            MOV     CX,BX                  ; SETUP BYTE CNT
  E02E                       269    C5:                                    ; INNER TEST LOOP
  E02E AC                    270            LODSB                          ; READ OLD TST BYTE FROM STORAGE [SI]+
  E02F 32C4                  271            XOR     AL,AH                  ; DATA READ AS EXPECTED ?
  E031 7525                  272            JNE     C7                     ;  NO - GO TO ERROR ROUTINE
  E033 8AC2                  273            MOV     AL,DL                  ; GET NEXT DATA PATTERN TO WRITE
  E035 AA                    274            STOSB                          ; WRITE INTO LOCATION JUST READ [DI]+
  E036 E2F6                  275            LOOP    CS                     ;  DECREMENT BYTE COUNT AND LOOP CX
                             276
  E038 22E4                  277            AND     AH,AH                  ; ENDING ZERO PATTERN WRITTEN TO STG ?
  E03A 7416                  278            JZ      C6X                    ; YES - RETURN TO CALLER WITH AL=0
  E03C 8AE0                  279            MOV     AH,AL                  ; SETUP NEW VALUE FOR COMPARE
  E03E 86F2                  280            XCHG    DH,DL                  ; MOVE NEXT DATA PATTERN TO DL
  E040 22E4                  281            AND     AH,AH                  ; READING ZERO PATTERN TO DL
  E042 7504                  282            JNZ     C6                     ; CONTINUE TEST SEQUENCE TILL ZERO DATA
  E044 8AD4                  283            MOV     DL,AH                  ;  ELSE SET ZERO FOR END READ PATTERN
  E046 EBE0                  284            JMP     C3                     ;  AND MAKE FINAL BACKWARDS PASS
  E048                       285    C6:
  E048 FC                    286            CLD                            ; SET DIR FLAG TO GO FORWARD
  E049 47                    287            INC     DI                     ; SET POINTER TO BEG LOCATION
  E04A 74DE                  288            JZ      C4                     ; READ/WRITE FORWARD IN STG
  E04C 4F                    289            DEC     DI                     ; ADJUST POINTER
  E04D BA0100                290            MOV     DX,00001H              ; SETUP 01 FOR PARITY BIT
                             291                                           ;  AND 00 FOR AND
  E050 EBD06                 292            JMP     C3                     ; READ/WRITE BACKWARD IN STG
  E052                       293    C6X:
  E052 E462                  294            IN      AL,PORT_C              ; DID A PARITY ERROR OCCUR  ?
  E054 24C0                  295            AND     AL,0C0H                ; ZERO FLAG WILL BE OFF PARITY ERROR
  E056 B000                  296            MOV     AL,000H                ; AL=0 DATA COMPARE OK
  E058                       297    C7:
  E058 FC                    298            CLD                            ; SET DEFAULT DIRCTN FLAG BACK TO INC
  E059 C3                    299            RET
                             300    STGTST  ENDP
                             301    ;---------------------------------------------------------------
                             302    ;       8088 PROCESSOR TEST                                    :
                             303    ; DESCRIPTION                                                  :
                             304    ;       VERIFY 8088 FLAGS, REGISTERS AND CONDITIONAL JUMPS     :
                             305    ;---------------------------------------------------------------
                             306            ASSUME  CS:CODE,DS:NOTHING,ES:NOTHING,SS:NOTHING
  E05B                       307            ORG     0E05BH
  E05B                       308    RESET   LABEL   FAR
  E05B                       309    START:
  E05B FA                    310             CLI                            ; DISABLE INTERRUPTS
  E05C B4D5                  311             MOV     AH,0D5H                ; SET SF, CF, ZF, AND AF FLAGS ON
  E05E 9E                    312             SAHF
  E05F 734C                  313             JNC     ERR01                  ; GO TO ERR ROUTINE IF CF NOT SET
  E061 754A                  314             JNZ     ERR01                  ; GO TO ERR ROUTINE IF ZF NOT SET
  E063 7B48                  315             JNP     ERR01                  ; GO TO ERR ROUTINE IF PF NOT SET
  E065 7946                  316             JNS     ERR01                  ; GO TO ERR ROUTINE IF SF NOT SET
  E067 9F                    317             LAHF                           ; LOAD FLAG IMAGE TO AH
  E068 B105                  318             MOV     CL,S                   ; LOAD CNT REG WITH SHIFT CNT
  E06A D2EC                  319             SHR     AH,CL                  ; SHIFT AF INTO CARRY BIT POS
  E06C 733F                  320             JNC     ERR01                  ; GO TO ERR ROUTINE IF AF NOT SET
  E06E B040                  321             MOV     AL,40H                 ; SET THE OF FLAG ON
  E070 D0E0                  322             SHL     AL,1                   ; SETUP FOR TESTING
  E072 7139                  323             JNO     ERR01                  ; GO TO ERR ROUTINE IF OF NOT SET
  E074 32E4                  324             XOR     AH,AH                  ; SET AH = 0
  E076 9E                    325             SAHF                           ; CLEAR SF, CF, ZF, AND PF
  E077 7634                  326             JBE     ERR01                  ; GO TO ERR ROUTINE IF CF ON
                             327                                            ; OR TO TO ERR ROUTINE IF ZF ON
  E079 7832                  328             JS      ERR01                  ; GO TO ERR ROUTINE IF SF ON
  E07B 7A30                  329             JP      ERR01                  ; GO TO ERR ROUTINE IF PF ON
  E07D 9F                    330             LAHF                           ; LOAD FLAG IMAGE TO AH
  E07E B105                  331             MOV     CL,5                   ; LOAD CNT REG WITH SHIFT CNT
  E080 D2EC                  332             SHR     AH,CL                  ; SHIFT 'AF' INTO CARRY BIT POS
  E082 7229                  333             JC      ERR01                  ; GO TO ERR ROUTINE IF ON
  E084 D0E4                  334             SHL     AH,1                   ; CHECK THAT 'OF' IS CLEAR
  E086 7025                  335             JO      ERR01                  ; GO TO ERR ROUTINE IF ON
                             336
                             337    ;----- READ/WRITE THE 8088 GENERAL AND SEGMENTATION REGISTERS
                             338    ;        WITH ALL ONE'S AND ZEROES'S.
                             339
  E088 B8FFFF                340            MOV    AX,0FFFFH                ; SETUP ONE'S PATTERN IN AX
  E08B F9                    341            STC
  E08C                       342    C8:
  E08C 8ED8                  343            MOV    DS,AX                    ; WRITE PATTERN TO ALL REGS
  E08E 8CDB                  344            MOV    BX,DS
  E090 8EC3                  345            MOV    ES,BX
  E092 8CC1                  346            MOV    CX,ES
  E094 8ED1                  347            MOV    SS,CX
  E096 8CD2                  348            MOV    DX,SS
  E098 8BE2                  349            MOV    SP,DX
  E09A 8BEC                  350            MOV    BP,SP
  E09C 8BF5                  351            MOV    SI,BP
  E09E 8BFE                  352            MOV    DI,SI
  E0A0 7307                  353            JNC    C9                       ; TST1A
  E0A2 33C7                  354            XOR    AX,DI                    ; PATTERN MAKE IT THRU ALL REGS
  E0A4 7507                  355            JNZ    ERR01                    ; NO - GO TO ERR ROUTINE
  E0A6 F8                    356            CLC
  E0A7 EBE3                  357            JMP    C8
  E0A9                       358    C9:                                     ; TST1A
  E0A9 0BC7                  359            OR     AX,DI                    ; ZERO PATTERN MAKE IT THRU?
  E0AB 7401                  360            JZ      C10                     ; YES - GO TO NEXT TEST
  E0AD F4                    361    ERR01:  HLT                             ; HALT SYSTEM
                             362    ;--------------------------------------------------------
                             363    ;       ROS CHECKSUM TEST I                             :
                             364    ; DESCRIPTION                                           :
                             365    ;       A CHECKSUM IS DONE FOR THE 8K ROS MODULE        :
                             366    ;       CONTAINING POD AND BIOS.                        :
                             367    ;--------------------------------------------------------
  E0AE                       368    C10:
                             369                                            ; ZERO IN AL ALREADY
  E0AE E6A0                  370            OUT     0A0H,AL                 ; DISABLE NMI INTERRUPTS
  E0B0 E683                  371            OUT     83H,AL                  ; INITIALZE DMA PAGE REG
  E0B2 BAD803                372            MOV     DX,3D8H
  E0B5 EE                    373            OUT     DX,AL                   ; DISABLE COLOR VIDEO
  E0B6 FEC0                  374            INC     AL
  E0B8 B2B8                  375            MOV     DL,0B8H
  E0BA EE                    376            OUT     DX,AL                   ; DISABLE B/W VIDEO,EN HIGH RES
  E0BB B089                  377            MOV     AL,99H                  ; SET 8255 A,C-INPUT,B-OUTPUT
  E0BD E663                  378            OUT     CMD_PORT,AL             ; WRITE 8255 CMD/MODE REG
  E0BF B0FC                  379            MOV     AL,0FCH                 ; DISABLE PARITY CHECKERS AND
  E0C1 E661                  380            OUT     PORT_B,AL               ;  GATE SNS SWS,CASS MOTOR OFF
  E0C3 8CC8                  381            MOV     AX,CS                   ; SETUP SS SEG REG
  E0C5 8ED0                  382            MOV     SS,AX
  E0C7 8ED8                  383            MOV     DS,AX                   ; SET UP DATA SEG TO POINT TO
                             384                                            ;  ROM ADDRESS
                             385            ASSUME  SS:CODE
  E0C9 B7E0                  386            MOV     BH,0E0H                 ; SETUP STARTING ROS ADDR(99 (E0000)
  E0CB BC16E0                387            MOV     SP,OFFSET C1            ; SETUP RETURN ADDRESS
  E0CE E97B0B                388            JMP     ROS_CHECKSUM
  E0D1                       389    C11:
  E0D1 75DA                  390            JNE     ERR01                   ; HALT SYSTEM IF ERROR
                             391    ;----------------------------------------------------------------
                             392    ;       8237 DMA INITIALIZATION CHANNEL REGISTER TEST           :
                             393    ; DESCRIPTION                                                   :
                             394    ;       DISABLE THE 8237 DMA CONTROLLER.  VERIFY THAT TIMER 1   :
                             395    ;       FUNCTIONS OK. WRITE/READ THE CURRENT ADDRESS AND WORD   :
                             396    ;       COUNT REGISTERS FOR ALL CHANNELS.  INITIALIZE AND       :
                             397    ;       START DMA FOR MEMORY REFRESH.                           :
                             398    ;----------------------------------------------------------------
  E0D3 B004                  399            MOV     AL,04                   ; DISABLE DMA CONTROLLER
  E0D5 E608                  400            OUT     DMA08,AL
                             401
                             403
  E0D7 B054                  404            MOV     AL,54H                  ; SEL TIMER 1,LSB,MODE 2
  E0D9 E643                  405            OUT     TIMER+3,AL
  E0DB 8AC1                  406            MOV     AL,CL                   ; SET INITIAL TIMER CNT TO 0
  E0DD E641                  407            OUT     TIMER+1,AL
  E0DF                       408    C12:                                    ; TIMER1_BITS_ON
  E0DF B040                  409            MOV     AL,40H                  ; LATCH TIMER 1 COUNT
  E0E1 E643                  410            OUT     TIMER+3,AL
  E0E3 80FBFF                411            CMP     BL,0FFH                 ; YES - SEE IF ALL BITS GO OFF
  E0E6 7407                  412            JE      C13                     ; TIMER1_BITS_OFF
  E0E8 E441                  413            IN      AL,TIMER+1              ; READ TIMER 1 COUNT
  E0EA 0AD8                  414            OR      BL,AL                   ; ALL BITS ON IN TIMER
  E0EC E2F1                  415            LOOP    C12                     ; TIMER1_BITS_ON
  E0EE F4                    416            HLT                             ; TIMER 1 FAILURE, HALT SYS
  E0EF                       417    C13:                                    ; TIMER1_BITS_OFF
  E0EF 8AC3                  418            MOV     AL,BL                   ; SET TIMER 1 CNT
  E0F1 2BC9                  419            SUB     CX,CX
  E0F3 E641                  420            OUT     TIMER+1,AL
  E0F5                       421    C14:                                    ; TIMER_LOOP
  E0F5 B040                  422            MOV     AL,40H                  ; LATCH TIMER 1 COUNT
  E0F7 E643                  423            OUT     TIMER+3,AL
  E0F9 90                    424            NOP                             ; DELAY FOR TIMER
  E0FA 90                    425            NOP
  E0FB E441                  426            IN      AL,TIMER+1              ; READ TIMER 1 COUNT
  E0FD 22D8                  427            AND     BL,AL
  E0FF 7403                  428            JZ      C15                     ; GO TO WRAP_DMA_REG
  E101 E2F2                  429            LOOP    C14                     ; TIMER_LOOP
  E103 F4                    430            HLT                             ; TIMER ERROR - HALT SYSTEM
                             431
                             432    ;----- INITIALIZE TIMER 1 TO REFRESH MEMORY
                             433
  E104                       434    C15:                                    ; WRAP_DMA_REG
  E104 B012                  435            MOV     AL,18                   ; SETUP DIVISOR FOR REFRESH
  E106 E641                  436            OUT     TIMER+1,AL              ; WRITE TIMER 1 CNT REG
  E108 E60D                  437            OUT     DMA+0DH,AL              ; SEND MASTER CLEAR TO DMA
                             438
                             439    ;----- WRAP DMA CHANNELS ADDRESS AND COUNT REGISTERS
                             440
  E10A B0FF                  441            MOV     AL,0FFH                 ; WRITE PATTERN FF TO ALL REGS
  E10C                       442    C16:
  E10C 8AD8                  443            MOV     BL,AL                   ; SAVE PATTERN FOR COMPARE
  E10E 8AF8                  444            MOV     BH,AL
  E110 B90800                445            MOV     CX,8                    ; SETUP LOOP CNT
  E113 2BD2                  446            SUB     DX,DX                   ; SETUP I/O PORT ADDR OF REG (0000)
  E115                       447    C17:
  E115 EE                    448            OUT     DX,AL                   ; WRITE PATTERN TO REG, LSB
  E116 50                    449            PUSH    AX
  E117 EE                    450            OUT     DX,AL                   ; MSB OF 16 BIT REG
  E118 B80101                451            MOV     AX,0101H                ; AX TO ANOTHER PAT BEFORE RD
  E11B EC                    452            IN      AL,DX                   ; READ 16-BIT DMA CH REG, LSB
  E11C 8AE0                  453            MOV     AH,AL                   ; SAVE LSB OF 16-BIT REG
  E11E EC                    454            IN      AL,DX                   ; READ MSB OF DMA CH REG
  E11F 3BD8                  455            CMP     BX,AX                   ; PATTERN READ AS WRITTEN?
  E121 7401                  456            JE      C18                     ; YES - CHECK NEXT REG
  E123 F4                    457            HLT                             ; NO - HALT THE SYSTEM
  E124                       458    C18:                                    ; NXT_DMA_CH
  E124 42                    459            INC     DX                      ; SET I/O PORT TO NEXT CH REG
  E125 E2EF                  460            LOOP    C17                     ; WRITE PATTERN TO NEXT REG
  E127 FEC0                  461            INC     AL                      ; SET PATTERN TO 0
  E129 74E1                  462            JZ      C16                     ; WRITE TO CHANNEL REGS
                             463
                             464    ;----- INITIALIZE AND START DMA FOR MEMORY REFRESH.
                             465
  E12B 8EDB                  466            MOV     DS,BX                   ; SET UP AB50 INTO DS AND ES
  E12D 8EC3                  467            MOV     ES,BX
                             468            ASSUME  DS:AB50,ES:AB50
                             469
  E12F B0FF                  470            MOV     AL,OFFH                 ; SET CNT OF 64K FOR RAM REFRESH
  E131 E601                  471            OUT     DMA+1,AL
  E133 50                    472            PUSH    AX
  E134 E601                  473            OUT     DMA+1,AL
  E136 B20B                  474            MOV     DL,0BH                  ; DX=000B
  E138 B058                  475            MOV     AL,058H                 ; SET DMA MODE,CH 0,READ,AUTOINT
  E13A EE                    476            OUT     DX,AL                   ; WRITE DMA MODE REG
  E13B B000                  477            MOV     AL,0                    ; ENABLE DMA CONTROLLER
  E13D E608                  478            OUT     DMA+8,AL                ; SETUP DMA COMMAND REG
  E13F 50                    479            PUSH    AX
  E140 E60A                  480            OUT     DMA+10,AL               ; ENABLE CHANNEL 0 FOR REFRESH
  E142 B103                  481            MOV     CL,3
  E144 B041                  482            MOV     AL,41H                  ; SET MODE FOR CHANNEL 1
  E146                       483    C18A:
  E146 EE                    484            OUT     DX,AL
  E147 FEC0                  485            INC     AL                      ; POINT TO NEXT CHANNEL
  E149 E2FB                  486            LOOP    C18A
                             487    ;----------------------------------------------------------------
                             488    ;       BASE 16K READ/WRITE STORAGE TEST                        :
                             489    ; DESCRIPTION                                                   :
                             490    ;       WRITE/READ/VERIFY DATA PATTERNS FF,55,AA,01, AND 00     :
                             491    ;       TO 1ST 16K OF STORAGE.  VERIFY STORAGE ADDRESSABILITY.  :
                             492    ;       INITIALIZE THE 8259 INTERRUPT CONTROLLER CHIP FOR       :
                             493    ;       CHECKING MANUFACTURING TEST 2 MODE.                     :
                             494    ;----------------------------------------------------------------
                             495
                             496    ;----- DETERMINE MEMORY SIZE AND FILL MEMORY WITH DATA
                             497
  E14B BA1302                498            MOV     DX,0213H                ; ENABLE EXPANSION BOX
  E14E B001                  499            MOV     AL,01H
  E150 EE                    500            OUT     DX,AL
  E151 8B2E7204              501            MOV     BP,DATA_WORD[OFFSET RESET_FLAG] ; SAVE 'RESET_FLAG' IN BP
  E155 81FD3412              502            CMP     BP,1234H                ; WARM START?
  E159 740A                  503            JE      C18B
  E15B BC41F090              504            MOV     SP,OFFSET C2
  E15F E9B6FE                505            JMP     STGTST
  E162                       506    C24:
  E162 7401                  507            JE      C18B                    ; PROCEED IF STGTST OK
  E164 F4                    508            HLT                             ; HALT IF NOT
  E165                       509    C18B:
  E165 2BFF                  510            SUB     DI,DI
  E167 E460                  511            IN      AL,PORT_A               ; DETERMINE BASE RAM SIZE
  E169 240C                  512            AND     AL,0CH                  ; ISOLATE RAM SIZE SWS
  E16B 0404                  513            ADD     AL, 4                   ; CALCULATE MEMORY SIZE
  E16D B10C                  514            MOV     CL, 12
  E16F D3E0                  515            SHL     AX, CL
  E171 8BC8                  516            MOV     CX, AX
  E173 PC                    517            CLB                             ; SET DIR FLAG TO INCR
  E174                       518    C19:
  E174 AA                    519            STOSB                           ; FILL BASE RAM WITH DATA
  E175 E2FD                  520            LOOP    C19                     ; LOOP TIL ALL ZERO
  E177 892E7204              521            MOV     DATA_WORD[OFFSET RESET_FLAG1,BP
                             522
                             523    ;----- DETERMINE IO CHANNEL RAM SIZE
                             524
  E17B B0F8                  525            MOV     AL,0F8H                 ; ENABLE SWITCH 5
  E170 E661                  526            OUT     PORT_B,AL
  E17F E462                  527            IN      AL,PORT_C               ; READ SWITCHES
  E181 2401                  528            AND     AL,00000001B            ; ISOLATE SWITCH 5
  E183 B10C                  529            MOV     CL,12D
  E185 D3C0                  530            ROL     AX,CL
  E187 B0FC                  531            MOV     AL,0FCH                 ; DISABLE SW. 5
  E189 E661                  532            OUT     PORT_B,AL
  E18B E462                  533            IN      AL,PORT_C
  E18D 240F                  534            AND     AL,0FH
  E18F 0AC4                  535            OR      AL,AH                   ; COMBINE SWITCH VALUES
  E191 8AD8                  536            MOV     BL,AL                   ; SAVE
  E193 B420                  537            MOV     AH,32
  E195 F6E4                  538            MUL     AH                      ; CALC. LENGTH
  E197 A31504                539            MOV     DATA_WORD[OFFSET IO_RAM_SIZE],AX        ;SAVE IT
  E19A 7418                  540            JZ      C21
  E19C BA0010                541            MOV     DX,1000H                ; SEGMENT FOR I/O RAM
  E19F 8AE0                  542            MOV     AH,AL
  E1A1 B000                  543            MOV     AL,0
  E1A3                       544    C20:                                    ; FILL_IO:
  E1A3 8EC2                  545            MOV     ES,DX
  E1A5 B90080                546            MOV     CX,8000H                ; FILL 32K BYTES
  E1A8 2BFF                  547            SUB     DI,DI
  E1AA F3                    548            REP     STOSB
  E1AB AA
  E1AC 81C20008              549            ADD     DX,800H                 ; NEXT SEGMENT VALUE
  E1B0 FECB                  550            DEC     BL
  E1B2 75EF                  551            JNZ     C20                     ; FILL_IO
                             552    ;----------------------------------------------------------------
                             553    ;       INITIALIZE THE 8259 INTERRUPT CONTROLLER CHIP           :
                             554    ;----------------------------------------------------------------
  E1B4                       555    C21:
  E1B4 B013                  556            MOV     AL,13H                  ; ICW1 - EDGE, SNGL, ICMA
  E1B6 E620                  557            OUT     INTA00,AL
  E1B8 B008                  558            MOV     AL,8                    ; SETUP ICW2 - INT TYPE 8 (8-F)
  E1BA E621                  559            OUT     INTA01,AL
  E1BC B009                  560            MOV     AL,9                    ; SETUP ICW4 - BUFFR0,8086 MODE
  E1BE E621                  561            OUT     INTA01,AL
  E1C0 2BC0                  562            SUB     AX,AX                   ; POINT ES TO BEGIN
  E1C2 8EC0                  563            MOV     ES,AX                   ; OF R/W STORAGE
                             564    ;----------------------------------------------------------------------
                             565    ;       CHECK FOR MANUFACTURING TEST 2 TO LOAD PROGRAMS FROM KEYBOARD.:
                             566    ;----------------------------------------------------------------------
                             567
                             568    ;----- SETUP STACK SEG AND SP
                             569
  E1C4 B83000                570            MOV     AX,STACK                ; GET STACK VALUE
  E1C7 8ED0                  571            MOV     SS,AX                   ; SET THE STACK UP
  E1C9 BC0001                572            MOV     SP,OFFSET TOS           ; STACK IS READY TO GO
  E1CC 81FD3412              573            CMP     BP,1234H                ; RESET_FLAG SET?
  E1D0 7425                  574            JE      C25                     ; YES - SKIP MFG TEST
  E1D2 2BFF                  575            SUB     DI,DI
  E1D4 8EDF                  576            MOV     DS, DI
  E1D6 BB2400                577            MOV     BX, 24H
  E1D9 C70747FF              578            MOV     WORD PTR [BX],OFFSET D11 ; SET UP KB INTERRUPT
  E1DD 43                    579            INC     BX
  E1DE 43                    580            INC     BX
  E1DF 8C0F                  581            MOV     [BX],CS
  E1E1 E85F04                582            CALL    KBD_RESET               ; READ IN KB RESET COD ETO BL
  E1E4 80FB65                583            CMP     BL,065H                 ; IS THIS MANUFACTURING TEST 2?
  E1E7 750E                  584            JNZ     C25                     ; JUMP IF NOT MAN. TEST
  E1E9 B2FF                  585            MOV     DL,255                  ; READ IN TEST PROGRAM
  E1EB                       586    C22:
  E1EB E86204                587            CALL    SP_TEST
  E1EE 8AC3                  588            MOV     AL,BL
  E1F0 AA                    589            STOSB
  E1F1 FECA                  590            DEC     DL
  E1F3 75F6                  591            JNZ     C22                     ; JUMP IF NOT DONE YET
  E1F5 CD3E                  592            INT     3EH                     ; SET INTERRUPT TYPE 62 ADDRESS F8H
  E1F7                       593    C25:
                             594
                             595    ;----- SET UP THE BIOS INTERRUPT VECTORS TO TEMP INTERRUPT
                             596
  E1F7 B92000                597            MOV     CX,32                   ; FILL ALL 32 INTERRUPTS
  E1FA 2BFF                  598            SUB     DI,DI                   ; FIRST INTERRUPT LOCATION
  E1FC                       599    D3:
  E1FC B847FF                600            MOV     AX,OFFSET D11           ; MOVE ADDR OF INTR PROC TO TBL
  E1FF AB                    601            STOSW
  E200 8CC8                  602            MOV     AX,CS                   ; GET ADDR OF INTR PROC SEG
  E202 AB                    603            STOSW
  E203 E2F7                  604            LOOP    D3                      ; VECTBLO
                             605
                             606    ;----- SET UP OTHER INTERRUPTS AS NECESSARY
                             607
  E205 C7060800C3E2          608            MOV     NMI_PTR,OFFSET NMI_INT  ; NMI INTERRUPT
  E20B C706140054FF          609            MOV     INT5_PTR,OFFSET PRINT_SCREEN    ; PRINT SCREEN
  E211 C706620000F6          610            MOV     BASIC_PTR+2,0F600H      ; SEGMENT FOR CASSETTE BASIC
                             611
                             612    ;----------------------------------------------------------------
                             613    ;       8259 INTERRUPT CONTROLLER TEST                          :
                             614    ; DESCRIPTION                                                   :
                             615    ;       READ/WRITE THE INTERRUPT MASK REGISTER (IMR) WITH ALL   :
                             616    ;       ONES AND ZEROES. ENABLE SYSTEM INTERRUPTS.  MASK DEVICE :
                             617    ;       INTERRUPTS OFF. CHECK FOR HOT INTERRUPTS (UNEXPECTED).  :
                             618    ;-------------------------------------------------------------------
                             619
                             620    ;----- TEST THE IMR REGISTER
                             621
  E217 BA2100                622            MOV     DX,0021H                ; POINT INTR. CHIP ADDR 21
  E21A B000                  623            MOV     AL,0                    ; SET IMR TO ZERO
  E21C EE                    624            OUT     DX,AL
  E21D EC                    625            IN      AL,DX                   ; READ IMR
  E21E 0AC0                  626            OR      AL,AL                   ; IMR = 0?
  E220 7515                  627            JNZ     D6                      ; GO TO ERR ROUTINE IF NOT 0
  E222 B0FF                  628            MOV     AL,0FFH                 ; DISABLE DEVICE INTERRUPTS
  E224 EE                    629            OUT     DX,AL                   ; WRITE TO IMR
  E225 EC                    630            IN      AL,DX                   ; READ IMR
  E226 0401                  631            ADD     AL,1                    ; ALL IMR BIT ON?
  E228 750D                  632            JNZ     D6                      ; NO - GO TO ERR ROUTINE
                             633
                             634    ;----- CHECK FOR HOT INTERRUPTS
                             635
                             636    ;----- INTERRUPTS ARE MASKED OFF.  CHECK THAT NO INTERRUPTS OCCUR.
                             637
  E22A 32E4                  638            XOR     AH,AH                   ; CLEAR AH REG
  E22C FB                    639            STI                             ; ENABLE EXTERNAL INTERRUPTS
  E22D 2BC9                  640            SUB     CX,CX                   ; WAIT 1 SEC FOR ANY INTRS THAT
  E22F                       641    D4:
  E22F E2FE                  642            LOOP    D4                      ;  MIGHT OCCUR
  E231                       643    D5:
  E231 E2FE                  644            LOOP    D5
  E233 0A34                  645            OR      AH,AH                   ; DID ANY INTERRUPTS OCCUR?
  E235 7408                  646            HZ      D7                      ; NO - GO TO NEXT TEST
  E237                       647    D6:
  E237 BA0101                648            MOV     DX,101H                 ; BEEP SPEAKER IF ERROR
  E23A E89203                649            CALL    ERR_BEEP                ; GO TO BEEP SUBROUTINE
  E23D FA                    650            CLI
  E23E F4                    651            HLT                             ; HALT THE SYSTEM
                             652    ;------------------------------------------------
                             653    ;       8253 TIMER CHECKOUT                     :
                             654    ; DESCRIPTION                                   :
                             655    ;       VERIFY THAT THE SYSTEM TIMER (0)        :
                             656    ;       DOESN"T COUNT TOO FAST OR TOO SLOW.     :
                             657    ;------------------------------------------------
  E23F                       658    D7:
  E23F B0FE                  659            MOV     AL,0FEH                 ; MASK ALL INTRS EXCEPT LVL 0
  E241 EE                    660            OUT     DX,AL                   ; WRITE THE 8259 IMR
  E242 B010                  661            MOV     AL,00010000B            ; SEL TIM 0, LSB, MODE 0, BINARY
  E244 E643                  662            OUT     TIM_CTL,AL              ; WRITE TIMER CONTROL MOD REG
  E246 B91600                663            MOV     CX,16H                  ; SET PGM LOOP CNT
  E249 8AC1                  664            MOV     AL,CL                   ; SET TIMER 0 CNT REG
  E24B E640                  665            OUT     TIMER0,AL               ; WRITE TIMER 0 CNT REG
  E240                       666    D8:
  E240 F6C4FF                667            TEST    AH,0FFH                 ; DID TIMER 0 INTERRUPT OCCUR
  E250 7504                  668            JNZ     D9                      ; YES - CHECK TIMER OP FOR SLOW TIME
  E252 E2F9                  669            LOOP    D8                      ; WAIT FOR INTR FOR SPECIFIED TIMER
  E254 EBE1                  670            JMP     D6                      ; TIMER 0 INTR DIDN"T OCCUR - ERR
  E256                       671    D9:
  E256 B112                  672            MOV     CL,18                   ; SET PGM LOOP CNT
  E258 B0FF                  673            MOV     AL,0FFH                 ; WRITE TIMER 0 CNT REG
  E25A E640                  674            OUT     TIMER0,AL
  E25C B8FE00                675            MOV     AX,0FEH
  E25F EE                    676            OUT     DX,AL
  E260                       677    D10L
  E260 F6C4FF                678            TEST    AH,0FFH                 ; DID TIMER 0 INTERRUPT OCCUR?
  E263 75D2                  679            JNZ     D6                      : YES - TIMER CNTING TOO FAST, ERR
  E265 E2F9                  680            LOOP    D10                     ; WAIT FOR INTR FOR SPECIFIED TIME
                             681
                             682    ;----- ESTABLISH BIOS SUBROUTINE CALL INTERRUP VECTORS
                             683
  E267 1E                    684            PUSH    DS                      ; SAVE POINTER TO DATA AREA
  E268 BF4000                685            MOV     DI,OFFSET VIDEO_INT     ; SETUP ADDR TO INTR AREA
  E26B 0E                    686            PUSH    CS
  E26C 1F                    687            POP     DS                      ; SETUP ADDR OF VECTOR TABLE
  E26D BE03FF90              688            MOV     SI,OFFSET VECTOR_TABLE+16       ; START WITH VIDEO ENTRY
  E271 B91000                689            MOV     CX,16
                             690
                             691    ;----- SETUP TIMER 0 TO MODE 3
                             692
  E274 B0FF                  693            MOV     AL,0FFH                 ; DISABLE ALL DEVICE INTERRUPTS
  E276 EE                    694            OUT     DX,AL
  E277 B036                  695            MOV     AL,36H                  ; SET TIM 0,LSB,MSB,MODE 3
  E279 E643                  696            OUT     TIMER+3,AL              ; WRITE TIMER MODE REG
  E27B B000                  697            MOV     AL,0
  E27D E640                  698            OUT     TIMER,AL                ; WRITE LSB TO TIMER 0 REG
  E27F                       699    E1A:
  E27F A5                    700            MOVSW                           ; MOVE VECTOR TABLE TO RAM
  E280 47                    701            INC     DI                      ; MOVE PAST SEGMENT POINTER
  E281 47                    702            INC     DI
  E282 E2FB                  703            LOOP    E1A
  E284 E640                  704            OUT     TIMER,AL                ; WRITE MSB TO TIMER 0 REG
  E286 1F                    705            POP     DS                      ; RECOVER DATA SEG POINTER
                             706
                             707    ;----- SETUP TIMER 0 TO BLINK LED IF MANUFACTURING TEST MODE
                             708
  E287 E8B903                709            CALL    KBD_RESET               ; SEND SOFTWARE RESET TO KEYBRD
  E28A 80FBAA                710            CMP     BL,0AAH                 ; SCAN CODE 'AA' RETURNED?
  E28D 741E                  711            JE      E6                      ; YES - CONTINUE (NON MFG MODE)
  E28F B03C                  712            MOV     AL,3CH                  ; EN KBD, SET KBD CLK LINE LOW
  E291 E661                  713            OUT     PORT_B,AL               ; WRITE 8255 PORT B
  E293 90                    714            NOP
  E294 90                    715            NOP
  E295 E460                  716            IN      AL,PORT_A               ; WAS A BIT CLOCKED IN?
  E297 24FF                  717            AND     AL,0FFH
  E299 750E                  718            JNZ     E2                      ; YES - CONTINUE (NON MFG MODE)
  E29B FE061204              719            INC     DATA_AREA[OFFSET MFG_TST]       ; ELSE SET SW FOR MFG TEST MODE
  E29F C70620006DE6          720            MOV     INT_ADDR,OFFSET BLINK_INT       ; SETUP TIMER ITNR TO BLINK LED
  E2A5 B0FE                  721            MOV     AL,0FEH                 ; ENABLE TIMER INTERRUPT
  E2A7 E621                  722            OUT     INTA01,AL
  E2A9                       723    E2:                                     ; JUMPER_NOT_IN:
  E2A9 B0CC                  724            MOV     AL,0CCH                 ; RESET THE KEYBOARD
  E2AB E661                  725            OUT     PORT_B,AL
                             726
                             727    ;--------------------------------------------------------
                             728    ;       INITIALIZE AND START CRT CONTROLLER (6845)      :
                             729    ;       TEST VIDEO READ/WRITE STORAGE.                  :
                             730    ; DESCRIPTION                                           :
                             731    ;       RESET THE VIDEO ENABLE SIGNAL.                  :
                             732    ;       SELECT ALPHANUMERIC MODE, 40 * 25, B & W.       :
                             733    ;       READ/WRITE DATA PATTERNS TO STG. CHECK STG      :
                             734    ;       ADDRESSABILITY.                                 :
                             735    ;--------------------------------------------------------
  E2AD                       736    E6:
  E2AD E460                  737            IN      AL,PORT_A               ; READ SENSE SWITCHES
  E2AF B400                  738            MOV     AH,0
  E2B1 A31004                739            MOV     DATA_WORD[OFFSET EQUIP_FLAG],AX : STORE SENSE SW INFO
  E2B4                       740    E6A:
  E2B4 2430                  741            AND     AL,30H                  ; ISOLATE VIDEO SWS
  E2B6 7529                  742            JNZ     E7                      ; VIDEO SWS SET TO 0?
  E2B8 C706400053FF          743            MOV     VIDEO_INT,OFFSET DUMMY_RETURN
  E2BE E9A200                744            JMP     E18_1                   ; SKIP VIDO TESTS FOR BURN-IN
                             745
  E2C3                       746            ORG     0E2C3H
  E2C3                       747    NMI_INT PROC    NEAR
  E2C3 50                    748            PUSH    AX                      ; SAVE ORIG CONTENTS OF AX
  E2C4 E462                  749            IN      AL,PORT_C
  E2C6 A8C0                  750            TEST    AL,0C0H                 ; PARITY CHECK?
  E2C8 7415                  751            JZ      D14                     ; NO, EXIT FROM ROUTINE
  E2CA BEDAFF90              752            MOV     SI,OFFSET D1            ; ADDR OF ERROR MSG
  E2CE A840                  753            TEST    AL,40H                  ; I/O PARITY CHECK
  E2D0 7504                  754            JNZ     D13                     ; DISPLAY ERROR MSG
  E2D2 BE23FF90              755            MOV     SI,OFFSET D2            ; MUST BE PLANAR
  E2D6                       756    D13:
  E2D6 2BC0                  757            SUB     AX,AX                   ; INIT AND SET MODE FOR VIDEO
  E2D8 CD10                  758            INT     10H                     ; CALL VIDEO_IO PROCEDURE
  E2DA 380D03                759            CALL    P_MSG                   ; PRINT ERROR MSG
  E2DD FA                    760            CLI
  E2DE F4                    761            HLT                             ; HALT SYSTEM
  E2DF                       762    D14:
  E2DF 58                    763            POP     AX                      ; RESTORE ORIG CONTENTS OF AX
  E2E0 CF                    764            IRET
                             765    NMI_INT ENDP
  E2E1                       766    E7:                                     ; TEST_VIDEO:
  E2E1 3C30                  767            CMP     AL,30H                  ; B/W CARD ATTACHED?
  E2E3 7408                  768            JE      E8                      ; YES - SET MODE FOR B/W CARD
  E2E5 FEC4                  769            INC     AH                      ; SET COLOR MODE FOR COLOR CD
  E2E7 3C20                  770            CMP     AL,20H                  ; 80X25 MODE SELECTED?
  E2E9 7502                  771            JNE     E8                      ; NO - SET MODE FOR 40X25
  E2EB B403                  772            MOV     AH,3                    ; SET MODE FOR 80X25
  E2ED                       773    E8:
  E2ED 86E0                  774            XCHG    AH,AL                   ; SET_MODE
  E2EF 50                    775            PUSH    AX                      ; SAVE VIDEO MODE ON STACK
  E2F0 2AE4                  776            SUB     AH,AH                   ; INITIALIZE TO ALPHANUMERIC MD
  E2F2 CD10                  777            INT     10H                     ; CALL VIDEO_IO
  E2F4 58                    778            POP     AX                      ; RESTORE VIDEO SENSE SWS IN AH
  E2F5 50                    779            PUSH    AX                      ; RESAVE VALUE
  E2F6 BB00B0                780            MOV     BX,0B000H               ; BEG VIDEO RAM ADDR B/W CD
