; WARNING: THIS FILE IS NOT YET FINISHED! USE OF THIS FILE MAY CAUSE UNEXPECTED RESULTS AND IN EXTREME CIRCUMSTANCES MAY CAUSE DATA CORRUPTION!
                             1                  PAGE,120
                             2                  TITLE   ENHANCED GRAPHICS ADAPTER BIOS
                             3                  EXTRN   CGMN:NEAR, CGDDOT:NEAR, INT_1F_1:NEAR, CGMN_FDG:NEAR
                             4                  EXTRN   END_ADDRESS:NEAR
                             5
                             6          ;----------------------------------------------------------------
                             7          ;       THE  BIOS ROUTINES ARE MEANT TO BE ACCESSED THROUGH     :
                             8          ;       SOFTWARE INTERRUPTS ONLY.  ANY ADDRESSES PRESENT IN     :
                             9          ;       THE LISTINGS  ARE INCLUDED  ONLY FOR  COMPLETENESS,     :
                             10         ;       NOT FOR  REFERENCE.   APPLICATIONS WHICH  REFERENCE     :
                             11         ;       ABSOLUTE   ADDRESSES   WITHIN   THE   CODE  SEGMENT     :
                             12         ;       VIOLATE THE STRUCTURE AND DESIGN OF BIOS.               :
                             13         ;----------------------------------------------------------------
                             14
                             15         .LIST
                             16      C          INCLUDE         VFRONT.INC
                             17      C          SUBTTL  VFRONT.INC
                             18      C
                             19      C
                             20      C  ;--- INT 10 -------------------------------------------------------------
                             21      C  ; VIDEO_IO                                                              :
                             22      C  ;       THESE ROUTINES PROVIDE THE CRT INTERFACE                        :
                             23      C  ;       THE FOLLOWING FUNCTIONS ARE PROVIDED:                           :
                             24      C  ;       (AH)=0  SET MODE (AL) CONTAINS MODE VALUE                       :
                             25      C  ;                                                                       :
                             26      C  ;       AL AD   TYPE    RES      NOTES  DF-DIN  DISPLAY      MAX PGS    :
                             27      C  ;       -------------------------------------------------------------   :
                             28      C  ;     *  0 B8   ALPHA   640X200         40X25   COLOR - BW      8       :
                             29      C  ;        1 B8   ALPHA   640X200         40X25   COLOR           8       :
                             30      C  ;     *  2 B8   ALPHA   640X200         80X25   COLOR - BW      8       :
                             31      C  ;        3 B8   ALPHA   640X200         80X25   COLOR           8       :
                             32      C  ;        4 B8   GRPHX   320X200         40X25   COLOR           1       :
                             33      C  ;        5 B8   GRPHX   320X200         40X25   COLOR - BW      1       :
                             34      C  ;        6 B8   GRPHX   640X200         80X25   COLOR - BW      1       :
                             35      C  ;     *  7 B0   GRPHX   720X350         80X25   MONOCHROME      8       :
                             36      C  ;                                                                       :
                             37      C  ;        8      RESERVED                                                :
                             38      C  ;        9      RESERVED                                                :
                             39      C  ;        A      RESERVED                                                :
                             40      C  ;        B      RESERVED - INTERNAL USE                                 :
                             41      C  ;        C      RESERVED - INTERNAL USE                                 :
                             42      C  ;                                                                       :
                             43      C  ;        D A0   GRPHX   320X200         40X25   COLOR           8       :
                             44      C  ;        E A0   GRPHX   640X200         80X25   COLOR           4       :
                             45      C  ;        F A0   GRPHX   640X350         80X25   MONOCHROME      2       :
                             46      C  ;       10 A0   GRPHX   640X350         80X25   HI RES          2       :
                             47      C  ;                                                                       :
                             48      C  ;                                                                       :
                             49      C  ;               NOTE : HIGH BIT AL SET PREVENTS REGEN BUFFER CLEAR ON   :
                             50      C  ;                       MODES RUNNING ON THE COMBO VIDEO ADAPTER        :
                             51      C  ;                                                                       :
                             52      C  ;               *** NOTE BW MODES OPERATE SAME AS COLOR MODES, BUT      :
                             53      C  ;                        COLOR BURST IS NOT ENABLED                     :
                             54      C  ;       (AH)=1  SET CURSOR TYPE                                         :
                             55      C  ;               (CH) =  BITS 4-0 = START LINE FOR CURSOR                :
                             56      C  ;                       ** HARDWARE WILL ALWAYS CAUSE BLINK             :
                             57      C  ;                       ** SETTING BIT 5 OR 6 WILL CAUSE ERRATIC        :
                             58      C  ;                          BLINKING OR NO CURSOR AT ALL                 :
                             59      C  ;               (CL) =  BITS 4-0 = END LINE FOR CURSOR                  :
                             60      C  ;       (AH)=2  SET CURSOR POSITION                                     :
                             61      C  ;               (DH,DL) = ROW,COLUMN  (0,0) IS UPPER LEFT               :
                             62      C  ;               (BH) = PAGE NUMBER                                      :
                             63      C  ;       (AH)=3  READ CURSOR POSITION                                    :
                             64      C  ;               (BH) = PAGE NUMBER                                      :
                             65      C  ;               ON EXIT (DH,DL) = ROW,COLUMN OF CURRENT CURSOR          :
                             66      C  ;                       (CH,CL) = CURSOR MODE CURRENTLY SET             :
                             67      C  ;       (AH)=4  READ LIGHT PEN POSITION                                 :
                             68      C  ;               ON EXIT:                                                :
                             69      C  ;               (AH) = 0 -- LIGHT PEN SWITCH NOT DOWN/NOT TRIGGERED     :
                             70      C  ;               (AH) = 1 -- VALID LIGHT PEN VALUE IN REGISTERS          :
                             71      C  ;                       (DH,DL) = RASTER LINE (0-999)                   :
                             72      C  ;                       (CH) = RASTER LINE (0-199)                      :
                             73      C  ;                       (CX) = RASTER LINE (0-NNN) NEW GRAPHICS MODES   :
                             74      C  ;                       (BX) = PIXEL COLUMN (0-319,639)                 :
                             75      C  ;       (AH)=5  SELECT ACTIVE DISPLAY PAGE                              :
                             76      C  ;               (AL) = NEW PAGE VALUE, SEE AH=0 FOR PAGE INFO           :
                             77      C  ;       (AH)=6  SCROLL ACTIVE PAGE UP                                   :
                             78      C  ;               (AL) = NUMBER OF LINES, INPUT LINES BLANKED AT BOTTOM   :
                             79      C  ;                      OF WINDOW                                        :
                             80      C  ;                       AL = 0 MEANS BLANK ENTIRE WINDOW                :
                             81      C  ;               (CH,CL) = ROW,COLUMN OF UPPER LEFT CORNER OF SCROLL     :
                             82      C  ;               (DH,DL) = ROW,COLUMN OF LOWER RIGHT CORNER OF SCROLL    :
                             83      C  ;               (BH) = ATTRIBUTE TO BE USED ON BLANK LINE               :
                             84      C  ;       (AH)=7  SCROLL ACTIVE PAGE DOWN                                 :
                             85      C  ;               (AL) = NUMBER OF LINES, INPUT LINES BLANKED AT TOP      :
                             86      C  ;                      OF WINDOW                                        :
                             87      C  ;                       AL = 0 MEANS BLANK ENTIRE WINDOW                :
                             88      C  ;               (CH,CL) = ROW,COLUMN OF UPPER LEFT CORNER OF SCROLL     :
                             89      C  ;               (DH,DL) = ROW,COLUMN OF LOWER RIGHT CORNER OF SCROLL    :
                             90      C  ;               (BH) = ATTRIBUTE TO BE USED ON BLANK LINE               :
                             91      C  ;                                                                       :
                             92      C  ;       CHARACTER HANDLING ROUTINES                                     :
                             93      C  ;                                                                       :
                             94      C  ;       (AH) = 8 READ ATTRIBUTE/CHARACTER AT CURRENT CURSOR POSITION    :
                             95      C  ;               (BH) = DISPLAY PAGE                                     :
                             96      C  ;               ON EXIT:                                                :
                             97      C  ;               (AL) = CHAR READ                                        :
                             98      C  ;               (AH) = ATTRIBUTE OF CHARACTER READ (ALPHA MODES ONLY)   :
                             99      C  ;       (AH) = 9 WRITE ATTRIBUTE/CHARACTER AT CURRENT CURSOR POSITION   :
                             100     C  ;               (BH) = DISPLAY PAGE                                     :
                             101     C  ;               (CX) = COUNT OF CHARACTERS TO WRITE                     :
                             102     C  ;               (AL) = CHAR TO WRITE                                    :
                             103     C  ;               (BL) = ATTRIBUTE OF CHARACTER (ALPHA)/COLOR OF CHAR     :
                             104     C  ;                      (GRAPHICS)                                       :
                             105     C  ;                       SEE NOTE ON WRITE DOT FOR BIT 7 OF BL = 1.      :
                             106     C  ;       (AH) = A  WRITE CHARACTER ONLY AT CURRENT CURSOR POSITION       :
                             107     C  ;               (BH) = DISPLAY PAGE                                     :
                             108     C  ;               (CX) = COUNT OF CHARACTERS TO WRITE                     :
                             109     C  ;               (AL) = CHAR TO WRITE                                    :
                             110     C  ;       FOR READ/WRITE CHARACTER INTERFACE WHILE IN GRAPHICS MODE, THE  :
                             111     C  ;               CHARACTERS ARE FORMED FROM A CHARACTER GENERATOR IMAGE  :
                             112     C  ;               MAINTAINED IN THE SYSTEM ROM.  ONLY THE 1ST 128 CHARS   :
                             113     C  ;               ARE CONTAINED THERE.  TO READ/WRITE THE SECOND 128      :
                             114     C  ;               CHARS, THE USER MUST INITIALIZE THE POINTER AT          :
                             115     C  ;               INTERRUPT 1FH (LOCATION 0007CH) TO POINT TO THE 1K BYTE :
                             116     C  ;               TABLE CONTAINING THE CODE POINTS FOR THE SECOND         :
                             117     C  ;               128 CHARS (128-255).                                    :
                             118     C  ;                                                                       :
                             119     C  ;               FOR THE NEW GRAPHICS MODES 256 GRAPHICS CHARS ARE       :
                             120     C  ;               SUPPLIED IN THE SYSTEM ROM.                             :
                             121     C  ;                                                                       :
                             122     C  ;       FOR WRITE CHARACTER INTERFACE IN GRAPHICS MODE, THE REPLICATION :
                             123     C  ;               FACTOR CONTAINED IN (CX) ON ENTRY WILL PRODUCE VALID    :
                             124     C  ;               RESULTS ONLY FOR CHARACTERS CONTAINED ON THE SAME ROW.  :
                             125     C  ;               CONTINUATION TO SUCCEEDING LINES WILL NOT PRODUCE       :
                             126     C  ;               CORRECTLY.                                              :
                             127     C  ;                                                                       :
                             128     C  ;       GRAPHICS INTERFACE                                              :
                             129     C  ;       (AH) = 8  SET COLOR PALETTE                                     :
                             130     C  ;               FOR USE IN COMPATIBILITY MODES                          :
                             131     C  ;               (BH) = PALETTE COLOR ID BEING SET (0-127)               :
                             132     C  ;               (BL) = COLOR VALUE TO BE USED WITH THAT COLOR ID        :
                             133     C  ;                  NOTE: FOR THE CURRENT COLOR CARD, THIS ENTRY POINT   :
                             134     C  ;                        HAS MEANING ONLY FOR 320X200 GRAPHICS.         :
                             135     C  ;                       COLOR ID = 0 SELECTS THE BACKGROUND COLOR (0-15):
                             136     C  ;                       COLOR ID = 1 SELECTS THE PALETTE TO BE USED:    :
                             137     C  ;                               0 = GREEN(1)/RED(2)/BROWN(3)            :
                             138     C  ;                               1 = CYAN(1)/MAGENTA(2)/WHITE(3)         :
                             139     C  ;                       IN 40X25 OR 80X25 ALPHA MODS, THE VALUE SET     :
                             140     C  ;                               FOR PALETTE COLOR 0 INDICATES THE       :
                             141     C  ;                               BORDER COLOR TO BE USED (VALUES 0-31,   :
                             142     C  ;                               WHERE 16-31 SELECT THE HIGH INTENSITY   :
                             143     C  ;                               BACKGROUND SET).                        :
                             144     C  ;       (AH) = C  WRITE DOT                                             :
                             145     C  ;               (BH) = PAGE                                             :
                             146     C  ;               (DX) = ROW NUMBER                                       :
                             147     C  ;               (CX) = COLUMN NUMBER                                    :
                             148     C  ;               (AL) = COLOR VALUE                                      :
                             149     C  ;                       IF BIT 7 OF AL = 1, THEN THE COLOR VALUE IS     :
                             150     C  ;                       EXCLUSIVE OR'D WITH THE CURRENT CONTENTS OF     :
                             151     C  ;                       THE DOT                                         :
                             152     C  ;       (AH) = D  READ DOT                                              :
                             153     C  ;               (BH) = PAGE                                             :
                             154     C  ;               (DX) = ROW NUMBER                                       :
                             155     C  ;               (CX) = COLUMN NUMBER                                    :
                             156     C  ;               (AL) RETURNS THE DOT READ                               :
                             157     C  ;                                                                       :
                             158     C  ; ASCII TELETYPE ROUTINE FOR OUTPUT                                     :
                             159     C  ;                                                                       :
                             160     C  ;       (AH) = E  WRITE TELETYPE TO ACTIVE PAGE                         :
                             161     C  ;               (AL) = CHAR TO WRITE                                    :
                             162     C  ;               (BL) = FOREGROUND COLOR IN GRAPHICS MODE                :
                             163     C  ;               NOTE -- SCREEN WIDTH IS CONTROLLED BY PREVIOUS MODE SET :
                             164     C  ;                                                                       :
                             165     C  ;       (AH) = F  CURRENT VIDEO STATE                                   :
                             166     C  ;               RETURNS THE CURRENT VIDEO STATE                         :
                             167     C  ;               (AL) = MODE CURRENTLY SET  (SEE AH=0 FOR EXPLANATION)   :
                             168     C  ;               (AH) = NUMBER OF CHARACTER COLUMNS ON SCREEN            :
                             169     C  ;               (BH) = CURRENT ACTIVE DISPLAY PAGE                      :
                             170     C  ;                                                                       :
                             171     C  ;       (AH) = 10  SET PALETTE REGISTERS                                :
                             172     C  ;                                                                       :
                             173     C  ;               (AL) = 0        SET INDIVIDUAL PALETTE REGISTER         :
                             174     C  ;                BL = PALETTE REGISTER TO BE SET                        :
                             175     C  ;                BH = VALUE TO SET                                      :
                             176     C  ;                                                                       :
                             177     C  ;               AL = 1        SET OVERSCAN REGISTER                     :
                             178     C  ;               BH = VALUE TO SET                                       :
                             179     C  ;                                                                       :
                             180     C  ;               AL = 2        SET ALL PALETTE REGISTERS AND OVERSCAN    :
                             181     C  ;               ES:DX POINTS TO A 17 BYTE TABLE                         :
                             182     C  ;               BYTES 0 - 15 ARE THE PALETTE VALUES, RESPECTIVELY       :
                             183     C  ;               BYTE 16 IS THE OVERSCAN VALUE                           :
                             184     C  ;                                                                       :
                             185     C  ;               AL = 3          TOGGLE INTENSIFY/BLINKING BIT           :
                             186     C  ;                       BL - 0  ENABLE INTENSIFY                        :
                             187     C  ;                       BL - 1  ENABLE BLINKING                         :
                             188     C  ;                                                                       :
                             189     C  ;       (AH) = 11  CHARACTER GENERATOR ROUTINE                          :
                             190     C  ;               NOTE : THIS CALL WILL INITIATE A MODE SET, COMPLETELY   :
                             191     C  ;                       RESETTING THE VIDEO ENVIRONMENT BUT MAINTAINING :
                             192     C  ;                       THE REGEN BUFFER.                               :
                             193     C  ;                                                                       :
                             194     C  ;               AL = 00   USER ALPHA LOAD                               :
                             195     C  ;                       ES:BP - POINTER TO USER TABLE                   :
                             196     C  ;                       CX    - COUNT TO STORE                          :
                             197     C  ;                       DX    - CHARACTER OFFSET INTO TABLE             :
                             198     C  ;                       BL    - BLOCK TO LOAD                           :
                             199     C  ;                       BH    - NUMBER OF BYTES PER CHARACTER           :
                             200     C  ;               AL = 01   ROM MONOCHROME SET                            :
                             201     C  ;                       BL    - BLOCK TO LOAD                           :
                             202     C  ;               AL = 02   ROM 8X8 DOUBLE DOT                            :
                             203     C  ;                       BL    - BLOCK TO LOAD                           :
                             204     C  ;               AL = 03   SET BLOCK SPECIFIER                           :
                             205     C  ;                       BL    - CHAR GEN BLOCK SPECIFIER                :
                             206     C  ;                               D3-D2  ATTR BIT 3 ONE,  CHAR GEN 0-3    :
                             207     C  ;                               D1-D0  ATTR BIT 3 ZERO, CHAR GEN 0-3    :
                             208     C  ;                       NOTE : WHEN USING AL = 03 A FUNCTION CALL       :
                             209     C  ;                               AX = 1000H                              :
                             210     C  ;                               BX = 0712H                              :
                             211     C  ;                               IS RECOMMENDED TO SET THE COLOR PLANES  :
                             212     C  ;                               RESULTING IN 512 CHARACTERS AND EIGHT   :
                             213     C  ;                               CONSISTENT COLORS.                      :
                             214     C  ;                                                                       :
                             215     C  ;       NOTE : THE FOLLOWING INTERFACE (AL=1X) IS SIMILAR IN FUNCTION   :
                             216     C  ;               TO (AL=0X) EXCEPT THAT :                                :
                             217     C  ;                       - PAGE ZERO MUST BE ACTIVE                      :
                             218     C  ;                       - POINTS (BYTES/CHAR) WILL BE RECALCULATED      :
                             219     C  ;                       - ROWS WILL BE CALCULATED FROM THE FOLLOWING:   :
                             220     C  ;                               INT[(200 OR 350) / POINTS] - 1          :
                             221     C  ;                       - CRT_LEN WILL BE CALCULATED FROM :             :
                             222     C  ;                               (ROWS + 1) * CRT_COLS * 2               :
                             223     C  ;                       - THE CRTC WILL BE REPROGRAMMED AS FOLLOWS :    :
                             224     C  ;                               R09H = POINTS - 1       MAX SCAN LINE   :
                             225     C  ;                                       R09H DONE ONLY IN MODE 7        :
                             226     C  ;                               ROAH = POINTS - 2       CURSOR START    :
                             227     C  ;                               R0BH = 0                CURSOR END      :
                             228     C  ;                               R12H =                  VERT DISP END   :
                             229     C  ;                                      [(ROWS + 1) * POINTS] - 1        :
                             230     C  ;                               R14H = POINTS           UNDERLINE LOC   :
                             231     C  ;                                                                       :
                             232     C  ;               THE ABOVE REGISTER CALCULATIONS MUST BE CLOSE TO THE    :
                             233     C  ;               ORIGINAL TABLE VALUES OR UNDETERMINED RESULTS WILL      :
                             234     C  ;               OCCUR.                                                  :
                             235     C  ;                                                                       :
                             236     C  ;               NOTE : THE FOLLOWING INTERFACE IS DESIGNED TO BE        :
                             237     C  ;                       CALLED ONLY IMMEDIATELY AFTER A MODE SET HAS    :
                             238     C  ;                       BEEN ISSUED. FAILURE TO ADHERE TO THIS PRACTICE :
                             239     C  ;                       MAY CAUSE UNDETERMINED RESULTS.                 :
                             240     C  ;                                                                       :
                             241     C  ;               AL = 10   USER ALPHA LOAD                               :
                             242     C  ;                       ES:BP - POINTER TO USER TABLE                   :
                             243     C  ;                       CX    - COUNT TO STORE                          :
                             244     C  ;                       DX    - CHARACTER OFFSET INTO TABLE             :
                             245     C  ;                       BL    - BLOCK TO LOAD                           :
                             246     C  ;                       BH    - NUMBER OF BYTES PER CHARACTER           :
                             247     C  ;               AL = 11   ROM MONOCHROME SET                            :
                             248     C  ;                       BL    - BLOCK TO LOAD                           :
                             249     C  ;               AL = 12   ROM 8X8 DOUBLE DOT                            :
                             250     C  ;                       BL    - BLOCK TO LOAD                           :
                             251     C  ;                                                                       :
                             252     C  ;                                                                       :
                             253     C  ;               NOTE : THE FOLLOWING INTERFACE IS DESIGNED TO BE        :
                             254     C  ;                       CALLED ONLY IMMEDIATELY AFTER A MODE SET HAS    :
                             255     C  ;                       BEEN ISSUED. FAILURE TO ADHERE TO THIS PRACTICE :
                             256     C  ;                       MAY CAUSE UNDETERMINED RESULTS.                 :
                             257     C  ;                                                                       :
                             258     C  ;               AL = 20   USER GRAPHICS CHARS  INT 01FH (8X8)           :
                             259     C  ;                       ES:BP - POINTER TO USER TABLE                   :
                             260     C  ;               AL = 21   USER GRAPHICS CHARS                           :
                             261     C  ;                       ES:BP - POINTER TO USER TABLE                   :
                             262     C  ;                       CX    - POINTS (BYTES PER CHARACTER)            :
                             263     C  ;                       BL    - ROW SPECIFIER                           :
                             264     C  ;                                                                       :
                             265     C  ;                               BL = 0  USER                            :
                             266     C  ;                                       DL - ROWS                       :
                             267     C  ;                               BL = 1  14 (0EH)                        :
                             268     C  ;                               BL = 2  25 (19H)                        :
                             269     C  ;                               BL = 3  43 (2BH)                        :
                             270     C  ;                                                                       :
                             271     C  ;               AL = 22   ROM 8 X 14 SET                                :
                             272     C  ;                       BL    - ROW SPECIFIER                           :
                             273     C  ;               AL = 23   ROM 8 X 8 DOUBLE DOT                          :
                             274     C  ;                       BL    - ROW SPECIFIER                           :
                             275     C  ;                                                                       :
                             276     C  ;                                                                       :
                             277     C  ;               AL = 30   INFORMATION                                   :
                             278     C  ;                               CX    - POINTS                          :
                             279     C  ;                               DL    - ROWS                            :
                             280     C  ;                       BH    - 0       RETURN CURRENT INT 1FH PTR      :
                             281     C  ;                               ES:BP - PTR TO TABLE                    :
                             282     C  ;                       BH    - 1       RETURN CURRENT INT 44H PTR      :
                             283     C  ;                               ES:BP - PTR TO TABLE                    :
                             284     C  ;                       BH    - 2       RETURN ROM 8 X 14 PTR           :
                             285     C  ;                               ES:BP - PTR TO TABLE                    :
                             286     C  ;                       BH    - 3       RETURN ROM DOUBLE DOT PTR       :
                             287     C  ;                               ES:BP - PTR TO TABLE                    :
                             288     C  ;                       BH    - 4       RETURN ROM DOUBLE DOT PTR (TOP) :
                             289     C  ;                               ES:BP - PTR TO TABLE                    :
                             290     C  ;                       BH    - 5       RETURN ROM ALPHA ALTERNATE 9X14 :
                             291     C  ;                               ES:BP - PTR TO TABLE                    :
                             292     C  ;                                                                       :
                             293     C  ;       (AH) = 12   ALTERNATE SELECT                                    :
                             294     C  ;                                                                       :
                             295     C  ;               BL = 10  RETURN EGA INFORMATION                         :
                             296     C  ;                       BH = 0 - COLOR MODE IN EFFECT <3><0><X>         :
                             297     C  ;                            1 - MONOC MODE IN EFFECT <3><B><,X>        :
                             298     C  ;                       BL = MEMORY VALUE                               :
                             299     C  ;                               0 0 - 064K      0 1 - 128K              :
                             301     C  ;                               1 0 - 192K      1 1 - 256K              :
                             301     C  ;                       CH = FEATURE BITS                               :
                             302     C  ;                       CL = SWITCH SETTING                             :
                             303     C  ;                                                                       :
                             304     C  ;               BL = 20  SELECT ALTERNATE PRINT SCREEN ROUTINE          :
                             305     C  ;                                                                       :
                             306     C  ;       (AH) = 13   WRITE STRING                                        :
                             307     C  ;                       ES:BP - POINTER TO STRING TO BE WRITTEN         :
                             308     C  ;                       CX    - CHARACTER ONLY COUNT                    :
                             309     C  ;                       DX    - POSITION TO BEGIN STRING, IN CURSOR     :
                             310     C  ;                               TERMS                                   :
                             311     C  ;                       BH    - PAGE NUMBER                             :
                             312     C  ;                                                                       :
                             313     C  ;               AL = 0                                                  :
                             314     C  ;                       BL    - ATTRIBUTE                               :
                             315     C  ;                       STRING - (CHAR, CHAR, CHAR, ...)                :
                             316     C  ;                       CURSOR NOT MOVED                                :
                             317     C  ;               AL = 1                                                  :
                             318     C  ;                       BL    - ATTRIBUTE                               :
                             319     C  ;                       STRING - (CHAR, CHAR, CHAR, ...)                :
                             320     C  ;                       CURSOR  IS MOVED                                :
                             321     C  ;               AL = 2                                                  :
                             322     C  ;                       STRING - (CHAR, ATTR, CHAR, ATTR, ...)          :
                             323     C  ;                       CURSOR NOT MOVED                                :
                             324     C  ;               AL = 3                                                  :
                             325     C  ;                       STRING - (CHAR, ATTR, CHAR, ATTR, ...)          :
                             326     C  ;                       CURSOR  IS MOVED                                :
                             327     C  ;                                                                       :
                             328     C  ;               NOTE : CHAR RET, LINE FEED, BACKSPACE, AND BELL ARE     :
                             329     C  ;                       TREATED AS COMMANDS RATHER THAN PRINTABLE       :
                             330     C  ;                       CHARACTERS.                                     :
                             331     C  ;                                                                       :
                             332     C  ;------------------------------------------------------------------------
                             333     C
                             334     C  SRLOAD  MACRO   SEGREG,VALUE
                             335     C          IFNB <VALUE>
                             336     C          IFIDN <VALUE>,<0>
                             337     C          SUB     DX,DX
                             338     C          ELSE
                             339     C          MOV     DX,VALUE
                             340     C          ENDIF
                             341     C          ENDIF
                             342     C          MOV     SEGREG,DX
                             343     C          ENDM
                             344     C
                             345     C
                             346     C  ;----- LOW MEMORY SEGMENT
                             347     C
