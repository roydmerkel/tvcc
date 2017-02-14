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
                              34
                              35    ;----------------------------------------
                              36    ;       8088 INTERRUPT LOCATIONS        :
                              37    ;----------------------------------------
                              38
  ----                        39    ABS0            SEGMENT AT 0
  0000                        40    STG_LOC0        LABEL   BYTE
  0008                        41            ORG     2*4
  0008                        42    NMI_PTR         LABEL    WORD
  0014                        43            ORG     5*4
  0014                        44    INT5_PTR        LABEL   WORD
  0020                        45            ORG     8*4
  0020                        46    INT_ADDR        LABEL   WORD
  0020                        47    INT_PTR         LABEL   DWORD
  0040                        48            ORG     10H*4
  0040                        49    VIDEO_INT       LABEL   WORD
  0074                        50            ORG     1DH*4
  0074                        51    PARM_PTR        LABEL   DWORD           ; POINTER TO VIDEO PARMS
  0060                        52            ORG     18H*4
  0060                        53    BASIC_PTR       LABEL   WORD            ; ENTRY POINT FOR CASSETTE BASIC
  0078                        54            ORG     01EH*4                  ; INTERRUPT 1EH
  0078                        55    DISK_POINTER    LABEL   DWORD
  007C                        56            ORG     01FH*4                  ; LOCATION OF POINTER
  007C                        57    EXT_PTR LABEL   DWORD                   ; POINTER TO EXTENSION
  0400                        58            ORG     400H
  0400                        59    DATA_AREA       LABEL   BYTE            ; ABSOLUTE LOCATION OF DATA SEGMENT
  0400                        60    DATA_WORD       LABEL   WORD
  0500                        61            ORG     0500H
  0500                        62    MFG_TEST_RTN    LABEL   FAR
  7C00                        63                    ORG     7C00H
  7C00                        64    BOOT_LOCN       LABEL   FAR
  ----                        65    ABS0            ENDS
                              66
                              67    ;--------------------------------------------------------
                              68    ;       STACK -- USED DURING INITIALIZATION ONLY        :
                              69    ;--------------------------------------------------------
                              70
  ----                        71    STACK           SEGMENT AT 30H
  0000 (128                   72                    DW      128 DUP(?)
       ????
       )
  0100                        73    TOS             LABEL   WORD
  ----                        74    STACK           ENDS
                              75
                              76    ;----------------------------------------
                              77    ;        ROM BIOS DATA AREAS            :
                              78    ;----------------------------------------
                              79
  ----                        80    DATA            SEGMENT AT 40H
  0000 (4                     81    RS232_BASE      DW      4 DUP (?)       ; ADDRESSES OF RS232 ADAPTERS
       ????
       )
  0008 (4                     82    PRINTER_BASE    DW      4 DUP(?)        ; ADDRESSES OF PRINTERS
       ????
       )
  0010 ????                   83    EQUIP_FLAG      DW      ?               ; INSTALLED HARDWARE
  0012 ??                     84    MFG_TST         DB      ?               ; INITIALIZATION FLAG
  0013 ????                   85    MEMORY_SIZE     DW      ?               ; MEMORY SIZE IN K BYTES
  0015 ??                     86    MFG_ERR_FLAG    DB      ?               ; SCRATCHPAD FOR MANUFACTURING
  0016 ??                     87                    DB      ?               ; ERROR CODES
                              88
                              89    ;----------------------------------------
                              90    ;          KEYBOARD DATA AREAS          :
                              91    ;----------------------------------------
                              92
  0017 ??                     93    KB_FLAG         DB      ?
                              94
                              95    ;----- SHIFT FLAG EQUATES WITHIN KB FLAG
                              96
    0080                      97    INS_STATE       EQU     80H             ; INSRT STATE IS ACTIVE
    0040                      98    CAPS_STATE      EQU     40H             ; CAPS LOCK STATE HAS BEEN TOGGLED
    0020                      99    NUM_STATE       EQU     20H             ; NUM LOCK STATE HAS BEEN TOGGLED
    0010                     100    SCROLL_STATE    EQU     10H             ; SCROLL LOCK STATE HAS BEEN TOGGLED
    0008                     101    ALT_SHIFT       EQU     08H             ; ALTERNATE SHIFT KEY DEPRESSED
    0004                     102    CLT_SHIFT       EQU     04H             ; CONTROL SHIFT KEY DEPRESSED
    0002                     103    LEFT_SHIFT      EQU     02H             ; LEFT SHIFT KEY DEPRESSED
    0001                     104    RIGHT_SHIFT     EQU     01H             ; RIGHT SHIFT KEY DEPRESSED
                             105
  0018 ??                    106    KB_FLAG_1       DB      ?               ; SECOND BYTE OF KEYBOARD
                             107
    0080                     108    INS_SHIFT       EQU     80H             ; INSERT KEY IS DEPRESSED
    0040                     109    CAPS_SHIFT      EQU     40H             ; CAPS LOCK KEY IS DEPRESSED
    0020                     110    NUM_SHIFT       EQU     20H             ; NUM LOCK KEY IS DEPRESSED
    0010                     111    SCROLL_SHIFT    EQU     10H             ; SCROLL LOCK KEY IS DEPRESSED
    0008                     112    HOLD_STATE      EQU     08H             ; SUSPEND KEY HAS BEEN TOGGLED
                             113
  0019 ??                    114    ALT_INPUT       DB      ?               ; STORAGE FOR ALTERNATE KEYPAD ENTRY
  001A ????                  115    BUFFER_HEAD     DW      ?               ; POINTER TO HEAD OF KEYBOARD BUFFER
  001C ????                  116    BUFFER_TAIL     DW      ?               ; POINTER TO TAIL OF KEYBOARD BUFFER
  001E (16                   117    KB_BUFFER       DW      16 DUP(?)       ; ROOM FOR 15 ENTRIES
       ????
       )
  003E                       118    KB_BUFFER_END   LABEL   WORD
                             119
                             120    ;----- HEAD = TAIL INDICATES THAT THE BUFFER IS EMPTY
                             121
    0045                     122    NUM_KEY         EQU     69              ; SCAN CODE FOR NUMBER LOCK
    0046                     123    SCROLL_KEY      EQU     70              ; SCROLL LOCK KEY
    0038                     124    ALT_KEY         EQU     56              ; ALTERNATE SHIFT KEY SCAN CODE
    001D                     125    CTL_KEY         EQU     29              ; SCAN CODE FOR CONTROL KEY
    003A                     126    CAPS_KEY        EQU     58              ; SCAN CODE FOR SHIFT KEY
    002A                     127    LEFT_KEY        EQU     42              ; SCAN CODE FOR LEFT SHIFT
    0036                     128    RIGHT_KEY       EQU     54              ; SCAN CODE FOR RIGHT SHIFT
    0052                     129    INS_KEY         EQU     82              ; SCAN CODE FOR INSERT KEY
    0053                     130    DEL_KEY         EQU     83              ; SCAN CODE FOR DELETE KEY
                             131
                             132    ;----------------------------------------
                             133    ;       DISKETTE DATA AREAS             ;
                             134    ;----------------------------------------
  003E ??                    135    SEEK_STATUS     DB      ?               ; DRIVE RECALIBRATION STATUS
                             137                                            ; BIT 3-0 = DRIVE 3-0 NEEDS RECAL BEFORE
                             137                                            ; NEXT SEEK IF BIT IS = 0
                             138
    0080                     139    INT_FLAG        EQU     080H            ; INTERRUPT OCCURENCE FLAG
  003F ??                    140    MOTOR_STATUS    DB      ?               ; MOTOR STATUS
                             141                                            ; BIT 3-0 = DRIVE 3-0 IS CURRENTLY
                             142                                            ; RUNNING
                             143                                            ; BIT 7   = CURRENT OP IS A WRITE,
                             144                                            ;   REQUIRES DELAY
                             145
  0040 ??                    146    MOTOR_COUNT     DB      ?               ; TIME OUT COUNTER FOR DRIVE TURN OFF
    0025                     147    MOTOR_WAIT      EQU     37              ; TWO SEC OF COUNT FOR MOTOR TURN OFF
                             148
  0041 ??                    149    DISKETTE_STATUS DB      ?               ; BYTE OF RETURN CODE INFO FOR STATUS
    0080                     150    TIME_OUT        EQU     80H             ; ATTACHMENT FAILED TO RESPOND
    0040                     151    BAD_SEEK        EQU     40H             ; SEEK OPERATION FAILED
    0020                     152    BAD_NEC         EQU     20H             ; NEC CONTROLLER HAS FAILED
    0010                     153    BAD_CRC         EQU     10H             ; BAD CRC ON DISKETTE READ
    0009                     154    DMA_BOUNDARY    EQU     09H             ; ATTEMPT TO DMA ACROSS 64K BOUNDARY
    0008                     155    BAD_DMA         EQU     08H             ; DMA OVERRUN ON OPERATION
    0004                     156    RECORD_NOT_FND  EQU     04H             ; REQUESTED SECTOR NOT FOUND
    0003                     157    WRITE_PROTECT   EQU     03H             ; WRITE ATTEMPTED ON WRITE PROT DISK
    0002                     158    BAD_ADDR_MARK   EQU     02H             ; ADDRESS MARK NOT FOUND
    0001                     159    BAD_CMD         EQU     01H             ; BAD COMMAND PASSED TO DISKETTE I/O
                             160
  0042 (?                    161    NEC_STATUS      DB      7 DUP(?)        ; STATUS BYTES FROM NEC
       ??
       )
                             162
                             163    ;----------------------------------------
                             164    ;       VIDEO DISPLAY AREA              :
                             165    ;----------------------------------------
  0049 ??                    166    CRT_MODE        DB      ?               ; CURRENT CRT MODE
  004A ????                  167    CRT_COLS        DW      ?               ; NUMBER OF COLUMNS ON SCREEN
  004C ????                  168    CRT_LEN         DW      ?               ; LENGTH OF REGEN IN BYTES
  004E ????                  169    CRT_START       DW      ?               ; STARTING ADDRESS IN REGEN BUFFER
  0050 (8                    170    CURSOR_POSN     DW      8 DUP(?)        ; CURSOR FOR EACH OF UP TO 8 PAGES
       ????
       )
  0060 ????                  171    CURSOR_MODE     DW      ?               ; CURRENT CURSOR MOD SETTING
  0062 ??                    172    ACTIVE_PAGE     DB      ?               ; CURRENT PAGE BEING DISPLAYED
  0063 ????                  173    ADDR_6845       DW      ?               ; BASE ADDRESS FOR ACTIVE DISPLAY CARD
  0065 ??                    174    CRT_MODE_SET    DB      ?               ; CURRENT SETTING OF THE 3X8 REGISTER
  0066 ??                    175    CRT_PALETTE     DB      ?               ; CURRENT PALETTE SETTING COLOR CARD
                             176
                             177    ;----------------------------------------
                             178    ;       POST DATA AREA                  :
                             179    ;----------------------------------------
  0067 ????                  180    IO_ROM_INIT     DW      ?               ; PNTR TO OPTIONAL I?O ROM INIT ROUTINE
  0069 ????                  181    IO_ROM_SEG      DW      ?               ; POINTER TO IO ROM SEGMENT
  006B ??                    182    INTR_FLAG       DB      ?               ; FLAG TO INDICATE AN INTERRUPT HAPPEND
                             183
                             184    ;----------------------------------------
                             185    ;           TIMER DATA AREA             :
                             186    ;----------------------------------------
  006C ????                  187    TIMER_LOW       DW      ?               ; LOW WORD OF TIMER COUNT
  006E ????                  188    TIMER_HIGH      DW      ?               ; HIGH WORD OF TIMER COUNT
  0070 ??                    189    TIMER_OFL       DB      ?               ; TIMER HAS ROLLED OVER SINCE LAST READ
                             190    ; COUNTS_SEC     EQU     18
                             191    ; COUNTS_MIN     EQU     1092
                             192    ; COUNTS_HOUR    EQU     65543
                             193    ; COUNTS_DAY     EQU     1573040 = 1800B0H
                             194
                             195    ;----------------------------------------
                             196    ;          SYSTEM DATA AREA             :
                             197    ;----------------------------------------
  0071 ??                    198    BIOS_BREAK      DB      ?               ; BIT 7 = 1 IF BREAK KEY WAS DEPRESSED
  0072 ????                  199    RESET_FLAG      DW      ?               ; WORD = 1234H IF KB RESET UNDERWAY
                             200    ;----------------------------------------
                             201    ;         FIXED DISK DATA AREA          :
                             202    ;----------------------------------------
  0074 ????                  203                    DW      ?               ;
  0076 ????                  204                    DW      ?               ;
                             205    ;----------------------------------------
                             206    ;       PRINTER AND RS232 TIMEOUT CTRS  :
                             207    ;----------------------------------------
  0078 (4                    208    PRINT_TIM_OUT   DB      4 DUP(?)        ; PRINTER TIME OUT COUNTER
       ??
       )
  007C (4                    209    RS232_TIM_OUT   DB      4 DUP(?)        ; RS232 TIME OUT COUNTER
       ??
       )
                             210    ;----------------------------------------
                             211    ;       ADDITIONAL KEYBOARD DATA AREA   :
                             212    ;----------------------------------------
  0080 ????                  213    BUFFER_START    DW      ?
  0082 ????                  214    BUFFER_END      DW      ?
  ----                       215    DATA    ENDS
                             216    ;----------------------------------------
                             217    ;          EXTRA DATA AREA              :
                             218    ;----------------------------------------
  ----                       219    XXDATA          SEGMENT AT 50H
  0000 ??                    220    STATUS_BYTE     DB      ?
  ----                       221    XXDATA          ENDS
                             222    ;----------------------------------------
                             223    ;       VIDEO DISPLAY BUFFER            :
                             224    ;----------------------------------------
  ----                       225    VIDEO_RAM       SEGMENT AT 0B800H
  0000                       226    REGEN           LABEL   BYTE
  0000                       227    REGENW          LABEL   WORD
  0000 (16385                228                    DB      16384 DUP(?)
       ??
       )
  ----                       229    VIDEO_RAM       ENDS
                             230    ;----------------------------------------
                             231    ;         ROM RESIDENT CODE             :
                             232    ;----------------------------------------
  ----                       233    CODE            SEGMENT AT 0F000H
  0000 (57344                234                    DB      57344 DUP(?)                    ; FILL LOWEST 56K
       ??
       )
                             235
  E000 31353031343736        236                    DB      '1501512 COPR. IBM 1981'                ; COPYRIGHT NOTICE
       20434F50522E20
       49424D20313938
       32
                             237
                             238
                             239    ;----------------------------------------------------------------
                             240    ;       INITIAL RELIABILITY TESTS -- PHASE 1                    :
                             241    ;----------------------------------------------------------------
                             242
                             243                    ASSUME    CS:CODE,SS:CODE,ES:ABS0,DS:DATA
                             244
                             245    ;----------------------------------------
                             246    ;       DATA DEFINITIONS                :
                             247    ;----------------------------------------
                             248
  E016 D1E0                  249    C1      DW      C11                     ; RETURN ADDRESS
  E018 7EE1                  250    C2      DW      C24                     ; RETURN ADDRESS FOR DUMMY STACK
                             251
  E01A 204B42204F4B          252    F3B     DB      ' KB OK',13             ; KB FOR MEMORY SIZE
  E020 0D
                             253
                             254    ;---------------------------------------------------------------------
                             255    ;       LOAD A BLOCK OF TEST CODE THROUGH THE KEYBOARD PORT          :
                             256    ;       FOR MANUFACTUING TEST.                                       :
                             257    ;       THIS ROUTINE WILL LOAD A TEST (MAX LENGTH=FAFFH) THROUGH     :
                             258    ;       THE KEYBOARD PORT. COD WILL BE LOADED AT LOCATION            :
                             259    ;       0000:0500. AFTER LOADING, CONTROL WILL BE TRANSFERED         :
                             260    ;       TO LOCATION 0000:0500. STACK WILL BE LOCATED JUST BELOW      :
                             261    ;       THE TEST CODE. THIS ROUTINE ASSUMES THAT THE FIRST 2         :
                             262    ;       BYTES TRANSFERED CONTAIN THE COUNT OF BYTES TO BE LOADED     :
                             263    ;       (BYTE 1=COUNT LOW, BYTE 2=COUNT HI.)                         :
                             264    ;---------------------------------------------------------------------
                             265
                             266    ;----- FIRST, GET THE COUNT
                             267
  E021                       268    MFG_BOOT:
  E021 E8131A                269            CALL    SP_TEST                ; GET COUNT LOW
  E024 8AFB                  270            MOV     BH,BL                  ; SAVE IT
  E026 E80E1A                271            CALL                           ; GET COUNT HI
  E029 8AEB                  272            MOV     CH,BL
  E02B 8ACF                  273            MOV     CL,BH                  ; CX NOW HAS COUNT
  E02D FC                    274            CLD                            ; SET DIR. FLAG TO INCREMENT
  E02E FA                    275            CLI
  E02F BF0005                276            MOV     DI,0500H               ; SET TARGET OFFSET (DS=0000)
  E032 B0FD                  277            MOV     AL,0FDH                ; UNMASK K/B INTERRUPT
  E034 E621                  278            OUT     INTA01,AL
  E036 B00A                  279            MOV     AL,0AH                 ; SEND READ INT. REQUEST REG. CMD
  E038 E620                  280            OUT     INTAC0,AL
  E03A BA6100                281            MOV     DX,61H                 ; SET UP PORT B ADDRESS
  E03D BBCC4C                282            MOV     BX,4CCH                ; CONTROL BITS FOR PORT B
  E040 B402                  283            MOV     AH,02H                 ; K/B REQUEST PENDING MAKS
  E042                       284    TST:
  E042 8AC3                  285            MOV     AL,BL
  E044 EE                    286            OUT     DX,AL                  ; TOGGLE K/B CLOCK
  E045 8AC7                  287            MOV     AL,BH
  E047 EE                    288            OUT     DX,AL
  E048 4A                    289            DEC     DX                     ; POINT DX AT ADDR. 60 (KB DATA)
  E049                       290    TST1:
  E049 E420                  291            IN      AL,INTA00              ; GET IRR REG
  E04B 22C4                  292            AND     AH,AH                  ; KB REQUEST PENDING?
  E040 74FA                  293            JZ      TST1                   ; LOOP TILL DATA PRESENT
  E04F EC                    294            IN      AL,DX                  ; GET DATA
  E050 AA                    295            STOSB                          ; STORE IT
  E051 42                    296            INC     DX                     ; POINT DX BACK AT PORT B (61)
  E052 E2EE                  297            LOOP    TST                    ; LOOP TILL ALL BYTES READ
                             298
  E054 EA0005000             299            JMP     C3                     ; FAR JUMP TO CODE THAT WAS JUST
                             300                                           ; LOADED
                             301
                             302    ;----------------------------------------
                             303    ;       8088 PROCESSOR TEST             :
                             304    ; DESCRIPTION                           :
                             305    ;       VERIFY 8088 FLAGS, REGISTERS    :
                             306    ;       AND CONDITIONAL JUMPS           :
                             307    ;----------------------------------------
                             308            ASSUME  CS:CODE,DS:NOTHING,ES:NOTHING,SS:NOTHING
  E05B                       309            ORG     0E05BH
  E05B                       310    RESET   LABEL   FAR
  E05B FA                    311    START:   CLI                            ; DISABLE INTERRUPTS
  E05C B4D5                  312             MOV     AH,0D5H                ; SET SF, CF, ZF, AND AF FLAGS ON
  E05E 9E                    313             SAHF
  E05F 734C                  314             JNC     ERR01                  ; GO TO ERR ROUTINE IF CF NOT SET
  E061 754A                  315             JNZ     ERR01                  ; GO TO ERR ROUTINE IF ZF NOT SET
  E063 7B48                  316             JNP     ERR01                  ; GO TO ERR ROUTINE IF PF NOT SET
  E065 7946                  317             JNS     ERR01                  ; GO TO ERR ROUTINE IF SF NOT SET
  E067 9F                    318             LAHF                           ; LOAD FLAG IMAGE TO AH
  E068 B105                  319             MOV     CL,S                   ; LOAD CNT REG WITH SHIFT CNT
  E06A D2EC                  320             SHR     AH,CL                  ; SHIFT AF INTO CARRY BIT POS
  E06C 733F                  321             JNC     ERR01                  ; GO TO ERR ROUTINE IF AF NOT SET
  E06E B040                  322             MOV     AL,40H                 ; SET THE OF FLAG ON
  E070 D0E0                  323             SHL     AL,1                   ; SETUP FOR TESTING
  E072 7139                  324             JNO     ERR01                  ; GO TO ERR ROUTINE IF OF NOT SET
  E074 32E4                  325             XOR     AH,AH                  ; SET AH = 0
  E076 9E                    326             SAHF                           ; CLEAR SF, CF, ZF, AND PF
  E077 7634                  327             JBE     ERR01                  ; GO TO ERR ROUTINE IF CF ON
                             328                                            ; OR TO TO ERR ROUTINE IF ZF ON
  E079 7832                  329             JS      ERR01                  ; GO TO ERR ROUTINE IF SF ON
  E07B 7A30                  330             JP      ERR01                  ; GO TO ERR ROUTINE IF PF ON
  E07D 9F                    331             LAHF                           ; LOAD FLAG IMAGE TO AH
  E07E B105                  332             MOV     CL,5                   ; LOAD CNT REG WITH SHIFT CNT
  E080 D2EC                  333             SHR     AH,CL                  ; SHIFT 'AF' INTO CARRY BIT POS
  E082 7229                  334             JC      ERR01                  ; GO TO ERR ROUTINE IF ON
  E084 D0E4                  335             SHL     AH,1                   ; CHECK THAT 'OF' IS CLEAR
  E086 7025                  336             JO      ERR01                  ; GO TO ERR ROUTINE IF ON
                             337
                             338    ;----- READ/WRITE THE 8088 GENERAL AND SEGMENTATION REGISTERS
                             339    ;        WITH ALL ONE'S AND ZEROES'S.
                             340
  E088 B8FFFF                341            MOV    AX,0FFFFH                ; SETUP ONE'S PATTERN IN AX
  E08B F9                    342            STC
  E08C 8ED8                  343    C8:     MOV    DS,AX                    ; WRITE PATTERN TO ALL REGS
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
                             362    ;----------------------------------------
                             363    ;       ROS CHECKSUM TEST I             :
                             364    ; DESCRIPTION                           :
                             365    ;       A CHECKSUM IS DONE FOR THE 8K   :
                             366    ;       ROS MODULE CONTAINING POD AND   :
                             367    ;       BIOS.                           :
                             368    ;----------------------------------------
  E0AE                       369    C10:
                             370                                            ; ZERO IN AL ALREADY
  E0AE E6A0                  371            OUT     0A0H,AL                 ; DISABLE NMI INTERRUPTS
  E0B0 E683                  372            OUT     83H,AL                  ; INITIALZE DMA PAGE REG
  E0B2 BAD803                373            MOV     DX,3D8H
  E0B5 EE                    374            OUT     DX,AL                   ; DISABLE COLOR VIDEO
  E0B6 FEC0                  375            INC     AL
  E0B8 B2B8                  376            MOV     DL,0B8H
  E0BA EE                    377            OUT     DX,AL                   ; DISABLE B/W VIDEO,EN HIGH RES
  E0BB B089                  378            MOV     AL,99H                  ; SET 8255 A,C-INPUT,B-OUTPUT
  E0BD E663                  379            OUT     CMD_PORT,AL
  E0BF B0A5                  380            MOV     AL,10100101B
                             381                                            ; ENABLE PARITY CHECKERS AND
  E0C1 E661                  382            OUT     PORT_B,AL               ; PULL KB CLOCK HI, TRI-STATE
                             383                                            ; KEYBOARD INPUTS,ENABLE HIGH
                             384                                            ; BANK OF SWITCHES->PORT C(0-3)
  E0C3 B001                  385            MOV     AL,01H                  ; <><><><><><><><><><><><>
  E0C5 E660                  386            OUT     PORT_A,AL               ; <><><>CHECKPOINT 1<><><>
  E0C7 8CC8                  387            MOV     AX,CS                   ; SETUP SS SEG REG
  E0C5 8ED0                  388            MOV     SS,AX
  E0C7 8ED8                  389            MOV     DS,AX                   ; SET UP DATA SEG TO POINT TO
                             390                                            ; ROM ADDRESS
  E0CD FC                    391            CLD                             ; SET DIRECTION FLAG TO INC.
                             392            ASSUME  SS:CODE
  E0CE BB00E0                393            MOV     BX,0E000H               ; SETUP STARTING ROS ADDR
  E0D1 BC16E0                394            MOV     SP,OFFSET C1            ; SETUP RETURN ADDRESS
  E0D4 E91B1B                395            JMP     ROS_CHECKSUM
  E0D7 7504                  396    C11:    JNE     ERR01                   ; HALT SYSTEM IF ERROR
                             397    ;--------------------------------------------------------
                             398    ;       8237 DMA INITIALIZATION CHANNEL REGISTER TEST   :
                             399    ; DESCRIPTION                                           :
                             400    ;       DISABLE THE 8237 DMA CONTROLLER.  VERIFY THAT   :
                             401    ;       FUNCTIONS OK. WRITE/READ THE CURRENT ADDRESS    :
                             402    ;       COUNT REGISTERS FOR ALL CHANNELS.  INITIALIZE   :
                             403    ;       START DMA FOR MEMORY REFRESH.                   :
                             404
                             405    ;--------------------------------------------------------
                             406
                             407    ;----- DISABLE DMA CONTROLLER
                             408
  E0D9 B002                  409            MOV     AL,02H                  ; <><><><><><><><><><><><>
  E0DB E660                  410            OUT     PORT_A,AL               ; <><><><><><><><><><><><>
  E0DD B004                  411            MOV     AL,04                   ; DISABLE DMA CONTROLLER
  E0DF E608                  412            OUT     DMA08,AL
                             431
                             414    ;----- VERIFY THAT TIMER 1 FUNCTIONS OK
                             415
  E0E1 B054                  416            MOV     AL,54H                  ; SEL TIMER 1,LSB,MODE 2
  E0E3 E643                  417            OUT     TIMER+3,AL
  E0E5 8AC1                  418            MOV     AL,CL                   ; SET INITIAL TIMER CNT TO 0
  E0E7 E641                  429            OUT     TIMER+1,AL
  E0E9                       420    C12:                                    ; TIMER1_BITS_ON
  E0E9 B040                  421            MOV     AL,40H                  ; LATCH TIMER 1 COUNT
  E0EB E643                  422            OUT     TIMER+3,AL
  E0ED 80FBFF                423            CMP     BL,0FFH                 ; YES - SEE IF ALL BITS GO OFF
  E0F0 7407                  424            JE      C13                     ; TIMER1_BITS_OFF
  E0F2 E441                  425            IN      AL,TIMER+1              ; READ TIMER 1 COUNT
  E0F4 0AD8                  426            OR      BL,AL                   ; ALL BITS ON IN TIMER
  E0F6 E2F1                  427            LOOP    C12                     ; TIMER1_BITS_ON
  E0F8 F4                    428            HLT                             ; TIMER 1 FAILURE, HALT SYS
  E0F9                       429    C13:                                    ; TIMER1_BITS_OFF
  E0F9 8AC3                  430            MOV     AL,BL                   ; SET TIMER 1 CNT
  E0FB 2BC9                  431            SUB     CX,CX
  E0FD E641                  432            OUT     TIMER+1,AL
  E0FF                       433    C14:                                    ; TIMER_LOOP
  E0FF B040                  434            MOV     AL,40H                  ; LATCH TIMER 1 COUNT
  E101 E643                  435            OUT     TIMER+3,AL
  E103 90                    436            NOP                             ; DELAY FOR TIMER
  E104 90                    437            NOP
  E105 E441                  438            IN      AL,TIMER+1              ; READ TIMER 1 COUNT
  E107 22D8                  439            AND     BL,AL
  E109 7403                  440            JZ      C15                     ; GO TO WRAP_DMA_REG
  E10B E2F2                  441            LOOP    C14                     ; TIMER_LOOP
  E10D F4                    442            HLT                             ; HALT SYSTEM
                             443
                             444    ;----- INITIALIZE TIMER 1 TO REFRESH MEMORY
                             445
  E10E B003                  446    C15:    MOV     AL,03H                  ; <><><><><><><><><><><><>
  E110 E660                  447            OUT     PORT_A,AL               ; M<M<M<CHECKPOINT 3<><><>
                             448                                            ; WRAP_DMA_REG
  E112 E60D                  449            OUT     DMA+0DH,AL              ; SEND MASTER CLEAR TO DMA
                             450
                             451    ;----- WRAP DMA CHANNELS ADDRESS AND COUNT REGISTERS
                             452
  E114 B0FF                  453            MOV     AL,0FFH                 ; WRITE PATTERN FF TO ALL REGS
  E116 8AD8                  454    C16:    MOV     BL,AL                   ; SAVE PATTERN FOR COMPARE
  E118 8AF8                  455            MOV     BH,AL
  E11A B90800                456            MOV     CX,8                    ; SETUP LOOP CNT
  E11D BA0000                457            MOV     DX,DMA                  ; SETUP I/O PORT ADDR OF REG
  E120 EE                    458    C17:    OUT     DX,AL                   ; WRITE PATTERN TO REG, LSB
  E121 50                    459            PUSH    AX                      ; SATISFY 8237 I/O TIMINGS
  E122 EE                    460            OUT     DX,AL                   ; MSB OF 16 BIT REG
  E123 B001                  461            MOV     AX,01H                  ; AX TO ANOTHER PAT BEFORE RD
  E125 EC                    462            IN      AL,DX                   ; READ 16-BIT DMA CH REG, LSB
  E126 8AE0                  463            MOV     AH,AL                   ; SAVE LSB OF 16-BIT REG
  E128 EC                    464            IN      AL,DX                   ; READ MSB OF DMA CH REG
  E129 3BD8                  465            CMP     BX,AX                   ; PATTERN READ AS WRITTEN?
  E12B 7401                  466            JE      C18                     ; YES - CHECK NEXT REG
  E12D F4                    467            HLT                             ; NO - HALT THE SYSTEM
  E12E                       468    C18:                                    ; NXT_DMA_CH
  E12E 42                    469            INC     DX                      ; SET I/O PORT TO NEXT CH REG
  E12F E2EF                  470            LOOP    C17                     ; WRITE PATTERN TO NEXT REG
  E131 FEC0                  471            INC     AL                      ; SET PATTERN TO 0
  E133 74E1                  472            JZ      C16                     ; WRITE TO CHANNEL REGS
                             473
                             474    ;----- INITIALIZE AND START DMA FOR MEMORY REFRESH.
                             475
  E135 8EDB                  476            MOV     DS,BX                   ; SET UP ABS0 INTO DS AND ES
  E137 8EC3                  477            MOV     ES,BX
                             478            ASSUME  DS:ABS0,ES:ABS0
  E139 B0FF                  479            MOV     AL,0FFH                 ; SET CNT OF 64K FOR REFRESH
  E13B E601                  480            OUT     DMA+1,AL
  E13D 50                    481            PUSH    AX
  E13E E601                  482            OUT     DMA+1,AL
  E140 B058                  483            MOV     AL,058H                 ; SET DMA MODE,CH 0,RD.,AUOTINT
  E142 E60B                  484            OUT     DMA+0BH,AL              ; WRITE DMA MODE REG
  E144 B000                  485            MOV     AL,0                    ; ENABLE DMA CONTROLLER
  E146 8AE8                  486            MOV     CH,AL                   ; SET COUNT HIGH=00
  E148 E608                  487            OUT     DMA+8,AL                ; SETUP DMA COMMAND REG
  E14A 50                    488            PUSH    AX
  E14B E60A                  489            OUT     DMA+10,AL               ; ENABLE DMA CH 0
  E14D B012                  490            MOV     AL,18                   ; START TIMER 1
  E14F E641                  491            OUT     TIMER+1,AL
  E151 B041                  492            MOV     AL,41H                  ; SET MODE FOR CHANNEL 1
  E153 E60B                  493            OUT     DMA+0BH,AL
  E155 50                    494            PUSH    AX
  E156 E408                  495            IN      AL,DMA+08               ; GET DMA STATUS
  E158 2410                  496            AND     AL,00010000B            ; IS TIMER REQUEST THERE?
  E15A 7401                  497            JZ      C18C                    ; (IT SHOULD'T BE)
  E15C F4                    498            HLT                             ; HALT SYS.(HOT TIMER 1 OUTPUT)
  E15D B042                  499    C18A:   MOV     AL,42H                  ; SET MODE FOR CHANNEL 5
  E15F E60B                  500            OUT     DMA+0BH,AL
  E161 B043                  501            MOV     AL,43H                  ; SET MODE FOR CHANNEL 3
  E163 E60B                  502            OUT     DMA+0BH,AL
                             503    ;------------------------------------------------
                             504    ;       BASE 16K READ/WRITE STORAGE TEST        :
                             505    ; DESCRIPTION                                   :
                             506    ;       WRITE/READ/VERIFY DATA PATTERNS         :
                             507    ;       AA,55,FF,01, AND 00 TO 1ST 32K OF       :
                             508    ;       STORAGE. VERIFY STORAGE ADDRESSABILITY. :
                             509    ;------------------------------------------------
                             510
                             511    ;----- DETERMINE MEMORY SIZE AND FILL MEMORY WITH DATA
                             512
  E165 BA1302                513            MOV     DX,0213H                ; ENABLE I/O EXPANSION BOX
  E168 B001                  514            MOV     AL,01H
  E16A EE                    515            OUT     DX,AL
                             516
  E16B 8B1E7204              517            MOV     BX,DATA_WORD[OFFSET RESET_FLAG] ; SAVE 'RESET_FLAG' IN BX
  E16F B90020                518            MOV     CX,2000H                ; SET FOR 16K WORDS
  E172 81FB3412              519            CMP     BX,1234H                ; WARM START?
  E176 7416                  520            JE      CLR_STG
  E178 BC18E0                521            MOV     SP,OFFSET C2
  E17B E9F104                522            JMP     STGTST_CNT
  E17E 7412                  523    C24:    JE      HOW_BIG                 ; STORAGE OK, DETERMINE SIZE
  E180 0AD8                  524            MOV     BL,AL                   ; SAVE FAILING BIT PATTERN
  E182 B004                  525            MOV     AL,04H                  ; <><><><><><><><><><><><>
  E184 E660                  526    C24A:   OUT     PORT_A,AL               ; <><><>CHECKPOINT 4<><><>
  E186 2BC9                  527            SUB     CX,CX                   ; BASE RAM FAILURE - HANG
  E188 E2FE                  528    C24B:   LOOP    C24B                    ; FLIPPING BETWEEN 04 AND
  E18A 86D8                  529            XCHG    BL,AL                   ; FAILING BIT PATTERN
  E18C EBF6                  530            JMP     C24A
  E18E                       531    CLR_STG:
  E18E 2BC0                  532            SUB     AX,AX                   ; MAKE AX=0000
  E190 F3                    533            REP     STOSW                   ; STORE 8K WORDS OF 0000
  E191 AB
  E192                       534    HOW_BIG:
  E192 891E7204              535            MOV     DATA_WORD[OFFSET RESET_FLAG],BX ; RESTORE RESET FLAG
  E196 BA0004                536            MOV     DX,0400H                ; SET POINTER TO JUST>16KB
  E199 8B1000                537            MOV     BX,16                   ; BASIC COUNT OF 16K
  E19C                       538    FILL_LOOP:
  E19C 8EC2                  539            MOV     ES,DX                   ; SET SEG REG.
  E19E 2BFF                  540            SUB     DI,DI
  E1A0 B855AA                541            MOV     AX,0AA55H               ; TEST PATTERN
  E1A3 8BC8                  542            MOV     CX,AX                   ; SAVE PATTERN
  E1A5 2689D5                543            MOV     ES:[DI],AX              ; SEND PATTERN TO MEM.
  E1A8 B00F                  544            MOV     AL,0FH                  ; PUT SOMETHING IN AL
  E1AA 268B05                545            MOV     AX,ES:[DI]              ; GET PATTERN
  E1AD 33C1                  546            XOR     AX,CX                   ; COMPARE PATTERNS
  E1AF 7511                  547            JNZ     HOW_BIG_END             ; GO END IF NO COMPARE
  E1B1 B90020                548            MOV     CX,2000H                ; SET COUNT FOR 8K WORDS
  E1B4 F3                    549            REP     STOSW                   ; FILL 8K WORDS
  E1B5 AB                    550
  E1BA 83C310                551            ADD     BX,16                   ; BUMP COUNT BY 16KB
  E1B0 80FEA0                552            CMP     DH,0A0H                 ; TP{ PF RA< AREA YET? (A0000)
  E1C0 75DA                  553            JNZ     FILL_LOOP
  E1C2                       554    HOW_BIG_END:
  E1C2 891E1304              555            MOV     DATA_WORD[OFFSET MEMORY_SIZE],BX        ; SAVE MEMORY SIZE
                             556
                             557    ;----- SETUP STACK SEG AND SP
                             558
  E1C6 B83000                559            MOV     AX,STACK                ; GET THE STACK VALUE
  E1C9 8ED0                  560            MOV     SS,AX                   ; SET THE STACK UP
  E1CB BC0001                561            MOV     SP,OFFSET TOS           ; STACK IS READY TO GO
                             562    ;----------------------------------------------------------------
                             563    ;       INITIALIZE THE 8259 INTERRUPT CONTROLLER CHIP           :
                             564    ;----------------------------------------------------------------
  E1CE B013                  565    C25:    MOV     AL,13H                  ; ICQ1 - EDGE, SNGL, ICW4
  E1D0 E620                  566            OUT     INTA00,AL
  E1D2 B008                  567            MOV     AL,8                    ; SETUP ICW2 - INT TYPE 8 (8-F)
  E1D4 E621                  568            OUT     INTA01,AL
  E1D6 B009                  569            MOV     AL,9                    ; SETUP ICW4 - BUFFRD,8086 MODE
  E1D8 E621                  570            OUT     INTA01,AL
  E1DA B0FF                  571            MOV     AL,0FFH                 ; MASK ALL INTS. OFF
  E1DC E621                  572            OUT     INTA01,AL               ; (VIDEO ROUTINE ENABLES INTS.)
                             573
                             574    ;---- SET UP THE INTERRUPT VECTORS TO TEMP INTERRUPT
                             575
  E1DE 1E                    576            PUSH    DS
  E1DF B92000                577            MOV     CX,32                   ; FILL ALL 32 INTERRUPTS
  E1E2 2BFF                  578            SUB     DI,DI                   ; FIRST INTERRUPT LOCATION
  E1E4 8EC7                  579            MOV     ES,DI                   ; SET ES=0000 ALSO
  E1E6 B823FF                580    D3:     MOV     AX,OFFSET D11           ; MOVE ADDR OF INTR PROC TO TBL
  E1E9 AB                    581            STOSW
  E1EA 8CC8                  582            MOV     AX,CS                   ; GET ADDR OF INTR PROC SEG
  E1EC AB                    583            STOSW
  E1ED E2F7                  584            LOOP    D3                      ; VECTBLO
                             585
                             586    ;----- ESTABLISH BIOS SUBROUTINE CALL INTERRUPT VECTORS
                             587
  E1EF BF4000                588            MOV     DI,OFFSET VIDEO_INT     ; SETUP ADDR TO INTR AREA
  E1F2 0E                    589            PUSH    CS
  E1F3 1F                    590            POP     DS                      ; SETUP ADDR OF VECTOR TABLE
  E1F4 8CD8                  591            MOV     AX,DS                   ; SET AX=SEGMENT
  E1F6 BE03FF90              592            MOV     SI,OFFSET VECTOR_TABLE+16       ; START WITH VIDEO ENTRY
  E1FA B91000                593            MOV     CX,16
  E1FD A5                    594    D3A:    MOVSW                           ; MOVE VECTOR TABLE TO RAM
  E1FE 47                    595            INC     DI                      ; SKIP SEGMENT POINTER
  E1FF 47                    596            INC     DI
  E200 E2FB                  597            LOOP    03A
                             598    ;------------------------------------------------
                             599    ;       DETERMINE CONFIGURATION AND MFG. MODE   :
                             600    ;------------------------------------------------
                             601
  E202 IF                    602            POP     DS
  E203 IE                    603            PUSH    DS                      ; RECOVER DATA SEG
  E204 E462                  604            IN      AL,PORT_C               ; GET SWITCH INFO
  E206 240F                  605            AND     AL,00001111B            ; ISOLATE SWITCHES
  E208 8AE0                  606            MOV     AH,AL                   ; SAVE
  E20A B0AD                  607            MOV     AL,10101101B            ; ENABLE OTHER BANK OF SWS.
  E20C E661                  608            OUT     PORT_B,AL
  E20E 90                    609            NOP
  E20F E462                  610            IN      AL,PORT_C
  E211 B104                  611            MOV     CL,4
  E213 D2C0                  612            ROL     AL,CL                   ; ROTATE TO HIGH NIBBLE
  E215 24F0                  613            AND     AL,11110000B            ; ISOLATE
  E217 0AC4                  614            OR      AL,AH                   ; COMBINE WITH OTHER BANK
  E219 2AE4                  615            SUB     AH,AH
  E21B A31004                616            MOV     DATA_WORD[OFFSET EQUIP_FLAG],AX ; SAVE SWITCH INFO
  E21E B099                  617            MOV     AL,99H
  E220 E663                  618            OUT     CMD_PORT,AL
  E222 E80518                619            CALL    KBD_RESET               ; SEE IF MFG. JUMPER IN
  E225 80FBAA                620            CMP     BL,0AAH                 ; KEYBOARD PRESENT?
  E228 7418                  621            JE      E6
  E22A 80FB65                622            CMP     BL,065H                 ; LOAD MFG. TEST REQUEST?
  E22D 7503                  623            JNE     D3B
  E22F E9EFFD                624            JMP     MFG_BOOT                ; GO TO BOOTSTRAP IF SO
  E232 B038                  625    D3B:    MOV     AL,38H
  E234 E661                  626            OUT     PORT_B,AL
  E236 90                    627            NOP
  E237 90                    628            NOP
  E238 E460                  629            IN      AL,PORT_A
  E23A 24FF                  630            AND     AL,0FFH                 ; WAS DATA LINE GROUNDED
  E23C 7504                  631            JNZ     E6
  E23E FE061204              632                    DATA_AREA[OFFSET MFG_TST]       ; SET MANUFACTURING TEST FLAG
                             633
                             634    ;--------------------------------------------------------
                             635    ;       INITIALIZE AND START CRT CONTROLLER (6845)      :
                             636    ;       TEST VIDEO READ/WRITE STORAGE.                  :
                             637    ; DESCRIPTION                                           :
                             638    ;       RESET THE VIDEO ENABLE SIGNAL.                  :
                             639    ;       SELECT ALPHANUMERIC MODE, 40 * 25, B & W.       :
                             640    ;       READ/WRITE DATA PATTERNS TO STG. CHECK STG      :
                             641    ;       ADDRESSABILITY.                                 :
                             642    ; ERROR = 1 LONG AND 2 SHORT BEEPS                      :
                             643    ;--------------------------------------------------------
  E242                       644    E6:
  E242 A11004                645            MOV     AX,DATA_WORD[OFFSET EQUIP_FLAG] ; GET SENSE SWITCH INFO
  E245 50                    646            PUSH    AX                      ; SAVE IT
  E246 B030                  647            MOV     AL,30H
  E248 A31004                648            MOV     DATA_WORD[OFFSET EQUIP_FLAG],AX
  E24B 2AE4                  649            SUB     AH,AH
  E24D CD10                  650            INT     10H                     ; SEND INIT TO B/W CARD
  E24F B020                  651            MOV     AL,20H
  E251 A31004                652            MOV     DATA_WORD[OFFSET EQUIP_FLAG],AX
  E254 2AE4                  653            SUB     AH,AH                   ; AND INIT COLOR CARD
  E256 CD10                  654            INT     10H
  E258 58                    655            POP     AX                      ; RECOVER REAL SWITCH INFO
  E259 A31004                656            MOV     DATA_WORD[OFFSET EQUIP_FLAG],AX ; RESTORE IT
                             657                                            ; AND CONTINUE
  E25C 2430                  658            AND     AL,30H                  ; ISOLATE VIDO SWS
  E25E 750A                  659            JNZ     E7                      ; VIDEO SWS SET TO 0?
  E260 BF4000                660            MOV     DI,OFFSET VIDEO_INT     ; SET INT 10H TO DUMMY
  E263 C7054BFF              661            MOV     [DI],OFFSET DUMMY_RETURN        ; RETURN IF NO VIDEO CARD
  E267 E9A000                662            JMP     E18_1                   ; BYPASS VIDEO TEST
  E26A                       663    E7:                                     ; TEST_VIDEO:
  E26A 3C30                  664            CMP     AL,30H                  ; B/W CARD ATTACHED?
  E26C 7408                  665            JE      E8                      ; YES - SET MODE FOR B/W CARD
  E26E FEC4                  666            INC     AH                      ; SET COLOR MODE FOR COLOR CD
  E270 3C20                  667            CMP     AL,20H                  ; 80X25 MODE SELECTED?
  E272 7502                  668            JNE     E8                      ; NO - SET MODE FOR 40X25
  E274 B403                  669            MOV     AH,3                    ; SET MODE FOR 80X25
  E276 86E0                  670    E8:     XCHG    AH,AL                   ; SET_MODE:
  E278 50                    671            PUSH    AX                      ; SAVE VIDEO MODE ON STACK
  E279 2AE4                  672            SUB     AH,AH                   ; INITIALIZE TO ALPHANUMERIC MD
  E27B CD10                  673            INT     10H                     ; CALL VIDEO_IO
  E27D 58                    674            POP     AX                      ; RESTORE VIDEO SENSE SWS IN AH
  E27E 50                    675            PUSH    AX                      ; RESAVE VALUE
  E27F BB00B0                676            MOV     BX,0B000H               ; BEG VIDEO RAM ADDR B/W CD