0000                         348     C  ABS0    SEGMENT AT      0
0014                         349     C          ORG     005H*4                          ; PRINT SCREEN VECTOR
0014                         350     C  INT5_PTR        LABEL   DWORD
0040                         351     C          ORG     010H*4                          ; VIDEO I/O VECTOR
0040                         352     C  VIDEO           LABEL   DWORD
007C                         353     C          ORG     01FH*4                          ; GRAPHIC CHARS 128-255
007C                         354     C  EXT_PTR         LABEL   DWORD
                             355     C
0108                         356     C          ORG     042H*4                          ; REVECTORED 10H*4
0108                         357     C  PLANAR_VIDEO    LABEL   DWORD
                             358     C
010C                         359     C          ORG     043H*4                          ; GRAPHIC CHARS 0-255
010C                         360     C  GRX_SET         LEBEL   DWORD
                             361     C
0410                         362     C          ORG     0410H
0410                         363     C  EQUIP_LOW       LABEL   BYTE
0410  ????                   364     C  EQUIP_FLAG      DW      ?
                             365     C
                             366     C  ;----- REUSE RAM FROM PLANAR BIOS
                             367     C
0449                         368     C          ORG     449H
0449  ??                     369     C  CRT_MODE        DB      ?
044A  ????                   370     C  CRT_COLS        DW      ?
044C  ????                   371     C  CRT_LEN         DW      ?
044E  ????                   372     C  CRT_START       DW      ?
0450    08 [                 373     C  CURSOR_POSW     DW      8 DUP(?)
              ????           374     C
                    ]        375     C
                             376     C
0460  ????                   377     C  CURSOR_MODE     DW      ?
0462  ??                     378     C  ACTIVE_PAGE     DB      ?
0463  ????                   379     C  ADDR_6845       DW      ?
0465  ??                     380     C  CRT_MOD_SET     DB      ?
0466  ??                     381     C  CRT_PALETTE     DB      ?
                             382     C
0472                         383     C          ORG     0472H
0472  ????                   384     C  RESET_FLAG      DW      ?
0484                         385     C          ORG     0484H
0484  ??                     386     C  ROWS    DB      ?                               ; ROWS ON THE SCREEN
0485  ??                     387     C  POINTS  DW      ?                               ; BYTES PER CHARACTER
                             388     C
0487  ??                     389     C  INFO    DB      ?
                             390     C
                             391     C  ; INFO
                             392     C  ;       D7 - HIGH BIT OF MODE SET, CLEAR/NOT CLEAR REGEN
                             393     C  ;       D6 - MEMORY  D6 D5 = 0 0 - 064K    0 1 - 128K
                             394     C  ;       D5 - MEMORY          1 0 - 192K    1 1 - 256K
                             395     C  ;       D4 - RESERVED
                             396     C  ;       D3 - EGA ACTIVE MONITOR (0), EGA NOT ACTIVE (1)
                             397     C  ;       D2 - WAIT FOR DISPLAY ENABLE (1)
                             398     C  ;       D1 - EGA HAS A MONOCHROME ATTACHED (1)
                             399     C  ;       D0 - SET C_TYPE EMULATE ACTIVE (0)
                             400     C
0488  ??                     401     C  INFO_3  DB      ?
                             402     C
                             403     C  ; INFO_3
                             404     C  ;       D7-D4   FEATURE BITS
                             405     C  ;       D3-D0   SWITCHES
                             406     C
04A8                         407     C          ORG     04A8H
04A8                         408     C  SAVE_PTR        LABEL   DWORD
                             409     C
                             410     C  ;----- SAVE_PTR
                             411     C  ;
                             412     C  ;       SAVE_PTR IS A POINTER TO A TABLE AS DESCRIBED AS FOLLOWS :
                             413     C  ;
                             414     C  ;       DWORD_1         VIDEO PARAMETER TABLE POINTER
                             415     C  ;       DWORD_2         DYNAMIC SAVE AREA POINTER
                             416     C  ;       DWORD_3         ALPHA MODE AUXILIARY CHAR GEN POINTER
                             417     C  ;       DWORD_4         GRAPHICS MODE AUXILIARY CHAR GEN POINTER
                             418     C  ;       DWORD_5         RESERVED
                             419     C  ;       DWORD_6         RESERVED
                             420     C  ;       DWORD_7         RESERVED
                             421     C  ;
                             422     C  ;       DWORD_1         PARAMETER TABLE POINTER
                             423     C  ;                       INITIALIZED TO BIOS EGA PARAMETER TABLE.
                             424     C  ;                       THIS VALUE MUST EXIST.
                             425     C  ;
                             426     C  ;       DWORD_2         PARAMETER SAVE AREA POINTER
                             427     C  ;                       INITIALIZED TO 0000:0000, THIS VALUE IS OPTIONAL.
                             428     C  ;                       WHEN NON-ZERO, THIS POINTER WILL BE USED AS POINTER
                             429     C  ;                       TO A RAM AREA WHERE CERTAIN DYNAMIC VALUES ARE TO
                             430     C  ;                       BE SAVED. WHEN IN EGA OPERATION THIS RAM AREA WILL
                             431     C  ;                       HOLD THE 16 EGA PALETTE REGISTER VALUES PLUS
                             432     C  ;                       THE OVERSCAN VALUE IN BYTES 0-16D RESPECTIVELY.
                             433     C  ;                       AT LEAST 256 BYTES MUST BE ALLOCATED FOR THIS AREA.
                             434     C  ;
                             435     C  ;       DWORD_3         ALPHA MODE AUXILIARY POINTER
                             436     C  ;                       INITIALIZED TO 0000:0000, THIS VALUE IS OPTIONAL.
                             437     C  ;                       WHEN NON-ZERO, THIS POINTER IS USED AS A POINTER
                             438     C  ;                       TO A TABLES DESCRIBED AS FOLLOWS :
                             439     C  ;
                             440     C  ;                       BYTE    BYTES/CHARACTER
                             441     C  ;                       BYTE    BLOCK TO LOAD, SHOULD BE ZERO FOR NORMAL
                             442     C  ;                               OPERATION
                             443     C  ;                       WORD    COUNT TO STORE, SHOULD BE 256D FOR NORMAL
                             444     C  ;                               OPERATION
                             445     C  ;                       WORD    CHARACTER OFFSET, SHOULD BE ZERO FOR NORMAL
                             446     C  ;                               OPERATION
                             447     C  ;                       DWORD   POINTER TO A FONT TABLE
                             448     C  ;                       BYTE    DISPLAYABLE ROWS
                             449     C  ;                               IF 'FF' THE MAXIMUM CALCULATED VALUE WILL BE
                             450     C  ;                               USED, ELSE THIS VALUE WILL BE USED
                             451     C  ;                       BYTE    CONSECUTIVE BYTES OF MODE VALUES FOR WHICH
                             452     C  ;                               THIS FONT DESCRIPTION IS TO BE USED.
                             453     C  ;                               THE END OF THIS STREAM IS INDICATED BY A
                             454     C  ;                               BYTE CODE OF 'FF'.
                             455     C  ;
                             456     C  ;                       NOTE : USE OF THIS POINTER MAY CAUSE UNEXPECTED
                             457     C  ;                              CURSOR TYPE OPERATION. FOR AN EXPLANATION
                             458     C  ;                              OF CURSOR TYPE SEE AH = 01 IN THE INTERFACE
                             459     C  ;                              SECTION.
                             460     C  ;
                             461     C  ;       DWORD_4         GRAPHICS MODE AUXILIARY POINTER
                             462     C  ;                       INITIALIZED TO 0000:0000, THIS VALUE IS OPTIONAL.
                             463     C  ;                       WHEN NON-ZERO, THIS POINTER IS USED AS A POINTER
                             464     C  ;                       TO A TABLES DESCRIBED AS FOLLOWS :
                             465     C  ;
                             466     C  ;                       BYTE    DISPLAYABLE ROWS
                             467     C  ;                       WORD    BYTES PER CHARACTER
                             468     C  ;                       DWORD   POINTER TO A FONT TABLE
                             469     C  ;                       BYTE    CONSECUTIVE BYTES OF MODE VALUES FOR WHICH
                             470     C  ;                               THIS FONT DESCRIPTION IS TO BE USED.
                             471     C  ;                               THE END OF THIS STREAM IS INDICATED BY A
                             472     C  ;                               BYTE CODE OF 'FF'.
                             473     C  ;
                             474     C  ;       DWORD_5 THRU DWORD_7
                             475     C  ;                       RESERVED AND SET TO 0000:0000.
                             476     C  ;
                             477     C
0500                         478     C          ORG     0500H
0500  ??                     479     C  STATUS_BYTE     DB      ?
0501                         480     C  ABS0    ENDS
                             481     C
= 0061                       482     C  PORT_B          EQU     61H                     ; 8255 PORT B ADDR
= 0040                       483     C  TIMER           EQU     40H
                             484     C
                             485     C  ;----- EQUATES FOR CARD PORT ADDRESSES
                             486     C
= 00C4                       487     C          SEQ_ADDR        EQU     0C4H
= 00C5                       488     C          SEQ_DATA        EQU     0C5H
= 00D4                       489     C          CRTC_ADDR       EQU     0D4H
= 00B4                       490     C          CRTC_ADDR_B     EQU     0B4H
= 00D5                       491     C          CRTC_DATA       EQU     0D5H            ; OR 0B5H
= 00CC                       492     C          GRAPH_1_POS     EQU     0CCH
= 00CA                       493     C          GRAPH_2_POS     EQU     0CAH
= 00CE                       494     C          GRAPH_ADDR      EQU     0CEH
= 00CF                       495     C          GRAPH_DATA      EQU     0CFH
= 00C2                       496     C          MISC_OUTPUT     EQU     0C2H
= 00C2                       497     C          IN_STAT_O       EQU     0C2H
= 00BA                       498     C          INPUT_STATUS_B  EQU     0BAH
= 00DA                       499     C          INPUT_STATUS    EQU     0DAH
= 00DA                       500     C          ATTR_READ       EQU     0DAH
= 00C0                       501     C          ATTR_WRITE      EQU     0C0H
                             502     C
                             503     C  ;----- EQUATES FOR ADDRESS REGISTER VALUES
                             504     C
= 0000                       505     C          S_RESET         EQU     00H
= 0001                       506     C          S_CLOCK         EQU     01H
= 0002                       507     C          S_MAP           EQU     02H
= 0003                       508     C          S_CGEN          EQU     03H
= 0004                       509     C          S_MEM           EQU     04H
                             510     C
= 0000                       511     C          C_HRZ_TOT       EQU     00H
= 0001                       512     C          C_HRZ_DSP       EQU     01H
= 0002                       513     C          C_STRT_HRZ_BLK  EQU     02H
= 0003                       514     C          C_END_HRZ_BLK   EQU     03H
= 0004                       515     C          C_STRT_HRZ_SYN  EQU     04H
= 0005                       516     C          C_END_HRZ_SYN   EQU     05H
= 0006                       517     C          C_VRT_TOT       EQU     06H
= 0007                       518     C          C_OVERFLOW      EQU     07H
= 0008                       519     C          C_PRE_ROW       EQU     08H
= 0009                       520     C          C_MAX_SCAN_LN   EQU     09H
= 000A                       521     C          C_CRSR_START    EQU     0AH
= 000B                       522     C          C_CRSR_END      EQU     0BH
= 000C                       523     C          C_STRT_HGH      EQU     0CH
= 000D                       524     C          C_STRT_LOW      EQU     0DH
= 000E                       525     C          C_CRSR_LOC_HGH  EQU     0EH
= 000F                       526     C          C_CRSR_LOC_LOW  EQU     0FH
= 0010                       527     C          C_VRT_SYN_STRT  EQU     10H             ; WRITE ONLY
= 0010                       528     C          C_LGHT_SYN_HGH  EQU     10H             ; READ ONLY
= 0011                       529     C          C_VRT_SYN_END   EQU     11H             ; WRITE ONLY
= 0011                       530     C          C_LGHT_PEN_LOW  EQU     11H             ; READ ONLY
= 0012                       531     C          C_VRT_DSP_END   EQU     12H
= 0013                       532     C          C_OFFSET        EQU     13H
= 0014                       533     C          C_UNDERLN_LOC   EQU     14H
= 0015                       534     C          C_STRT_VRT_BLK  EQU     15H
= 0016                       535     C          C_END_VRT_BLK   EQU     16H
= 0017                       536     C          C_MODE_CNTL     EQU     17H
= 0018                       537     C          C_LN_COMP       EQU     18H
                             538     C
= 0000                       539     C          G_SET_RESET     EQU     00H
= 0001                       540     C          G_ENBL_SET      EQU     01H
= 0002                       541     C          G_CLR_COMP      EQU     02H
= 0003                       542     C          G_DATA_ROT      EQU     03H
= 0004                       543     C          G_READ_MAP      EQU     04H
= 0005                       544     C          G_MODE          EQU     05H
= 0006                       545     C          G_MISC          EQU     06H
= 0007                       546     C          G_COLOR         EQU     07H
= 0008                       547     C          G_BIT_MASK      EQU     08H
                             548     C
= 0010                       549     C          P_MODE          EQU     10H
= 0011                       550     C          P_OVERSC        EQU     11H
= 0012                       551     C          P_CPLANE        EQU     12H
= 0013                       552     C          P_HPEL          EQU     13H
                             553     C
                             554     C          SUBTTL
                             555     C
                             556     C  ;----- CODE SEGMENT
                             557
0000                         558        CODE    SEGMENT PUBLIC
                             559
                             560     C          INCLUDE         VPOST.INC
                             561     C          SUBTTL  VPOST.INC
                             562     C          PAGE
                             563     C
                             564     C  ;----- POST
                             565     C
                             566     C          ASSUME  CS:CODE,DS:ABS0
0000                         567     C          ORG     0H
0000  55                     568     C          DB      055H                            ; SIGNATURE
0001  AA                     569     C          DB      0AAH                            ;  BYTES
0002  20                     570     C          DB      020H                            ; LENGTH INDICATOR
                             571     C
                             572     C  ;----- NOTE : DO NOT USE THE SIGNATURE BYTES AS A PRESENCE TEST
                             573     C
                             574     C  ;       PLANAR VIDEO SWITCH SETTINGS
                             575     C
                             576     C  ;       0 0 - UNUSED
                             577     C  ;       0 1 - 40 X 25 COLOR
                             578     C  ;       1 0 - 80 X 25 COLOR
                             579     C  ;       1 1 - 80 X 25 MONOCHROME
                             580     C  ; NOTE : 0 0 MUST BE SET WHEN THIS ADAPTER IS INSTALLED.
                             581     C
                             582     C  ;       VIDEO ADAPTER SWITCH SETTINGS
                             583     C
                             584     C  ;       0 0 0 0 - MONOC PRIMARY, EGA COLOR, 40X25
                             585     C  ;       0 0 0 1 - MONOC PRIMARY, EGA COLOR, 80X25
                             586     C  ;       0 0 1 0 - MONOC PRIMARY, EGA HI RES EMULATE (SAME AS 0001)
                             587     C  ;       0 0 1 1 - MONOC PRIMARY, EGA HI RES ENHANCED
                             588     C  ;       0 1 0 0 - COLOR 40 PRIMARY, EGA MONOCHROME
                             589     C  ;       0 1 0 1 - COLOR 80 PRIMARY, EGA MONOCHROME
                             590     C
                             591     C  ;       0 1 1 0 - MONOC SECONDARY, EGA COLOR, 40X25
                             592     C  ;       0 1 1 1 - MONOC SECONDARY, EGA COLOR, 80X25
                             593     C  ;       1 0 0 0 - MONOC SECONDARY, EGA HI RES EMULATE (SAME AS 0111)
                             594     C  ;       1 0 0 1 - MONOC SECONDARY, EGA HI RES ENHANCED
                             595     C  ;       1 0 1 0 - COLOR 40 SECONDARY, EGA MONOCHROME
                             596     C  ;       1 0 1 1 - COLOR 80 SECONDARY, EGA MONOCHROME
                             597     C
                             598     C  ;       1 1 0 0 - RESERVED
                             599     C  ;       1 1 0 1 - RESERVED
                             600     C  ;       1 1 1 0 - RESERVED
                             601     C  ;       1 1 1 1 - RESERVED
                             602     C
                             603     C  ;----- SETUP ROUTINE FOR THIS MODULE
                             604     C
0003                         605     C  VIDEO_SETUP     PROC    FAR
0003  EB 28                  606     C          JMP     SHORT   L1
0005  32 34 30 30            607     C          DB      '2400'
0009  36 32 37 37 33 35      608     C          DB      '6277356 (C)COPYRIGHT IBM 1984'
      36 20 28 43 29 43      609     C
      4F 50 59 52 49 47      610     C
      48 54 20 49 42 4D      611     C
      20 31 39 38 34         612     C
0026  39 2F 31 33 2F 38      613     C          DB      '9/13/84'
      34                     614     C
                             615     C
                             616     C  ;----- SET UP VIDEO VECTORS
                             617     C
002D                         618     C  L1:
002D  B6 03                  619     C          MOV     DH,3
002F  B2 DA                  620     C          MOV     DL,INPUT_STATUS
0031  EC                     621     C          IN      AL,DX
0032  B2 BA                  622     C          MOV     DL,INPUT_STATUS_B
0034  EC                     623     C          IN      AL,DX
0035  B2 C0                  624     C          MOV     DL,ATTR_WRITE
0037  B0 00                  625     C          MOV     AL,0
0039  EE                     626     C          OUT     DX,AL
                             627     C
                             628     C          SRLOAD  DS,0
003A  2B D2                  629     C+         SUB     DX,DX
003C  8E DA                  630     C+         MOV     DS,DX
003E  FA                     631     C          CLI
003F  C7 06 0040 R 0CD7 R    632     C          MOV     WORD PTR VIDEO,OFFSET COMBO_VIDEO
0045  8C 0E 0042 R           633     C          MOV     WORD PTR VIDEO+2, CS
0049  C7 06 0108 R F065      634     C          MOV     WORD PTR PLANAR_VIDEO,0F065H
004F  C7 06 010A R F000      635     C          MOV     WORD PTR PLANAR_VIDEO+2,0F000H
0055  C7 06 04A8 R 010C R    636     C          MOV     WORD PTR SAVE_PTR,OFFSET SAVE_TBL
005B  8C 0E 04AA R           637     C          MOV     WORD PTR SAVE_PTR+2, CS
005F  C7 06 007C R 0000 E    638     C          MOV     WORD PTR EXT_PTR, OFFSET INT_1F_1
0065  8C 0E 007E R           639     C          MOV     WORD PTR EXT_PTR+2, CS
0069  C7 06 010C R 0000 E    640     C          MOV     WORD PTR GRX_SET, OFFSET CGDDOT
006F  8C 0E 010E R           641     C          MOV     WORD PTR GRX_SET+2, CS
0073  FB                     642     C          STI
                             643     C
                             644     C  ;----- POST FOR COMBO VIDEO CARD
                             645     C
0074  C6 06 0487 R 04        646     C          MOV     INFO,00000100B
0079  E8 009B R              647     C          CALL    RD_SWS
007C  88 1E 0488 R           648     C          MOV     INFO_3,BL
0080  E8 00CE R              649     C          CALL    F_BTS
0083  08 06 0488 R           650     C          OR      INFO_3,AL
0087  8A 1E 0488 R           651     C          MOV     BL,INFO_3
008B  E8 00F3 R              652     C          CALL    MK_ENV
008E  E9 0244 R              653     C          JMP     POST
0091                         654     C  SKIP:
0091  CB                     655     C          RET
0092                         656     C  VIDEO_SETUP     ENDP
                             657     C
                             658     C
0092                         659     C  POR_1   PROC    NEAR
0092  EE                     660     C          OUT     DX,AL
0093  50                     661     C          PUSH    AX
0094  58                     662     C          POP     AX
0095  EC                     663     C          IN      AL,DX
0096  24 10                  664     C          AND     AL,010H
0098  D0 E8                  665     C          SHR     AL,1
009A  C3                     666     C          RET
009B                         667     C  POR_1   ENDP
                             668     C
                             669     C  ;----- READ THE SWITCH SETTINGS ON THE CARD
                             670     C
009B                         671     C  RD_SWS  PROC    NEAR
                             672     C          ASSUME  DS:ABS0
009B  B6 03                  673     C          MOV     DH,3
009D  B2 C2                  674     C          MOV     DL,MISC_OUTPUT
009F  B0 01                  675     C          MOV     AL,1
00A1  EE                     676     C          OUT     DX,AL
                             677     C
                             678     C  ;----- COULD BE 0,4,8,C
                             679     C
00A2  B0 0D                  680     C          MOV     AL,0DH
00A4  E8 0092 R              681     C          CALL    POR_1
00A7  D0 E8                  682     C          SHR     AL,1
00A9  D0 E8                  683     C          SHR     AL,1
00AB  D0 E8                  684     C          SHR     AL,1
00AD  8A D8                  685     C          MOV     BL,AL
                             686     C
00AF  B0 09                  687     C          MOV     AL,9
00B1  E8 0092 R              688     C          CALL    POR_1
00B4  D0 E8                  689     C          SHR     AL,1
00B6  D0 E8                  690     C          SHR     AL,1
00B8  0A D8                  691     C          OR      BL,AL
                             692     C
00BA  B0 05                  693     C          MOV     AL,5
00BC  E8 0092 R              694     C          CALL    POR_1
00BF  D0 E8                  695     C          SHR     AL,1
00C1  0A D8                  696     C          OR      BL,AL
                             697     C
00C3  B0 01                  698     C          MOV     AL,1
00C5  E8 0092 R              699     C          CALL    POR_1
00C8  0A D8                  700     C          OR      BL,AL
                             701     C
00CA  80 E3 0F               702     C          AND     BL,0FH
00CD  C3                     703     C          RET
00CE                         704     C  RD_SWS  ENDP
                             705     C
                             706     C  ;----- OBTAIN THE FEATURE BITS FROM DAUGHTER CARD
                             707     C
00CE                         708     C  F_BTS   PROC    NEAR
00CE  B6 03                  709     C          MOV     DH,3
00D0  B2 BA                  710     C          MOV     DL,0BAH
00D2  B0 01                  711     C          MOV     AL,1
00D4  EE                     712     C          OUT     DX,AL
00D5  B2 DA                  713     C          MOV     DL,0DAH
00D7  EE                     714     C          OUT     DX,AL
00D8  B2 C2                  715     C          MOV     DL,IN_STAT_0
00DA  EC                     716     C          IN      AL,DX                   ; READ FEATURE BITS
00DB  24 60                  717     C          AND     AL,060H
00DD  D0 E8                  718     C          SHR     AL,1
00DF  8A D8                  719     C          MOV     BL,AL
00E1  B2 BA                  720     C          MOV     DL,0BAH
00E3  B0 02                  721     C          MOV     AL,2
00E5  EE                     722     C          OUT     DX,AL
00E6  B2 DA                  723     C          MOV     DL,0DAH
00E8  EE                     724     C          OUT     DX,AL
00E9  B2 C2                  725     C          MOV     DL,IN_STAT_0
00EB  EC                     726     C          IN      AL,DX                   ; READ FEATURE BITS
00EC  24 60                  727     C          AND     AL,060H
00EE  D0 E0                  728     C          SHL     AL,1
00F0  0A C3                  729     C          OR      AL,BL
00F2  C3                     730     C          RET
00F3                         731     C  F_BTS   ENDP
                             732     C
                             733     C  ;----- ESTABLISH THE VIDEO ENVIRONMENT, KEYED OFF OF THE SWITCHES
                             734     C
00F3                         735     C  MK_ENV  PROC    NEAR
                             736     C          ASSUME  DS:ABS0
00F3  2A FF                  737     C          SUB     BH,BH
00F5  80 E3 0F               738     C          AND     BL,0FH
00F8  D1 E3                  739     C          SAL     BX,1
00FA  52                     740     C          PUSH    DX
00FB  86 03                  741     C          MOV     DH,3
00FD  8A E6                  742     C          MOV     AH,DH
00FF  5A                     743     C          POP     DX
0100  80 E4 01               744     C          AND     AH,1
0103  FE C4                  745     C          INC     AH
0105  F6 D4                  746     C          NOT     AH
0107  2E: FF A7 0128 R       747     C          JMP     WORD PTR CS:[BX + OFFSET T5]
                             748     C
010C                         749     C  SAVE_TBL        LABEL   DWORD
010C  0717 R                 750     C          DW      OFFSET  VIDEO_PARMS     ; PARMS
010E  C000                   751     C          DW      0C000H                  ; PARMS
0110  0000                   752     C          DW      0                       ; PAL SAVE AREA
0112  0000                   753     C          DW      0                       ; PAL SAVE AREA
0114  0000                   754     C          DW      0                       ; ALPHA TABLES
0016  0000                   755     C          DW      0                       ; ALPHA TABLES
0118  0000                   756     C          DW      0                       ; ALPHA TABLES
011A  0000                   757     C          DW      0                       ; GRAPHICS TABLES
                             758     C
011C  0000                   759     C          DW      0
011E  0000                   760     C          DW      0
0120  0000                   761     C          DW      0
0122  0000                   762     C          DW      0
0124  0000                   763     C          DW      0
0126  0000                   764     C          DW      0
                             765     C
0128                         766     C  T5      LABEL   WORD
0128  0173 R                 767     C          DW      OFFSET PST_0
012A  017E R                 768     C          DW      OFFSET PST_1
012C  017E R                 769     C          DW      OFFSET PST_2
012E  0189 R                 770     C          DW      OFFSET PST_3
0130  0194 R                 771     C          DW      OFFSET PST_4
0132  01A8 R                 772     C          DW      OFFSET PST_5
0134  01BC R                 773     C          DW      OFFSET PST_6
0136  01C7 R                 774     C          DW      OFFSET PST_7
                             775     C
0138  01C7 R                 776     C          DW      OFFSET PST_8
013A  01D2 R                 777     C          DW      OFFSET PST_9
013C  01DD R                 778     C          DW      OFFSET PST_A
013E  01F1 R                 779     C          DW      OFFSET PST_B
0140  0204 R                 780     C          DW      OFFSET PST_OUT
0142  0204 R                 781     C          DW      OFFSET PST_OUT
0144  0204 R                 782     C          DW      OFFSET PST_OUT
0146  0204 R                 783     C          DW      OFFSET PST_OUT
                             784     C
0148                         785     C  ENV_X   PROC    NEAR                    ; SET 40X25 COLOR ALPHA
0148  80 26 0410 R CF        786     C          AND     EQUIP_LOW,0CFH
0140  80 0E 0410 R 10        787     C          OR      EQUIP_LOW,010H
0152  B8 0001                788     C          MOV     AX,1H
0155  CD 10                  789     C          INT     10H
0157  C3                     790     C          RET
0158                         791     C  ENV_X   ENDP
                             792     C
0158                         793     C  ENV_0   PROC    NEAR                    ; SET 80X25 COLOR ALPHA
0158  80 26 0410 R CF        794     C          AND     EQUIP_LOW,0CFH
015D  80 0E 0410 R 20        795     C          OR      EQUIP_LOW,020H
0162  B8 0003                796     C          MOV     AX,03H
0165  CD 10                  797     C          INT     10H
0167  C3                     798     C          RET
0168                         799     C  ENV_0   ENDP
                             800     C
0168                         801     C  ENV_3   PROC    NEAR                    ; SET MONOCHROME ALPHA
0168  80 0E 0410 R 30        802     C          OR      EQUIP_LOW,030H
016D  B8 0007                803     C          MOV     AX,07H
0170  CD 10                  804     C          INT     10H
0172  C3                     805     C          RET
0173                         806     C  ENV_3   ENDP
                             807     C
                             808     C
0173                         809     C  PST_0:
0173  20 26 0487 R           810     C          AND     INFO,AH
0177  E8 0148 R              811     C          CALL    ENV_X
017A  E8 0168 R              812     C          CALL    ENV_3
017D  C3                     813     C          RET
017E                         814     C  PST_1:
017E                         815     C  PST_2:
017E  20 26 0487 R           816     C          AND     INFO,AH
0182  E8 0158 R              817     C          CALL    ENV_0
0185  E8 0168 R              818     C          CALL    ENV_3
0188 C3                      819     C          RET
0189                         820     C  PST_3:
0189  20 26 0487 R           821     C          AND     INFO,AH
0189  E8 0158 R              822     C          CALL    ENV_0
0190  E8 0168 R              823     C          CALL    ENV_3
0193  C3                     824     C          RET
0194                         825     C  PST_4:
0194  B6 03                  826     C          MOV     DH,3
0196  B2 C2                  827     C          MOV     DL,MISC_OUTPUT
0198  B0 00                  828     C          MOV     AL,0
019A  EE                     829     C          OUT     DX,AL
019B  F6 D4                  830     C          NOT     AH
019D  08 26 0487 R           831     C          OR      INFO,AH
01A1  E8 0168 R              832     C          CALL    ENV_3
01A4  E8 0148 R              833     C          CALL    ENV_X
01A7  C3                     834     C          RET
01A8                         835     C  PST_5:
01A8  B6 03                  836     C          MOV     DH,3
01AA  B2 C2                  837     C          MOV     DL,MISC_OUTPUT
01AC  B0 00                  838     C          MOV     AL,0
01AE  EE                     839     C          OUT     DX,AL
01AF  F6 D4                  840     C          NOT     AH
01B1  08 26 0487 R           841     C          OR      INFO,AH
01B5  E8 0168 R              842     C          CALL    ENV_3
01B8  E8 0158 R              843     C          CALL    ENV_0
01BB  C3                     844     C          RET
01BC                         845     C  PST_6:
01BC  20 26 0487 R           846     C          AND     INFO,AH
01C0  E8 0168 R              847     C          CALL    ENV_3
01C3  E8 0148 R              848     C          CALL    ENV_X
01C6  C3                     849     C          RET
01C7                         850     C  PST_7:
01C7                         851     C  PST_8:
01C7  20 26 0487 R           852     C          AND     INFO,AH
01C8  E8 0168 R              853     C          CALL    ENV_3
01CE  E8 0158 R              854     C          CALL    ENV_0
01D1  C3                     855     C          RET
01D2                         856     C  PST_9:
01D2  20 26 0487 R           857     C          AND     INFO,AH
01D6  E8 0168 R              858     C          CALL    ENV_3
01D9  E8 0158 R              859     C          CALL    ENV_0
01DC  C3                     860     C          RET
01DD                         861     C  PST_A:
01DD  B6 03                  862     C          MOV     DH,3
01DF  B2 C2                  863     C          MOV     DL,MISC_OUTPUT
01E1  B0 00                  864     C          MOV     AL,0
01E3  EE                     865     C          OUT     DX,AL
01E4  F6 D4                  866     C          NOT     AH
01E6  08 2 0487 R            867     C          OR      INFO,AH
01EA  E8 0148 R              868     C          CALL    ENV_X
01ED  E8 0168 R              869     C          CALL    ENV_3
01F0  C3                     870     C          RET
01F1                         871     C  PST_B:
01F1  B6 03                  872     C          MOV     DH,3
01F3  B2 C2                  873     C          MOV     DL,MISC_OUTPUT
01F5  B0 D0                  874     C          MOV     AL,0
01F7  EE                     875     C          OUT     DX,AL
01F8  F6 D4                  876     C          NOT     AH
01FA  08 26 0487 R           877     C          OR      INFO,AH
01FE  E8 0158 R              878     C          CALL    ENV_0
0201  E8 0168 R              879     C          CALL    ENV_3
0204                         880     C  PST_OUT:
0204  C3                     881     C          RET
0205                         882     C  MK_ENV  ENDP
                             883     C
                             884     C  ;------------------------------------------------------------------------
                             885     C  ; THIS ROUTINE TESTS THE CRT CARD INTERNAL DATA BUS AND IN A LIMITED    :
                             886     C  ; WAY TESTS THE CRTC VIDEO CHIP BY WRITING/READING FROM CURSOR REGISTER :
                             887     C  ; CARRY IS SET IF AN ERROR IS FOUND                                     :
                             888     C  ;                                                                       :
                             889     C  ; REGISTERS BX,SI,ES,DS ARE PRESERVED.                                  :
                             890     C  ; REGISTERS AX,CX,DX ARE MODIFYED.                                      :
                             891     C  ;------------------------------------------------------------------------
0205                         892     C  CD_PRESENCE_TST PROC    NEAR
0205  53                     893     C          PUSH    BX                      ; SAVE BX
0206  BB 007F                894     C          MOV     BX,07FH                 ; INITIAL WORD PATTERN BYTE
0209  8B FB                  895     C          MOV     DI,BX
020B  50                     896     C          PUSH    AX                      ; SAVE PORT ADDRESS
020C  E8 022C R              897     C          CALL    RD_CURSOR               ;
020F  8B F0                  898     C          MOV     SI,AX                   ; SAVE ORIGINAL VALUE
0211  58                     899     C          POP     AX                      ; RECOVER PORT ADDRESS
0212  50                     900     C          PUSH    AX                      ; SAVE PORT ADDRESS
0213  E8 0236 R              901     C          CALL    WR_CURSOR               ; WRITE CURSOR
0216  58                     902     C          POP     AX                      ; RECOVER PORT ADDRESS
0217  50                     903     C          PUSH    AX                      ; SAVE PORT ADDRESS
0218  58 022C R              904     C          CALL    RD_CURSOR               ; READ IT BACK
021B  3B C7                  905     C          CMP     AX,DI                   ; SAME?
021D  58                     906     C          POP     AX
021E  75 03                  907     C          JNZ     NOT_PRESENT             ; EXIT IF NOT EQUAL
0220  EB 05 90               908     C          JMP     TST_EX
0223                         909     C  NOT_PRESENT:
0223  33 C0                  910     C          XOR     AX,AX                   ; SET NOT PRESENT
0225  5B                     911     C          POP     BX
0226  C3                     912     C          RET
0227                         913     C  TST_EX:
0227  B8 0001                914     C          MOV     AX,1                    ; SET PRESENT ON EXIT
022A  5B                     915     C          POP     BX                      ; RESTORE BX
022B  C3                     916     C          RET
022C                         917     C  CD_PRESENCE_TST ENDP
                             918
                             919     C  ;------------------------------------------------------------------------
                             920     C  ; MODULE NAME  RD_CURSOR                                                :
                             921     C  ;  READ CURSOR POSITION [ADDRESS] (FROM CRTC) TO  AX                    :
                             922     C  ;                                                                       :
                             923     C  ; REGISTER AX IS MODIFIED.                                              :
                             924     C  ;------------------------------------------------------------------------
022C                         925     C  RD_CURSOR       PROC NEAR
022C  52                     926     C          PUSH    DX                      ; SAVE REGS USED
022D  88 D0                  927     C          MOV     DX,AX
022F  B0 0E                  928     C          MOV     AL,C_CRSR_LOC_HGH
0231  EE                     929     C          OUT     DX,AL
0232  42                     930     C          INC     DX
0233  EC                     931     C          IN      AL,DX
                             932     C                                          ; RETURN WITH CURSOR POS IN AX
                             933     C                                          ; RESTORE REGS USED
0234  5A                     934     C          POP     DX
0235  C3                     935     C          RET
0236                         936     C  RD_CURSOR       ENDP
                             937     C
                             938     C  ;-------------------------------------------------------------------------
                             939     C  ; MODULE NAME  WR_CURSOR                                                 :
                             940     C  ;  WRITE CURSOR POSITION [ADDRESS] (TO CRTC) WITH CONTENTS OF AX         :
                             941     C  ;                                                                        :
                             942     C  ; ALL REGISTERS PRESERVED                                                :
                             943     C  ;------------------------------------------------------------------------:
0236                         944     C  WR_CURSOR       PROC NEAR
                             945     C                                          ; SAVE REGS USED
0236  50                     946     C          PUSH    AX
0237  52                     947     C          PUSH    DX
0238  8B D0                  948     C          MOV     DX,AX
023A  B4 0E                  949     C          MOV     AH,C_CRSR_LOC_HGH       ; CURSOR LOCATION HIGH INDEX
023C  B0 7F                  950     C          MOV     AL,07FH                 ; TEST VALUE
023E  E8 0D15 R              951     C          CALL    OUT_DX
                             952     C                                          ; RETURN WITH CURSOR POS IN AX
                             953     C                                          ; RESTORE REGS USED
0241  5A                     954     C          POP     DX
0242  58                     955     C          POP     AX
0243  C3                     956     C          RET
0244                         957     C  WR_CURSOR       ENDP
                             958     C
                             959     C  POST:
                             960     C  ;--------------------------------------------------------:
                             961     C  ;       INITIALIZE AND START CRT CONTROLLER (6845)       :
                             962     C  ;       ON COLOR GRAPHICS AND MONOCHROME CARDS           :
                             963     C  ;       TEST VIDEO READ/WRITE STORAGE.                   :
                             964     C  ; DESCRIPTION                                            :
                             965     C  ;       RESET THE VIDEO ENABLE SIGNAL.                   :
                             966     C  ;       SELECT ALPHANUMERIC MODE, 40 * 25, 8 & W.        :
                             967     C  ;       READ/WRITE DATA PATTERNS TO STG. CHECK STG       :
                             968     C  ;       ADDRESSABILITY.                                  :
                             969     C  ;---------------------------------------------------------
                             970     C          ASSUME  DS:ABS0,ES:ABS0
0244  E8 0CFE R              971     C          CALL    DDS
0247  F6 06 0487 R 02        972     C          TEST    INFO,2
024C  75 12                  973     C          JNZ     COLOR_PRESENCE_TST
024E  B8 03B4                974     C          MOV     AX,03B4H
0251  E8 0205 R              975     C          CALL    CD_PRESENCE_TST
0254  3D 0001                976     C          CMP     AX,1
0257  74 03                  977     C          JE      CONT1
0259  E9 0317 R              978     C          JMP     P0D14
025C                         979     C  CONT1:
025C  B4 30                  980     C          MOV     AH,30H                  ; MONOCHROME CARD INSTALLED
025E  EB 10                  981     C          JMP     SHORT   OVER
0260                         982     C  COLOR_PRESENCE_TST:
0260  B8 03D4                983     C          MOV     AX,03D4H
0263  E8 0205 R              984     C          CALL    CD_PRESENCE_TST
0266  3D 0001                985     C          CMP     AX,1
0269  74 03                  986     C          JE      CONT2
026B  E9 0317                987     C          JMP     POD14
026E                         988     C  CONT2:
026E  B4 20                  989     C          MOV     AH,20H                  ; COLOR GRAPHICS CARD INSTALLED
0270                         990     C  OVER:
0270  50                     991     C          PUSH    AX                      ; RESAVE VALUE
0271  BB B000                992     C          MOV     BX,0B000H               ; BEG VIDEO RAM ADDR B/W CD
0274  BA 03B8                993     C          MOV     DX,3B8H                 ; MODE CONTROL B/W
0277  B9 1000                994     C          MOV     CX,4096                 ; RAM BYTE CNT FOR B/W CD
027A  B0 01                  995     C          MOV     AL,1                    ; SET MODE FOR B/W CARD
027C  80 FC 30               996     C          CMP     AH,30H                  ; B/W VIDEO CARD ATTACHED?
027F  74 08                  997     C          JE      E9                      ; YES - GO TEST VIDEO STG
0281  B7 B8                  998     C          MOV     BH,0B8H                 ; BEG VIDEO RAM ADDR COLOR CD
0283  B2 D8                  999     C          MOV     DL,0D8H                 ; MODE CONTROL COLOR
0285  B5 40                  1000    C          MOV     CH,40H                  ; RAM BYTE CNT FOR COLOR CD
0287  FE C8                  1001    C          DEC     AL                      ; SET MODE TO 0 FOR COLOR CD
0289                         1002    C  E9:                                     ; TEST_VIDEO_STG:
0289  EE                     1003    C          OUT     DX,AL                   ; DISABLE VIDEO FOR COLOR CD
                             1004    C
028A  8B 2E 0472 R           1005    C          MOV     BP,DS:RESET_FLAG        ; POD INITIALIZED BY KBD RESET
                             1006    C
028E  81 FD 1234             1007    C          CMP     BP,1234H                ; POD INITIATED BY KBD RESET?
0292  8E C3                  1008    C          MOV     ES,BX                   ; POINT ES TO VIDEO RAM STG
0294  73 07                  1009    C          JE      E10                     ; YES - SKIP VIDEO RAM TEST
0296  8E DB                  1010    C          MOV     DS,BX                   ; POINT DS TO VIDEO RAM STG
                             1011    C          ASSUME  DS:NOTHING,ES:NOTHING
0298  E8 02DF R              1012    C          CALL    STGTST_CNT              ; GO TEST VIDEO R/W STG
029B  75 2E                  1013    C          JNE     E17                     ; R/W STG FAILURE - BEEP SPK
                             1014    C  ;------------------------------------------------------------------
                             1015    C  ;       SETUP VIDEO DATA ON SCREEN FOR VIDEO LINE TEST.           :
                             1016    C  ; DESCRIPTION                                                     :
                             1017    C  ;       ENABLE VIDEO SIGNAL AND SET MODE.                         :
                             1018    C  ;       DISPLAY A HORIZONTAL BAR ON SCREEN.                       :
                             1019    C  ;------------------------------------------------------------------
029D                         1020    C  E10:
029D  58                     1021    C          POP     AX                      ; GET VIDEO SENSE SWS (AH)
029E  50                     1022    C          PUSH    AX                      ; SAVE IT
029F  B8 7020                1023    C          MOV     AX,7020H                ; WRT BLANKS IN REVERSE VIDEO
02A2  2B FF                  1024    C          SUB     DI,DI                   ; SETUP STARTING LOC
02A4  B9 0028                1025    C          MOV     CX,40                   ; NO. OF BLANKS TO DISPLAY
02A7  F3/ AB                 1026    C          REP     STOSW                   ; WRITE VIDEO STORAGE
                             1027    C  ;--------------------------------------------------------
                             1028    C  ;       CRT INTERFACE LINES TEST                        :
                             1029    C  ; DESCRIPTION                                           :
                             1030    C  ;       SENSE ON/OFF TRANSITION OF THE VIDEO ENABLE     :
                             1031    C  ;       AND HORIZONTAL SYNC LINES.                      :
                             1032    C  ;--------------------------------------------------------
02A9  58                     1033    C          POP     AX                      ; GET VIDEO SENSE SW INFO
02AA  50                     1034    C          PUSH    AX                      ; SAVE IT
02AB  80 FC 30               1035    C          CMP     AH,30H                  ; B/W CARD ATTACHED?
02AE  BA 03BA                1036    C          MOV     DX,03BAH                ; SETUP ADDR OF BW STATUS PORT
02B1  74 02                  1037    C          JE      E11                     ; YES - GO TEST LINES
02B3  B2 DA                  1038    C          MOV     DL,0DAH                 ; COLOR CARD IS ATTACHED
02B5                         1039    C  E11:                                    ; LINE_TST:
02B5  B4 08                  1040    C          MOV     AH,8
02B7                         1041    C  E12:                                    ; OFLOOP_CNT:
02B7  2B C9                  1042    C          SUB     CX,CX
02B9                         1043    C  E13:
02B9  EC                     1044    C          IN      AL,DX                   ; READ CRT STATUS PORT
02BA  22 C4                  1045    C          AND     AL,AH                   ; CHECK VIDEO/HORZ LINE
02BC  75 04                  1046    C          JNZ     E14                     ; ITS ON - CHECK IF IT GOES OFF
02BE  E2 F9                  1047    C          LOOP    E13                     ; LOOP TILL ON OR TIMEOUT
02C0  EB 09                  1048    C          JMP     SHORT E17               ; GO PRINT ERROR MSG
02C2                         1049    C  E14:
02C2  2B C9                  1050    C          SUB     CX,CX
02C4                         1051    C  E15:
02C4  EC                     1052    C          IN      AL,DX                   ; READ CRT STATUS PORT
02C5  22 C4                  1053    C          AND     AL,AH                   ; CHECK VIDEO/HORZ LINE
02C7  74 0A                  1054    C          JZ      E16                     ; ITS ON - CHECK NEXT LINE
02C9  E2 F9                  1055    C          LOOP    E15                     ; LOOP IF OFF TILL IT GOES ON
02CB                         1056    C  E17:                                    ;  CRT_ERR
02CB  BA 0102                1057    C          MOV     DX,102H
02CE  E8 06C8 R              1058    C          CALL    ERR_BEEP                ; GO BEEP SPEAKER
02D1  EB 06                  1059    C          JMP     SHORT E18
02D3                         1060    C  E16:                                    ; NXT_LINE
02D3  B1 03                  1061    C          MOV     CL,3                    ; GET NEXT BIT TO CHECK
02D5  D2 EC                  1062    C          SHR     AH,CL
02D7  75 DE                  1063    C          JNZ     E12                     ; GO CHECK HORIZONTAL LINE
02D9                         1064    C  E18:                                    ;  DISPLAY_CURSOR:
02D9  58                     1065    C          POP     AX                      ; GET VIDEO SENSE SWS (AH)
02DA  EB 3B                  1066    C          JMP     SHORT   POD14
                             1067    C
                             1068    C  ;------------------------------------------------------------------------
                             1069    C  ;       THIS SUBROUTINE PERFORMS A READ/WRITE STORAGE TEST ON           :
                             1070    C  ;       A 16K BLOCK OF STORAGE.                                         :
                             1071    C  ; ENTRY REQUIREMENTS:                                                   :
                             1072    C  ;       ES = ADDRESS OF STORAGE SEGMENT BEING TESTED                    :
                             1073    C  ;       DS = ADDRESS OF STORAGE SEGMENT BEING TESTED                    :
                             1074    C  ;       WHEN ENTERING AT STGTST_CNT, CX MUST BE LOADED WITH             :
                             1075    C  ;       THE BYTE COUNT.                                                 :
                             1076    C  ; EXIT PARAMETERS:                                                      :
                             1077    C  ;       ZERO FLAG = 0 IF STORAGE ERROR (DATA COMPARE OR PARITY CHECK.   :
                             1078    C  ;              AL = 0 DEONTES A PARITY CHECK. ELSE AL=XOR'ED BIT        :
                             1079    C  ;                     PATTERN OF THE EXPECTED DATA PATTERN VS THE       :
                             1080    C  ;                     ACTUAL DATA READ.                                 :
                             1081    C  ;       AX,BX,CX,DX,DI, AND SI ARE ALL DESTROYED.                       :
                             1082    C  ;------------------------------------------------------------------------
02DC                         1083    C  STGTST  PROC    NEAR
02DC  B9 4000                1084    C          MOV     CX,4000H                ; SETUP CNT TO TEST A 16K BLK
02DF                         1085    C  STGTST_CNT:
02DF  FC                     1086    C          CLD                             ; SET DIR FLAG TO INCREMENT
02E0  8B D9                  1087    C          MOV     BX,CX                   ; SAVE CNT (4K FOR VIDEO OR 16K)
02E2  B8 AAAA                1088    C          MOV     AX,0AAAAH               ; GET DATA PATTERN TO WRITE
02E5  BA FF55                1089    C          MOV     DX,0FF55H               ; SETUP OTHER DATA PATTERNS TO USE
02E8  2B FF                  1090    C          SUB     DI,DI                   ; DI = OFFSET 0 RELATIVE TO ES REG
02EA  F3/ AA                 1091    C          REP     STOSB                   ; WRITE STORAGE LOCATIONS
02EC                         1092    C  C3:                                     ;  STG01
02EC  4F                     1093    C          DEC     DI                      ; POINT TO LAST BYTE JUST WRITTEN
02ED  FD                     1094    C          STD                             ; SET DIR FLAG TO GO BACKWARDS
02EE                         1095    C  C4:
02EE  8B F7                  1096    C          MOV     SI,DI
02F0  8B CB                  1097    C          MOV     CX,BX                   ; SETUP BYTE CNT
02F2                         1098    C  C5:                                     ; INNER TEST LOOP
02F2  AC                     1099    C          LODSB                           ; READ OLD TEST BYTE [SI]+
02F3  32 C4                  1100    C          XOR     AL,AH                   ; DATA READ AS EXPECTED ?
02F5  75 1E                  1101    C          JNE     C7                      ;  NO - GO TO ERROR ROUTINE
02F7  8A C2                  1102    C          MOV     AL,DL                   ; GET NEXT DATA PATTERN TO WRITE
02F9  AA                     1103    C          STOSB                           ; WRITE INTO LOCATION JUST READ
02FA  E2 F6                  1104    C          LOOP    C5                      ;  DECREMENT COUNT AND LOOP CX
                             1105    C
02FC  22 E4                  1106    C          AND     AH,AH                   ; ENDING 0 PATTERN WRITTEN TO STG?
02FE  74 13                  1107    C          JZ      C6X                     ; YES - RETURN TO CALLER WITH AL=0
0300  8A E0                  1108    C          MOV     AH,AL                   ; SETUP NEW VALUE FOR COMPARE
0302  85 F2                  1109    C          XCHG    DH,DL                   ; MOVE NEXT DATA PATTERN TO DL
0304  22 E4                  1110    C          AND     AH,AH                   ; READING ZERO PATTERN THIS PASS ?
0306  75 04                  1111    C          JNZ     C6                      ; CONTINUE TEST SEQUENCE TILL 0
0308  8A D4                  1112    C          MOV     DL,AH                   ;  ELSE SET 0 FOR END READ PATTERN
030A  EB E0                  1113    C          JMP     C3                      ;  AND MAKE FINAL BACKWARDS PASS
030C                         1114    C  C6:
030C  FC                     1115    C          CLD                             ; SET DIR FLAG TO GO FORWARD
030D  47                     1116    C          INC     DI                      ; SET POINTER TO BEG LOCATION
030E  74 DE                  1117    C          JZ      C4                      ; READ/WRITE FORWARD IN STG
0310  4F                     1118    C          DEC     DI                      ; ADJUST POINTER
0311  EB D9                  1119    C          JMP     C3                      ; READ/WRITE BACKWARD IN STG
0313                         1120    C  C6X:
0313  B0 00                  1121    C          MOV     AL,000H                 ; AL=0 DATA COMPARE OK
0315                         1122    C  C7:
0315  FC                     1123    C          CLD                             ; SET DIRECTION FLAG BACK TO INC
0316  C3                     1124    C          RET
0317                         1125    C  STGTST  ENDP
                             1126    C
                             1127    C  ;--------------------------------------------------------
                             1128    C  ;               EGA CRT ATTACHMENT TEST                 :
                             1129    C  ;                                                       :
                             1130    C  ; 1. INIT CRT TO 40X25 - BW  ****SET TO MODE****        :
                             1131    C  ; 2. CHECK FOR VERTICAL AND VIDEO ENABLES. AND CHECK    :
                             1132    C  ;    TIMING OF SAME                                     :
                             1133    C  ; 3. CHECK VERTICAL INTERRUPT                           :
                             1134    C  ; 4. CHECK READ, BLUE, GREEN, AND INTENSIFY DOTS        :
                             1135    C  ; 5. INIT TO 40X25 - COLOR/MONO ****SET TO MODE****     :
                             1136    C  ;--------------------------------------------------------
                             1137    C
                             1138    C  ;----- NOMINAL TIME IS B286H FOR 60 HZ.
                             1139    C  ;----- NOMINAL TIME IS A2FEH FOR 50 HZ.
                             1140    C
= A0AC                       1141    C  MAX_VERT_COLOR  EQU     0A0ACH                  ; MAX TIME FOR VERT/VERT
                             1142    C                                                  ;  (NOMINAL + 10%)
= C460                       1143    C  MIN_VERT_COLOR  EQU     0C460H                  ; MIN TIME FOR VERT/VERT
                             1144    C                                                  ;  (NOMINAL - 10%)
= 00C8                       1145    C  CENAB_PER_FRAME EQU     200                     ; NUM OF ENABLES PER FRAME
= 8D99                       1146    C  MAX_VERT_MONO   EQU     08D99H                  ; MAX TIME FOR VERT/VERT
                             1147    C                                                  ;  (NOMINAL + 10%)
= B862                       1148    C  MIN_VERT_MONO   EQU     0B862H                  ; MIN TIME FOR VERT/VERT
                             1149    C                                                  ;  (NOMINAL - 10%)
= 015E                       1150    C  EENAB_PER_FRAME EQU     350                     ; ENHANCED ENABLES PER FRAME
= 015E                       1151    C  MENAB_PER_FRAME EQU     350                     ; NUM OF ENABLES PER FRAME
                             1152    C
= 0043                       1153    C  TIM_CTL         EQU     043H                    ; 0253 TIMER CONTROL PORT
= 0040                       1154    C  TIMER0          EQU     040H                    ; 8253 TIMER/CNTER 0 PORT
                             1155    C
0317                         1156    C  POD14   PROC    NEAR                            ; RESERVE 5 WORDS ON STACK
0317  83 EC 0A               1157    C          SUB     SP,0AH                          ; INIT SCRATCH PAD POINTER
031A  8B EC                  1158    C          MOV     SP,BP
                             1159    C
                             1160    C          ASSUME  DS:ABS0,ES:ABS0
031C  E8 0CFE R              1161    C          CALL    DDS
031F  B0 30                  1162    C          MOV     AL,00110000B                    ; SET TIMER 0 TO MODE 0
                             1163    C
0321  E6 43                  1164    C          OUT     TIM_CTL,AL
0323  B0 00                  1165    C          MOV     AL,00H
0325  E6 40                  1166    C          OUT     TIMER0,AL                       ; SEND FIRST BYTE TO TIMER
0327  F6 06 0487 R 02        1167    C          TEST    INFO,2
032C  74 1F                  1168    C          JZ      COLOR_EGA_V
032E  E8 0168 R              1169    C          CALL    ENV_3                           ; SET UP IN MONOCHROME
0331  C7 46 02 015E          1170    C          MOV     WORD PTR[BP][2],MENAB_PER_FRAME ; NUM.OF FRAMES FOR MONO
0336  C7 46 04 8D99          1171    C          MOV     WORD PTR[BP][4],MAX_VERT_MONO   ; MAX TIME FOR VERT/VERT
003B  C7 46 06 B862          1172    C          MOV     WORD PTR[BP][6],MIN_VERT_MONO   ; MIN TIME FOR VERT/VERT
0340  B2 B4                  1173    C          MOV     DL,CRTC_ADDR_B                  ; MONO CRTC REG
0342  B4 01                  1174    C          MOV     AH,C_HRZ_DSP                    ; HORIZ. TOTAL DIPLAY
0344  B0 27                  1175    C          MOV     AL,27H                          ;  TO 40 COL
0346  E8 0D15 R              1176    C          CALL    OUT_DX                          ;
0349  B2 BA                  1177    C          MOV     DL,INPUT_STATUS_B               ; 3BA
034B  EB 2A                  1178    C          JMP     SHORT   COMMON
034D                         1179    C  COLOR_EGA_V:
034D  E8 0148 R              1180    C          CALL    ENV_X                           ; SET UP IN 40X25 COLOR
0350  E8 0E9A R              1181    C          CALL    BRST_DET                        ; ENHANCED MODE
0353  7E 11                  1182    C          JNC     COLOR_V                         ; NO, 40X25
0355  B2 D4                  1183    C          MOV     DL,CRTC_ADDR                    ; BRST MODE ONLY!
0357  B4 01                  1184    C          MOV     AH,1                            ; HRZ DSP END
0359  B0 14                  1185    C          MOV     AH,20                           ; MODIFY FOR TEST ONLY
035B  E8 0D15 R              1186    C          CALL    OUT_DX
035E  C7 46 02 015E          1187    C          MOV     WORD PTR[BP][2],EENAB_PER_FRAME ; NUM.OF FRAMES FOR COLOR
0363  EB 06 90               1188    C          JMP     BRST_COLOR_V
0366                         1189    C  COLOR_V:
                             1190    C
0366  C7 46 02 00C8          1191    C          MOV     WORD PTR[BP][2],CENAB_PER_FRAME ; NUM.OF FRAMES FOR COLOR
036B                         1192    C  BRST_COLOR_V:
036B  C7 46 04 ADAC          1193    C          MOV     WORD PTR[BP][4],MAX_VERT_COLOR  ; MAX TIME FOR VERT/VERT
0370  C7 46 06 C460          1194    C          MOV     WORD PTR[BP][6],MIN_VERT_COLOR  ; MIN TIME FOR VERT/VERT
0375  B2 DA                  1195    C          MOV     DL,INPUT_STATUS                 ; SET ADDRESSING TO VIDEO
                             1196    C                                                  ;  ATTR STATUS
0377                         1197    C  COMMON:
0377  B8 0500                1198    C          MOV     AX,0500H                        ; SET TO VIDEO PAGE 0
037A  CD 10                  1199    C          INT     10H
037C  2B C9                  1200    C          SUB     CX,CX
                             1201    C
                             1202    C  ;----- LOOK FOR VERTICAL
                             1203    C
037E                         1204    C  POD14_1:
037E  EC                     1205    C          IN      AL,DX                           ; GET STATUS
037F  A8 08                  1206    C          TEST    AL,00001000B                    ; VERTICAL THERE YET?
0381  75 07                  1207    C          JNE     POD14_2                         ; CONTINUE IF IT IS
0383  E2 F9                  1208    C          LOOP    POD14_1                         ; KEEP LOOKING TILL COUNT
0385  B3 00                  1209    C          MOV     BL,00                           ;  EXHAUSTED
0387  E9 0448 R              1210    C          JMP     POD14_ERR                       ; NO VERTICAL
                             1211    C
                             1212    C  ;----- GOT VERTICAL - START TIMER
                             1213    C
038A                         1214    C  POD14_2:
038A  B0 00                  1215    C          MOV     AL,0
038C  E6 40                  1216    C          OUT     TIMER0,AL                       ; SEND 2ND BYTE TO TIMER TO
                             1217    C                                                  ; START IT
038E  2B DB                  1218    C          SUB     BX,BX                           ; INIT. ENABLE COUNTER
                             1219    C  ;----- WAIT FOR VERTICAL TO GO AWAY
0390  33 C9                  1220    C          XOR     CX,CX
0392                         1221    C  POD14_25:
0392  EC                     1222    C          IN      AL,DX                           ; GET STATUS
0393  A8 D8                  1223    C          TEST    AL,00001000B                    ; VERTICAL STILL THERE
0395  74 07                  1224    C          JZ      POD14_3                         ; CONTINUE IF IT'S GONE
0397  E2 F9                  1225    C          LOOP    POD14_25                        ; KEEP LOOKING TILL COUNT
0399  B3 01                  1226    C          MOV     BL,01H                          ;  EXHAUSTED
039B  E9 0448 R              1227    C          JMP     POD14_ERR                       ; VERTICAL STUCK ON
                             1228    C
                             1229    C  ;----- NOW START LOOKING FOR ENABLE TRANSITIONS
                             1230    C
039E                         1231    C  POD14_3:
039E  2B C9                  1232    C          SUB     CX,CX
03A0                         1233    C  POD14_4:
03A0  EC                     1234    C          IN      AL,DX                           ; GET STATUS
03A1  A8 01                  1235    C          TEST    AL,00000001B                    ; ENABLE ON YET?
03A3  74 15                  1236    C          JE      POD14_5                         ; GO ON IF IT IS
03A5  A8 08                  1237    C          TEST    AL,00001000B                    ; VERTICAL ON AGAIN?
03A7  75 23                  1238    C          JNE     POD14_75                        ; CONTINUE IF IT IS
03A9  E2 F5                  1239    C          LOOP    POD14_4                         ; KEEP LOOKING IF NOT
03AB  B3 02                  1240    C          MOV     BL,02H
03AD  E9 0448 R              1241    C          JMP     POD14_ERR                       ; ENABLE STUCK OFF
03B0                         1242    C  POD14_4A:
03B0  B3 03                  1243    C          MOV     BL,03H
03B2  E9 0448 R              1244    C          JMP     POD14_ERR                       ; VERTICAL STUCK ON
03B5                         1245    C  POD14_48:
03B5  B3 04                  1246    C          MOV     BL,04H
03B7  E9 0448 R              1247    C          JMP     POD14_ERR                       ; ENABLE STUCK ON
                             1248    C
                             1249    C  ;----- MAKE SURE VERTICAL WENT OFF WITH ENABLE GOING ON
                             1250    C
03BA                         1251    C  POD14_5:
03BA  A8 08                  1252    C          TEST    AL,00001000B                    ; VERTICAL OFF?
03BC  75 F2                  1253    C          JNZ      POD14_4A                       ; GO ON IF IT IS
                             1254    C  ;----- NOW WAIT FOR ENABLE TO GO OFF            ;  (ERROR IF NOT)
03BE                         1255    C  POD14_6:
03BE  EC                     1256    C          IN      AL,DX                           ; GET STATUS
03BF  A8 01                  1257    C          TEST    AL,00000001B                    ; ENABLE OFF YET?
03C1  E1 FB                  1258    C          LOOPE   POD14_6                         ; KEEP LOOKING IF NOT
03C3  E3 F0                  1259    C          JCXZ    POD14_4B                        ;  YET LOW
                             1260    C  ;----- ENABLE HAS TOGGLED, BUMP COUNTER AND TEST FOR NEXT VERTICAL
03C5                         1261    C  POD14_7:
03C5  43                     1262    C          INC     BX                              ; BUMP ENABLE COUNTER
03C6  74 04                  1263    C          JZ      POD14_75                        ; IF COUNTER WRAPS
                             1264    C                                                  ;  SOMETHING IS WRONG
03C8  A8 08                  1265    C          TEST    AL,00001000B                    ; DID ENABLE GO LOW
                             1266    C                                                  ;  BECAUSE OF VERTICAL
03CA  74 D2                  1267    C          JZ      POD14_3                         ; IF NOT, LOOK FOR ANOTHER
                             1268    C                                                  ;  ENABLE TOGGLE
                             1269    C  ;----- HAVE HAD COMPLETE VERTICAL-VERTICAL CYCLE, NOW TEST RESULTS
03CC                         1270    C  POD14_75:
03CC  B0 00                  1271    C          MOV     AL,00                           ; LATCH TIMER0
03CE  E6 43                  1272    C          OUT     TIM_CTL,AL
03D0  3B 5E 02               1773    C          CMP     BX,WORD PTR[BP][2]              ; NUMBER OF ENABLES BETWEEN
                             1774    C                                                  ;  VERTICALS O.K.?
03D3  74 04                  1275    C          JE      POD14_8
03D5  B3 05                  1276    C          MOV     BL,05H
03D7  EB 6F                  1277    C          JMP     SHORT POD14_ERR
03D9                         1278    C  POD14_8:
03D9  E4 40                  1279    C          IN      AL,TIMER0                       ; GET TIMER VALUE LOW
03DB  8A E0                  1280    C          MOV     AH,AL                           ; SAVE IT
03DD  90                     1281    C          NOP
03DE  E4 40                  1282    C          IN      AL,TIMER0                       ; GET TIMER HIGH
03E0  86 E0                  1283    C          XCHG    AH,AL
03E2  90                     1284    C          NOP
03E3  90                     1285    C          NOP
03E4  3B 46 04               1286    C          CMP     AX,WORD PTR[BP][4]              ; MAXIMUM VERTICAL TIMING
03E7  7F 04                  1287    C          JGE     POD14_9
03E9  B3 06                  1288    C          MOV     BL,06H
03EB  EB 5B                  1289    C          JMP     SHORT POD14_ERR
03ED                         1290    C  POD14_9:
03ED  3B 46 06               1291    C          CMP     AX,WORD PTR[BP][6]              ; MINIMUM VERTICAL TIMING
03F0  7E 04                  1292    C          JNE     POD14_10
03F2  B3 07                  1293    C          MOV     BL,07H
03F4  EB 52                  1294    C          JMP     SHORT   POD14_ERR
                             1295    C
                             1296    C  ;----- SEE IF RED, GREEN, BLUE AND INTENSIFY DOTS
                             1297    C
                             1298    C  ;----- FIRST, SET A LINE OF REVERSE VIDEO, INTENSIFIED BLANKS INTO BUFFER
03F6                         1299    C  POD14_10:
03F6  B8 09DB                1300    C          MOV     AX,09DBH                        ; WRITE CHARS, BLANKS
03F9  BB 000F                1301    C          MOV     BX,000FH                        ; PAGE 0, REVERSE VIDEO,
                             1302    C                                                  ;  HIGH INTENSITY
03FC  B9 0050                1303    C          MOV     CX,80                           ; 80 CHARACTERS
03FF  CD 10                  1304    C          INT     10H
0401  EC                     1305    C          IN      AL,DX
0402  52                     1306    C          PUSH    DX                              ; SAVE INPUT STATUS
0403  B2 C0                  1307    C          MOV     DL,ATTR_WRITE                   ; ATTRIBUTE ADDRESS
0405  B4 0F                  1308    C          MOV     AH,0FH                          ; PALETTE REG 'F'
0407  B0 3F                  1309    C          MOV     AL,03FH                         ; TEST VALUE
0409  E8 0D15 R              1310    C          CALL    OUT_DX                          ; VIDEO STATUS MUX
040C  B8 000F                1311    C          MOV     AX,0FH                          ; START WITH BLUE DOTS
040F  5A                     1312    C          POP     DX
0410                         1313    C  POP14_13:
0410  50                     1314    C          PUSH    AX                              ; SAVE
0411  52                     1315    C          PUSH    DX                              ; SAVE INPUT STATUS
0412  B2 C0                  1316    C          MOV     DL,ATTR_WRITE                   ; ATTRIBUTE ADDRESS
0414  B4 32                  1317    C          MOV     AH,32H                          ; COLOR PLANE ENABLE
0416  E8 0D15 R              1318    C          CALL    OUT_DX                          ; VIDEO STATUS MUX
0419  5A                     1319    C          POP     DX                              ; RECOVER INPUT STATUS
041A  58                     1320    C          POP     AX
041B  2B C9                  1321    C          SUB     CX,CX
                             1322    C  ;----- SEEE IF DOT COMES ON
041D                         1323    C  POD14_14:
041D  EC                     1324    C          IN      AL,DX                           ; GET STATUS
041E  A8 30                  1325    C          TEST    AL,00110000B                    ; DOT THERE?
0420  75 09                  1326    C          JNZ     POD14_15                        ; LOOK FOR DOT TO TURN OFF
0422  E2 F9                  1327    C          LOOP    POD14_14                        ; CONTINUE TEST FOR DOT ON
0424  B3 10                  1328    C          MOV     BL,10H
0426  0A DC                  1329    C          OR      BL,AH                           ; OR IN DOT BEING TESTED
0428  EB 1E 90               1330    C          JMP     POD14_ERR                       ; DOT NOT COMING ON
                             1331    C  ;----- SEE IF DOT GOES OFF
042B                         1332    C  POD14_15:
042B  2B C9                  1333    C          SUB     CX,CX
042D                         1334    C  POD14_16:
042D  EC                     1335    C          IN      AL,DX                           ; GET STATUS
042E  A8 30                  1336    C          TEST    AL,00110000B                    ; IS DOT STILL ON?
0430  74 08                  1337    C          JE      POD14_17                        ; GO ON IF DOT OFF
0432  E2 F9                  1338    C          LOOP    POD14_16                        ; ELSE, KEEP WAITING FOR
                             1339    C                                                  ;  DOT TO GO OFF
0434  B3 20                  1340    C          MOV     BL,20H
0436  0A DC                  1341    C          OR      BL,AH                           ; OR IN DOT BEING TESTED
0438  EB 0E                  1342    C          JMP     SHORT POD14_ERR
                             1343    C
                             1344    C  ;----- ADJUST TO POINT TO NEXT DOT
                             1345    C
043A                         1346    C  POD14_17:
043A  FE C4                  1347    C          INC     AH
043C  80 FC 30               1348    C          CMP     AH,030H                         ; ALL 3 DOTS DONE?
043F  74 25                  1349    C          JE      POD14_18                        ; GO END
0441  80 CC 0F               1350    C          OR      AH,0FH                          ; MAKE 0F,1F,2F
0444  8A C4                  1351    C          MOV     AL,AH
0446  EB C8                  1352    C          JMP     POD14_13                        ; GO LOOK FOR ANOTHER DOT
0448                         1353    C  POD14_ERR:
0448  B9 0006                1354    C          MOV     CX,6
044B  BA 0103                1355    C          MOV     DX,0103H                        ; ONE LONG AND THREE SHORT
044E  E8 06C8 R              1356    C          CALL    ERR_BEEP
0451  83 C4 0A               1357    C          ADD     SP,0AH                          ; BALANCE STACK
0454  B0 36                  1358    C          MOV     AL,00110110B                    ; RE-INIT TIMER 0
0456  E6 43                  1359    C          OUT     TIM_CTL,AL
0458  2A C0                  1360    C          SUB     AL,AL
045A  E6 40                  1361    C          OUT     TIMER0,AL
045C  90                     1362    C          NOP
045D  90                     1363    C          NOP
045E  E6 40                  1364    C          OUT     TIMER0,AL
0460  BD 0001                1365    C          MOV     BP,1
0463  E9 0091 R              1366    C          JMP     SKIP
                             1367    C          ASSUME  DS:ABS0
0466                         1368    C  POD14_18:
0466  EB 0CFE R              1369    C          CALL    DDS
0469  B8 0500                1370    C          MOV     AX,0500H                        ; SET TO VIDEO PAGE 0
046C  CD 10                  1371    C          INT     10H
046E  B0 36                  1372    C          MOV     AL,00110110B                    ; RE-INIT TIMER 0
0470  E6 43                  1373    C          OUT     TIM_CTL,AL
0472  2A C0                  1374    C          SUB     AL,AL
0474  E6 40                  1375    C          OUT     TIMER0,AL
0476  90                     1376    C          NOP
0477  90                     1377    C          NOP
0478  E6 40                  1378    C          OUT     TIMER0,AL
047A  83 C4 0A               1379    C          ADD     SP,0AH                          ; REMOVE SCRATCH PAD
047D  BD 0000                1380    C          MOV     BP,0                            ; MAKE BP NON ZERO
0480                         1381    C  POD14           ENDP
                             1382    C
                             1383    C  ;----- TEST STORAGE
                             1384    C
0480                         1385    C  MEM_TEST:
0480  1E                     1386    C          PUSH    DS
0481  E8                     1387    C          CALL    DDS
                             1388    C          ASSUME DS:ABS0
0484  F6 06 0487 R 02        1389    C          TEST    INFO,2
0489  74 12                  1390    C          JZ      D_COLOR_M
048B  80 0E 0410 R 30        1391    C          OR      EQUIP_LOW,030H
0490  B8 000F                1392    C          MOV     AX,0FH
0493  80 0E 0487 R 60        1393    C          OR      INFO,060H
0498  B8 000F                1394    C          MOV     AX,0FH
049B  EB 0D                  1395    C          JMP     SHORT D_OUT_M
049D                         1396    C  D_COLOR_M:
049D  80 26 0410 R CF        1397    C          AND     EQUIP_LOW,0CFH
04A2  80 0E 0410 R 20        1398    C          OR      EQUIP_LOW,020H
04A7  B8 000E                1399    C          MOV     AX,0EH                          ; INTERNAL COLOR MODE
04AA                         1400    C  D_OUT_M:                                        ; TEST IN COLOR
04AA  CD 10                  1401    C          INT     10H
04AC  83 EC 06               1402    C          SUB     SP,6                            ; RESERVE 3 WORDS ON STACK
04AF  8B EC                  1403    C          MOV     BP,SP                           ; SET BP
04B1  B8 A000                1404    C          MOV     AX,0A000H                       ; PUT BUFFER ADDRESS IN AX
                             1405    C          ASSUME  DS:NOTHING,ES:NOTHING
04B4  8E D8                  1406    C          MOV     DS,AX                           ; SET UP SEG REGS TO POINT
04B6  8E C0                  1407    C          MOV     ES,AX                           ;  TO BUFFER AREA
04B8  C7 46 02 0000          1408    C          MOV     WORD PTR[BP][2],0               ; INITIALIZE
04BD  C7 46 04 0000          1409    C          MOV     WORD PTR[BP][4],0               ; INITIALIZE
04C2  B6 03                  1410    C          MOV     DH,3
04C4  B2 C4                  1411    C          MOV     DL,SEQ_ADDR
04C6  B8 0201                1412    C          MOV     AX,0201H
04C9  E8 0D15 R              1413    C          CALL    OUT_DX
04CC  B2 CE                  1414    C          MOV     DL,GRAPH_ADDR                   ; ADDRESS READ MAP SELECT
04CE  B8 0400                1415    C          MOV     AX,0400H
04D1  E8 0D15 R              1416    C          CALL    OUT_DX
04D4  52                     1417    C          PUSH    DX
04D5  B2 DA                  1418    C          MOV     DL,ATTR_READ                    ; SET UP ATTRIBUTE
04D7  EC                     1419    C          IN      AL,DX
04D8  B2 C0                  1420    C          MOV     DL,ATTR_WRITE                   ; ATTRIBUTE WRITE ADDRESS
04DA  B8 3200                1421    C          MOV     AX,3200H
04DD  E8 0D15 R              1422    C          CALL    OUT_DX
04E0  E8 068F R              1423    C          CALL    HOW_BIG                         ; GO FIND AMOUNT OF MEMORY
04E3  80 FC 00               1424    C          CMP     AH,0
04E6  74 03                  1425    C          JZ      AA1
04E8  E9 05CD R              1426    C          JMP     EGA_MEM_ERROR
04EB                         1427    C  AA1:
04EB  E8 05D9 R              1428    C          CALL    MEMORY_OK                       ; GO TEST IT
04EE  80 FC 00               1429    C          CMP     AH,0
04F1  74 03                  1430    C          JZ      AA2
04F3  E9 05CD R              1431    C          JMP     EGA_MEM_ERROR
04F6                         1432    C  AA2:
04F6  5A                     1433    C          POP     DX
04F7  B2 C4                  1434    C          MOV     DL,SEQ_ADDR
04F9  B8 0202                1435    C          MOV     AX,020H
04FC  E8 0D15 R              1436    C          CALL    OUT_DX
04FF  B2 CE                  1437    C          MOV     DL,GRAPH_ADDR                   ; ADDRESS OF READ MAP
0501  B8 0401                1438    C          MOV     AX,0401H
0504  E8 0D15 R              1439    C          CALL    OUT_DX
0507  52                     1440    C          PUSH    DX
0508  B2 DA                  1441    C          MOV     DL,ATTR_READ                    ; SET UP ATTRIBUTE
050A  EC                     1442    C          IN      AL,DX
050B  B2 C0                  1443    C          MOV     DL,ATTR_WRITE                   ; ATTRIBUTE WRITE ADDRESS
050D  B8 3200                1444    C          MOV     AX,3200H
0510  E8 0D15 R              1445    C          CALL    OUT_DX
0513  C7 46 04 0000          1446    C          MOV     WORD PTR [BP][4],0              ; INITIALIZE
0518  E8 068F R              1447    C          CALL    HOW_BIG                         ; GO FIND AMOUNT OF MEMORY
051B  80 FC 00               1448    C          CMP     AH,0
051E  74 03                  1449    C          JZ      AA3
0520  E9 05CD R              1450    C          JMP     EGA_MEM_ERROR
0523                         1451    C  AA3:
0523  E8 05D9 R              1452    C          CALL    MEMORY_OK                       ; GO TEST IT
0526  80 FC 00               1453    C          CMP     AH,0
0529  74 03                  1454    C          JZ      AA4
052B  E9 05CD R              1455    C          JMP     EGA_MEM_ERROR
052E                         1456    C  AA4:
052E  5A                     1457    C          POP     DX
052F  B2 C4                  1458    C          MOV     DL,SEQ_ADDR
0531  B8 0204                1459    C          MOV     AX,0204H
0534  E8 0D15 R              1460    C          CALL    OUT_DX
0537  52                     1461    C          PUSH    DX
0538  B2 CE                  1462    C          MOV     DL,GRAPH_ADDR                   ; ADDRESS OF READ MAP
053A  B8 0402                1463    C          MOV     AX,0402H
053D  E8 0D15 R              1464    C          CALL    OUT_DX
0540  B2 DA                  1465    C          MOV     DL,ATTR_READ                    ; SET UP ATTRIBUTE
0542  EC                     1466    C          IN      IN,DX
0543  B2 C0                  1467    C          MOV     DL,ATTR_WRITE                   ; ATTRIBUTE WRITE ADDRESS
0545  B8 3200                1468    C          MOV     AX,3200H
0548  E8 0D15 R              1469    C          CALL    OUT_DX
054B  C7 46 04 0000          1470    C          MOV     WORD PTR[BP][4],0               ; INITIALIZE
0550  E8 068F R              1471    C          CALL    HOW_BIG                         ; GO FIND AMOUNT OF MEMORY
0553  80 FC 00               1472    C          CMP     AH,0
0556  74 03                  1473    C          JZ      AA5
0558  EB 73 90               1474    C          JMP     EGA_MEM_ERROR
055B                         1475    C  AA5:
055B  E8 05D9 R              1476    C          CALL    MEMORY_OK                       ; GO TEST IT
055E  80 FC 00               1477    C          CMP     AH,0
0561  74 03                  1478    C          JZ      AA6
0563  EB 68 90               1479    C          JMP     EGA_MEM_ERROR
0566                         1480    C  AA6:
0566  5A                     1481    C          POP     DX
0567  B2 C4                  1482    C          MOV     DL,SEQ_ADDR
0569  B8 0208                1483    C          MOV     AX,0208H
056C  E8 0D15 R              1484    C          CALL    OUT_DX
056F  B2 CE                  1485    C          MOV     DL,GRAPH_ADDR                   ; ADDRESS OF READ MAP
0571  B8 0403                1486    C          MOV     AX,0403H
0574  E8 0D15 R              1487    C          CALL    OUT_DX
0577  52                     1488    C          PUSH    DX
0578  B2 DA                  1489    C          MOV     DL,ATTR_READ                    ; SET UP ATTRIBUTE
057A  EC                     1490    C          IN      AL,DX
057B  B2 C0                  1491    C          MOV     DL,ATTR_WRITE                   ; ATTRIBUTE WRITE ADDRESS
057D  B8 3200                1492    C          MOV     AX,3200H
0580  E8 0D15 R              1493    C          CALL    OUT_DX
0583  C7 46 04 0000          1494    C          MOV     WORD PTR[BP][4],0               ; INITIALIZE
0588  E8 068F R              1495    C          CALL    HOW_BIG                         ; GO FIND AMOUNT OF MEMORY
058B  80 FC 00               1496    C          CMP     AH,0
058E  75 3D                  1497    C          JNZ     EGA_MEM_ERROR
0590  E8 05D9 R              1498    C          CALL    MEMORY_OK                       ; GO TEST IT
0593  80 FC 00               1499    C          CMP     AH,0
0596  75 35                  1500    C          JNZ     EGA_MEM_ERROR
0598  55                     1501    C          PUSH    BP                              ; SAVE SCRATCH PAD POINTER
0599  BD 0000                1502    C          MOV     BP,0                            ; RESET BP FOR XT
059C                         1503    C  EGA_MEM_EXIT:
059C  5E                     1504    C          POP     SI                              ; RESTORE
059D  5A                     1505    C          POP     DX
059E  E8 0CFE R              1506    C          CALL    DDS                             ; SET DATA SEGMENT
                             1507    C          ASSUME  DS:ABS0
05A1  E6: 8B 5C 02           1508    C          MOV     BX,WORD PTR SS:[SI][2]          ; GET EGA MEMORY SIZE
05A5  B1 06                  1509    C          MOV     CL,06H                          ; DIVIDE BY 64 TO GET
05A7  D3 EB                  1510    C          SHR     BX,CL                           ;  NUMBER OF 64KB BLOCKS
05A9  4B                     1511    C          DEC     BX
05AA  B1 05                  1512    C          MOV     CL,05H
05AC  D3 E3                  1513    C          SHL     BX,CL                           ; ISOLATE BITS 5 AND 6
05AE  80 E3 60               1514    C          AND     BL,01100000B
                             1515    C
05B1  80 26 0487 R 9F        1516    C          AND     INFO,10011111B
                             1517    C
05B6  08 1E 0487 R           1518    C          OR      INFO,BL
                             1519    C
05BA  80 0E 0487 R 04        1520    C          OR      INFO,00000100B                  ; 04H  SET 3XX ACTIVE
05BF  8A 1E 0488 R           1521    C          MOV     BL,INFO_3
05C3  E8 00F3 R              1522    C          CALL    MK_ENV
05C6  83 C4 06               1523    C          ADD     SP,6                            ; RESTORE STACK
05C9  1F                     1524    C          POP     DS
05CA  E9 0091 R              1525    C          JMP     SKIP                            ; GO TO END
05CD                         1526    C  EGA_MEM_ERROR:
05CD  BA 0103                1527    C          MOV     DX,0103H                        ; ONE LONG AND THREE SHORT
05D0  E8 06C8 R              1528    C          CALL    ERR_BEEP
05D3  55                     1529    C          PUSH    BP                              ; SAVE SCRATCH PAD POINTER
05D4  BD 0001                1530    C          MOV     BP,1                            ; INDICATE ERROR FOR XT
05D7  EB C3                  1531    C          JMP     EGA_MEM_EXIT
                             1532    C
                             1533    C  ;----- THIS ROUTINE FINDS AMOUNT OF MEMORY GOOD
                             1534    C
05D9                         1535    C  MEMORY_OK       PROC    NEAR
05D9  BB A000                1536    C          MOV     BX,0A000H                       ; SET PTR. TO BUFFER SEG
05DC  8E DB                  1537    C          MOV     DS,BX                           ; SET REG.REG
05DE  8E C3                  1538    C          MOV     ES,BX
05E0  8B 46 04               1539    C          MOV     AX,WORD PTR[BP][4]              ; SET COUNT FOR 32K WORDS
05E3  8A 38                  1540    C          MOV     CH,AL                           ; SET AMOUNT OF BUFFER
05E5  2A C9                  1541    C          SUB     CL,CL                           ;  TO BE TESTED
05E7  D1 E1                  1542    C          SHL     CX,1                            ; MULTIPLY BY TWO
05E9  E8 08FB R              1543    C          CALL    PODSTG
05EC  80 FC 00               1544    C          CMP     AH,0                            ; TEST FOR ERROR
05EF  75 09                  1545    C          JNZ     MEMORY_OK_ERR                   ; IF ERROR GO PRINT IT
05F1                         1546    C  MEMORY_OK_EX:
05F1  8B 46 04               1547    C          MOV     AX,WORD PTR[BP][4]              ; AMOUNT OF MEMORY FOUND
05F4  01 46 02               1548    C          ADD     WORD PTR[BP][2],AX              ; AMOUNT OF MEMORY GOOD
05F7  B8 0000                1549    C          MOV     AX,0
05FA                         1550    C  MEMORY_OK_ERR:
05FA  C3                     1551    C          RET
05FB                         1552    C  MEMORY_OK       ENDP
                             1553    C
                             1554    C  ;----------------------------------------------------------------
                             1555    C  ; THIS ROUTINE PERFORMS A READ/WRITE TEST ON A BLOCK OF STORAGE :
                             1556    C  ; (MAX. SIZE = 32KW). IF "WARM START", FILL BLOCK WITH 0000 AND :
                             1557    C  ; RETURN.                                                       :
                             1558    C  ; ON ENTRY:                                                     :
                             1559    C  ;   ES = ADDRESS OF STORAGE TO BE TESTED                        :
                             1560    C  ;   DS = ADDRESS OF STORAGE TO BE TESTED                        :
                             1561    C  ;   CX = WORD COUNT OF STORAGE BLOCK TO BE TESTED               :
                             1562    C  ;        (MAX. = 8000H (32K WORDS))                             :
                             1563    C  ; ON EXIT:                                                      :
                             1564    C  ;   ZERO FLAG = OFF IF STORAGE ERROR                            :
                             1565    C  ; AX,BX,CX,DX,DI,SI ARE ALL DESTROYED.                          :
                             1566    C  ;----------------------------------------------------------------
05FB                         1567    C  PODSTG  PROC    NEAR
05FB  55                     1568    C          PUSH    BP
05FC  FC                     1569    C          CLD                                     ; SET DIR TO INCREMENT
05FD  2B FF                  1570    C          SUB     DI,DI                           ; SET DI=0000 REL TO START
                             1571    C                                                  ;  OF SEGMENT
05FF  2B C0                  1572    C          SUB     AX,AX                           ; INITIAL DATA PATTERN FOR
                             1573    C                                                  ;  O0-FF TEST
0601  E8 0CFE R              1574    C          CALL    DDS
                             1575    C          ASSUME  DS:ABS0
0604  8B 1E 0472 R           1576    C          MOV     BX,DS:RESET_FLAG                ; WARM START?
0608  81 FB 1234             1577    C          CMP     BX,1234H
060C  8C C2                  1578    C          MOV     DX,ES
060E  8E DA                  1579    C          MOV     DS,DX                           ; RESTORE DS
0610  74 62                  1580    C          JE      PODSTG_5                        ; GO DO FILL WITH 0000
                             1581    C                                                  ;  IF WARM START
0612  81 FB 4321             1582    C          CMP     BX,4321H                        ; DCP WARM START?
0616  74 5C                  1583    C          JE      PODSTG_5                        ; DO FILL IF SO
0618                         1584    C  PODSTG_1:
0618  88 05                  1585    C          MOV     [DI],AL                         ; WRITE TEST DATA
061A  8A 05                  1586    C          MOV     AL,[DI]                         ; GET IT BACK
061C  32 C4                  1587    C          XOR     AL,AH                           ; COMPARE TO EXPECTED
061E  75 40                  1588    C          JNZ     PODSTG_ERR0                     ; ERROR EXIT IF MISCOMPARE
0620  FE C4                  1589    C          INC     AH                              ; FORM NEW DATA PATTERN
0622  8A C4                  1590    C          MOV     AL,AH
0624  75 F2                  1591    C          JNZ     PODSTG_1                        ; LOOP TILL ALL 256 DATA
                             1592    C                                                  ;  PATTERNS DONE
0626  8B E9                  1593    C          MOV     BP,CX                           ; SAVE WORD COUNT
0628  B8 AA55                1594    C          MOV     AX,0AA55H                       ; LOAD DATA PATTERN
062B  8B D8                  1595    C          MOV     BX,AX
062D  BA 55AA                1596    C          MOV     DX,055AAH                       ; LOAD OTHER DATA PATTERN
0630  F3/ AB                 1597    C          REP     STOSW                           ; FILL WORDS FROM LOW TO
                             1598    C                                                  ;  HIGH WITH AAAA
0632  4F                     1599    C          DEC     DI                              ; POINT TO LAST WORD
0633  4F                     1600    C          DEC     DI                              ;  WRITTEN
0634  FD                     1601    C          STD                                     ; SET DIR FLAG TO GO DOWN
0635  8B F7                  1602    C          MOV     SI,DI                           ; SET INDEX REGS. EQUAL
0637  8B CD                  1603    C          MOV     CX,BP                           ; RECOVER WORD COUNT
0639                         1604    C  PODSTG_2:                                       ; GO FROM HIGH TO LOW
0639  AD                     1605    C          LODSW                                   ; GET WORD FROM MEMORY
063A  33 C3                  1606    C          XOR     AX,BX                           ; EQUAL WHAT S/B THERE?
063C  75 22                  1607    C          JNZ     PODSTG_ERR0                     ; GO ERROR EXIT IF NOT
063E  8B C2                  1608    C          MOV     AX,DX                           ; GET 55 DATA PATTERN AND
0640  AB                     1609    C          STOSW                                   ;  STORE IN LOC JUST READ
0641  E2 F6                  1610    C          LOOP    PODSTG_2                        ; LOOP TILL ALL BYTES DONE
0643  8B CD                  1611    C          MOV     CX,BP                           ; RECOVER WORD COUNT
0645  FC                     1612    C          CLD                                     ; BACK TO INCREMENT
0646  46                     1613    C          INC     SI                              ; ADJUST PTRS
0647  46                     1614    C          INC     SI
0648  8B FE                  1615    C          MOV     DI,SI
064A                         1616    C  PODSTG_3:                                       ; LOW TO HIGH DOING WRODS
064A  AD                     1617    C          LODSW                                   ; GET A WORD
064B  33 C2                  1618    C          XOR     AX,DX                           ; SHOULD COMPARE TO DX
064D  75 11                  1619    C          JNZ     PODSTG_ERR0                     ; GO ERROR IF NOT
064F  AB                     1620    C          STOSW                                   ; WRITE 0000 BACK TO LOC
                             1621    C                                                  ;  JUST READ
0650  E2 F8                  1622    C          LOOP    PODSTG_3                        ; LOOP TILL DONE
                             1623    C
0652  FD                     1624    C          STD                                     ; BACK TO DECREMENT
0653  4E                     1625    C          DEC     SI                              ; ADJUST POINTER DOWN TO
                             1626    C                                                  ;  LAST WORD WRITTEN
0654  4E                     1627    C          DEC     SI
0655  8B CD                  1628    C          MOV     CX,BP                           ; GET WORD COUNT
0657                         1629    C  PODSTG_4:
0657  AD                     1630    C          LODSW                                   ; GET WORD
0658  08 C0                  1631    C          OR      AX,AX                           ; = TO 0000
065A  75 04                  1632    C          JNZ     PODSTG_ERR0                     ; ERROR IF NOT
065C  E2 F9                  1633    C          LOOP    PODSTG_4                        ; LOOP TILL DONE
065E  EB 11                  1634    C          JMP     SHORT PODSTG_ERR2
0660                         1635    C  PODSTG_ERR0:
0660  8B C8                  1636    C          MOV     CX,AX                           ; SAVE BITS IN ERROR
0662  32 E4                  1637    C          XOR     AH,AH
0664  0A ED                  1638    C          OR      CH,CH                           ; HIGH BYTE ERROR?
; WARNING: THIS FILE IS NOT YET FINISHED! USE OF THIS FILE MAY CAUSE UNEXPECTED RESULTS AND IN EXTREME CIRCUMSTANCES MAY CAUSE DATA CORRUPTION!
