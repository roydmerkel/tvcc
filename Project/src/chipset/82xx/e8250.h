/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/chipset/82xx/e8250.h                                     *
 * Created:     2003-08-25 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2003-2011 Hampa Hug <hampa@hampa.ch>                     *
 *****************************************************************************/

/*****************************************************************************
 * This program is free software. You can redistribute it and / or modify it *
 * under the terms of the GNU General Public License version 2 as  published *
 * by the Free Software Foundation.                                          *
 *                                                                           *
 * This program is distributed in the hope  that  it  will  be  useful,  but *
 * WITHOUT  ANY   WARRANTY,   without   even   the   implied   warranty   of *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  General *
 * Public License for more details.                                          *
 *****************************************************************************/


/* UART 8250/16450/16550 */


#ifndef PCE_E8250_H
#define PCE_E8250_H 1


/* specific chip types */
#define E8250_CHIP_8250  0
#define E8250_CHIP_16450 1

/* register offsets */
#define E8250_RXD     0x00
#define E8250_TXD     0x00
#define E8250_DIV_LO  0x00
#define E8250_DIV_HI  0x01
#define E8250_IER     0x01
#define E8250_IIR     0x02
#define E8250_LCR     0x03
#define E8250_MCR     0x04
#define E8250_LSR     0x05
#define E8250_MSR     0x06
#define E8250_SCRATCH 0x07

/* Interrupt Enable Register */
#define E8250_IER_MSR  0x08
#define E8250_IER_ERBK 0x04
#define E8250_IER_TBE  0x02
#define E8250_IER_RRD  0x01

/* Interrupt Indication Register */
#define E8250_IIR_MSR  (0x00 << 1)
#define E8250_IIR_TBE  (0x01 << 1)
#define E8250_IIR_RRD  (0x02 << 1)
#define E8250_IIR_ERBK (0x03 << 1)
#define E8250_IIR_PND  0x01

/* Line Control Register */
#define E8250_LCR_DLAB   0x80
#define E8250_LCR_BRK    0x40
#define E8250_LCR_PARITY 0x38
#define E8250_LCR_STOP   0x04
#define E8250_LCR_DATA   0x03

/* parity codes */
#define E8250_PARITY_N 0x00
#define E8250_PARITY_O 0x01
#define E8250_PARITY_E 0x03
#define E8250_PARITY_M 0x05
#define E8250_PARITY_S 0x07

/* Line Status Register */
#define E8250_LSR_TXE 0x40
#define E8250_LSR_TBE 0x20
#define E8250_LSR_RRD 0x01

/* Modem Control Register */
#define E8250_MCR_LOOP 0x10
#define E8250_MCR_OUT2 0x08
#define E8250_MCR_OUT1 0x04
#define E8250_MCR_RTS  0x02
#define E8250_MCR_DTR  0x01

/* Modem Status Register */
#define E8250_MSR_DCD  0x80
#define E8250_MSR_RI   0x40
#define E8250_MSR_DSR  0x20
#define E8250_MSR_CTS  0x10
#define E8250_MSR_DDCD 0x08
#define E8250_MSR_DRI  0x04
#define E8250_MSR_DDSR 0x02
#define E8250_MSR_DCTS 0x01


#define E8250_BUF_MAX 256


/*!***************************************************************************
 * @short The UART 8250 structure
 *****************************************************************************/
typedef struct {
	unsigned       chip;

	/* input buffer. holds bytes that have not yet been received. */
	unsigned       inp_i;
	unsigned       inp_j;
	unsigned       inp_n;
	unsigned char  inp[E8250_BUF_MAX];

	/* output buffer. holds bytes that have been sent. */
	unsigned       out_i;
	unsigned       out_j;
	unsigned       out_n;
	unsigned char  out[E8250_BUF_MAX];

	unsigned char  txd;
	unsigned char  rxd;
	unsigned char  ier;
	unsigned char  iir;
	unsigned char  lcr;
	unsigned char  lsr;
	unsigned char  mcr;
	unsigned char  msr;
	unsigned char  scratch;

	char           tbe_ack;

	/* (input clock) / (bit clock) */
	unsigned       bit_clk_div;

	char           clocking;

	unsigned       read_clk_cnt;
	unsigned       read_clk_div;
	unsigned       read_char_cnt;
	unsigned       read_char_max;

	unsigned       write_clk_cnt;
	unsigned       write_clk_div;
	unsigned       write_char_cnt;
	unsigned       write_char_max;

	/* enables or disables the scratch register */
	unsigned char  have_scratch;

	unsigned short divisor;

	unsigned char  irq_val;
	void           *irq_ext;
	void           (*irq) (void *ext, unsigned char val);

	/* output buffer is not empty */
	void           *send_ext;
	void           (*send) (void *ext, unsigned char val);

	/* input buffer is not full */
	void           *recv_ext;
	void           (*recv) (void *ext, unsigned char val);

	/* setup (DIV, MCR or LCR) has changed */
	void           *setup_ext;
	void           (*setup) (void *ext, unsigned char val);

	/* MSR is about to be read */
	void           *check_ext;
	void           (*check) (void *ext, unsigned char val);
} e8250_t;


/*!***************************************************************************
 * @short Initialize an UART
 * @param uart The UART structure
 *****************************************************************************/
void e8250_init (e8250_t *uart);

/*!***************************************************************************
 * @short  Create and initialize an UART
 * @return A new UART structure or NULL on error
 *****************************************************************************/
e8250_t *e8250_new (void);

/*!***************************************************************************
 * @short Free the resources used by an UART structure
 * @param uart The UART structure
 *****************************************************************************/
void e8250_free (e8250_t *uart);

/*!***************************************************************************
 * @short Delete an UART structure
 * @param uart The UART structure
 *****************************************************************************/
void e8250_del (e8250_t *uart);


/*!***************************************************************************
 * @short Set the emulated chip type to 8250
 * @param uart The UART structure
 *****************************************************************************/
void e8250_set_chip_8250 (e8250_t *uart);

/*!***************************************************************************
 * @short Set the emulated chip type to 16450
 * @param uart The UART structure
 *****************************************************************************/
void e8250_set_chip_16450 (e8250_t *uart);

/*!***************************************************************************
 * @short Set the emulated chip type
 * @param uart The UART structure
 * @param chip The chip type
 *****************************************************************************/
int e8250_set_chip (e8250_t *uart, unsigned chip);

/*!***************************************************************************
 * @short Set the emulated chip type
 * @param uart The UART structure
 * @param str  The chip type as a string
 *****************************************************************************/
int e8250_set_chip_str (e8250_t *uart, const char *str);


/*!***************************************************************************
 * @short Set the IRQ function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called on IRQ
 *****************************************************************************/
void e8250_set_irq_fct (e8250_t *uart, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the send function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called when the output queue is not empty
 *****************************************************************************/
void e8250_set_send_fct (e8250_t *uart, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the receive function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called when the input queue is not full
 *****************************************************************************/
void e8250_set_recv_fct (e8250_t *uart, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the setup function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called when the UART setup changes
 *****************************************************************************/
void e8250_set_setup_fct (e8250_t *uart, void *ext, void *fct);

/*!***************************************************************************
 * @short Set the check function
 * @param uart The UART structure
 * @param ext  The transparent parameter for fct
 * @param fct  The function to be called when the MSR is about to be read
 *****************************************************************************/
void e8250_set_check_fct (e8250_t *uart, void *ext, void *fct);


/*!***************************************************************************
 * @short Set the input and output queue sizes
 * @param uart The UART structure
 * @param inp  The input queue size
 * @param out  The output queue size
 *****************************************************************************/
void e8250_set_buf_size (e8250_t *uart, unsigned inp, unsigned out);

/*!***************************************************************************
 * @short Set the transmission rate
 *****************************************************************************/
void e8250_set_multichar (e8250_t *uart, unsigned read_max, unsigned write_max);

/*!***************************************************************************
 * @short Set the bit clock divisor
 *****************************************************************************/
void e8250_set_bit_clk_div (e8250_t *uart, unsigned div);

/*!***************************************************************************
 * @short  Get the rate divisor
 * @param  uart The UART structure
 * @return The 16 bit divisor
 *****************************************************************************/
unsigned short e8250_get_divisor (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the data rate
 * @param  uart The UART structure
 * @return The data rate in bits per second
 *****************************************************************************/
unsigned long e8250_get_bps (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the number of data bits per word
 * @param  uart The UART structure
 * @return The number of data bits per word
 *****************************************************************************/
unsigned e8250_get_databits (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the number of stop bits
 * @param  uart The UART structure
 * @return The number of stop bits
 *****************************************************************************/
unsigned e8250_get_stopbits (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the parity
 * @param  uart The UART structure
 * @return The parity
 *****************************************************************************/
unsigned e8250_get_parity (e8250_t *uart);


/*!***************************************************************************
 * @short  Get the DTR output signal
 * @param  uart The UART structure
 * @return The state of the DTR signal
 *****************************************************************************/
int e8250_get_dtr (e8250_t *uart);

/*!***************************************************************************
 * @short  Get the RTS output signal
 * @param  uart The UART structure
 * @return The state of the RTS signal
 *****************************************************************************/
int e8250_get_rts (e8250_t *uart);


/*!***************************************************************************
 * @short  Set the DSR input signal
 * @param  uart The UART structure
 * @return The state of the DSR signal
 *****************************************************************************/
void e8250_set_dsr (e8250_t *uart, unsigned char val);

/*!***************************************************************************
 * @short  Set the CTS input signal
 * @param  uart The UART structure
 * @return The state of the CTS signal
 *****************************************************************************/
void e8250_set_cts (e8250_t *uart, unsigned char val);

/*!***************************************************************************
 * @short  Set the DCD input signal
 * @param  uart The UART structure
 * @return The state of the DCD signal
 *****************************************************************************/
void e8250_set_dcd (e8250_t *uart, unsigned char val);

/*!***************************************************************************
 * @short  Set the RI input signal
 * @param  uart The UART structure
 * @return The state of the RI signal
 *****************************************************************************/
void e8250_set_ri (e8250_t *uart, unsigned char val);


/*!***************************************************************************
 * @short  Add a byte to the input queue
 * @param  uart The UART structure
 * @param  val  The byte
 * @return Nonzero on error (queue full)
 *****************************************************************************/
int e8250_set_inp (e8250_t *uart, unsigned char val);

/*!***************************************************************************
 * @short  Clear the input queue
 * @param  uart The UART structure
 *****************************************************************************/
void e8250_get_inp_all (e8250_t *uart);

/*!***************************************************************************
 * @short  Check if the input queue is full
 * @param  uart The UART structure
 * @return Nonzero if the queue is full
 *****************************************************************************/
int e8250_inp_full (e8250_t *uart);

/*!***************************************************************************
 * @short  Check if the input queue is empty
 * @param  uart The UART structure
 * @return Nonzero if the queue is empty
 *****************************************************************************/
int e8250_inp_empty (e8250_t *uart);


/*!***************************************************************************
 * @short  Get a byte from the output queue
 * @param  uart   The UART structure
 * @retval val    The byte
 * @param  remove If true, remove the byte from the output queue
 * @return Nonzero on error (queue empty)
 *****************************************************************************/
int e8250_get_out (e8250_t *uart, unsigned char *val, int remove);

/*!***************************************************************************
 * @short  Clear the output queue
 * @param  uart The UART structure
 *****************************************************************************/
void e8250_get_out_all (e8250_t *uart);

/*!***************************************************************************
 * @short  Check if the output queue is empty
 * @param  uart The UART structure
 * @return Nonzero if the queue is empty
 *****************************************************************************/
int e8250_out_empty (e8250_t *uart);


/*!***************************************************************************
 * @short  Send a byte
 * @param  uart The UART structure
 * @param  val  The byte
 * @return Nonzero on error (output queue empty)
 *****************************************************************************/
int e8250_send (e8250_t *uart, unsigned char *val);

/*!***************************************************************************
 * @short  Receive a byte
 * @param  uart The UART structure
 * @param  val  The byte
 * @return Nonzero on error (input queue full)
 *****************************************************************************/
int e8250_receive (e8250_t *uart, unsigned char val);


unsigned char e8250_get_div_lo (e8250_t *uart);
unsigned char e8250_get_div_hi (e8250_t *uart);
unsigned char e8250_get_rxd (e8250_t *uart);
unsigned char e8250_get_ier (e8250_t *uart);
unsigned char e8250_get_iir (e8250_t *uart);
unsigned char e8250_get_lcr (e8250_t *uart);
unsigned char e8250_get_mcr (e8250_t *uart);
unsigned char e8250_get_lsr (e8250_t *uart);
unsigned char e8250_get_msr (e8250_t *uart);
unsigned char e8250_get_scratch (e8250_t *uart);

void e8250_set_scratch (e8250_t *uart, unsigned char val);


void e8250_set_uint8 (e8250_t *uart, unsigned long addr, unsigned char val);
void e8250_set_uint16 (e8250_t *uart, unsigned long addr, unsigned short val);
void e8250_set_uint32 (e8250_t *uart, unsigned long addr, unsigned long val);

unsigned char e8250_get_uint8 (e8250_t *uart, unsigned long addr);
unsigned short e8250_get_uint16 (e8250_t *uart, unsigned long addr);
unsigned long e8250_get_uint32 (e8250_t *uart, unsigned long addr);

void e8250_reset (e8250_t *uart);

void e8250_clock (e8250_t *uart, unsigned clk);
/* ---------------------------------------------------------------------------------------------------------------- */
/* IBM Personal System/2 Model 25/30: Quote from "Technical Reference"; Interrupt Sharing:                          */
/* ---------------------------------------------------------------------------------------------------------------- */
/* A standardized hardware design concept has been established to enable multiple adapters to share an interrupt    */
/* level. The integrated adapters do not use interrupt sharing. The following describes this design concept and     */
/* discusses the programming support required.                                                                      */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Design Overview:                                                                                                 */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Most interrupt-supporting adapters hold the IRQ line inactive and then drive the line active to cause an         */
/* interrupt. In contrast, the shared interrupt hardware design allows the IRQ line to float high. Each adapter on  */
/* the line may cause an interrupt by pulsing the line low. The leading edge of the pulse arms the interrupt        */
/* controller; the trailing edge of the pulse causes the interrupt.                                                 */
/*                                                                                                                  */
/* Each adapter sharing an interrupt level must monitor the IRQ line. When any adapter pulses the line, all other   */
/* adapters on that interrupt must not issue an interrupt request until they are rearmed.                           */
/*                                                                                                                  */
/*If an adapter's interrupt is active when it is rearmed, the adapter must reissue the interrupt. This prevents lost*/
/* interrupts in case two adapters issue an interrupt at exactly the same time and an interrupt handler issues a    */
/* Global Rearm after servicing one of them.                                                                        */
/*                                                                                                                  */
/* The following diagram shows the shared interrupt hardware logic.                                                 */
/* ---------------------------------------------------------------------------------------------------------------- */
/*  - See included file in source code directory tree for details:                                                  */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Figure 1-10. Shared Interrupt Hardware Logic:                                                                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Program Support:                                                                                                 */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The interrupt-sharing program support described in the following pro-vides for an orderly means to:              */
/*  - Link a task's interrupt handler to a chain of interrupt handlers                                              */
/*  - Share the interrupt level while the task is active                                                            */
/*  - Unlink the interrupt handler from the chain when the task is deac-tivated                                     */
/* Linking onto the Chain: Each newly activated task replaces the inter-rupt vector in low memory with a pointer to */
/* its own interrupt handler. The old interrupt vector is used as a forward pointer and is stored away at a fixed   */
/* offset from the new task's interrupt handler. This method of linking means the last handler to link is the first */
/* one in the chain.                                                                                                */
/*                                                                                                                  */
/* Sharing the Interrupt Level: When the new task's handler gains control as a result of an interrupt, the handler  */
/* reads the contents of the adapter's Interrupt Status register to determine whether its adapter caused the        */
/* interrupt. If its adapter did cause the interrupt, the handler services the interrupt, disables (clears) the     */
/* interrupts (CLI), and writes to address hex 02FX, where X corresponds to inter-rupt levels 2 through 7. Each     */
/* adapter in the chain decodes the address, which results in a Global Rearm. The handler then issues a nonspecific */
/* End of Interrupt (EOI) and finally issues a Return from Interrupt (IRET). If its adapter did not cause the       */
/* interrupt, the handler passes control to the next interrupt handler in the chain.                                */
/*                                                                                                                  */
/* Unlinking from the Chain: To unlink from the chain, a task must first locate its handler's position within the   */
/* chain. By starting at the inter-rupt vector in low memory and using the offset of each handler's forward pointer */
/* to find the entry point of each handler, the chain can be methodically searched until the task finds its own     */
/* handler. The forward pointer of the previous handler in the chain is replaced by the task's pointer, removing the*/
/* handler from the chain.                                                                                          */
/*                                                                                                                  */
/*NOTE: if the handler cannot locate its position in position in the chain or, if the signature of any prior handler*/
/* is not hex 424B, it must not unlink.                                                                             */
/*                                                                                                                  */
/*Error Recovery: If the unlinking routine discovers that the interrupt chain has been corrupted, an unlinking error*/
/* recovery procedure must be in place. Each application can incorporate its own unlinking error procedure into the */
/*unlinking routine. One application may choose to display an error message requiring the operator to either correct*/
/* the situation or reset the system. The application, however, must not unlinke.                                   */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Precautions:                                                                                                     */
/* ---------------------------------------------------------------------------------------------------------------- */
/* The following precautions must be taken when designing hardware or programs that use shared interrupts.          */
/*                                                                                                                  */
/* Hardware designers should ensure that the adapters:                                                              */
/*  - Do not power up with an interrupt pending or enabled.                                                         */
/*  - Do not generate interrupts that are not serviced by a handler. Generating interrupts when a handler is not    */
/*  active to service them causes that interrupt level to lock up. The design concept relies on the handler to clear*/
/*  its adapter's interrupt and issue the Global Rearm.                                                             */
/*  - Can be disarmed so that they do not remain active after their application has terminated.                     */
/* Programmers should:                                                                                              */
/*  - Ensure that their programs contain a short routine that can be executed with the AUTOEXEC.BAT to disable their*/
/*  adapter's interrupts. This precaution ensures that the adapters are deactivated for a system  reboot that does  */
/*  not clear memory.                                                                                               */
/*  - Treat words as words, not as bytes.                                                                           */
/* NOTE: Remember that data is stored in memory using the Intel format (word hex 424B is stored as hex 4B42).       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Interrupt Chaining Structure:                                                                                    */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ENTRY: JMP       SHORT PAST        ; Jump around structure                                                       */
/*        FPTR      DD      0         ; Forward Pointer                                                             */
/*        SIGNATURE DW      424BH     ; Used when unlinking to identify compatible interrupt handlers               */
/*        FLAGS     DB      0         ; Flags                                                                       */
/*        FIRST     EQU     80H       ; Flags for being first in chain                                              */
/*        JMP       SHORT   RESET                                                                                   */
/*        RES_BYTES DB      DUP 7 (0) ; Future Expansion                                                            */
/* PAST:  ...                         ; Actual start of codee                                                       */
/* The interrupt chaning structure is a 16-byte format containing FPTR, SIGNATURE, RES_BYTES, and a Jump instruction*/
/* to a reset routine. It begins at the third byte from the interrupt handler's entry point. The first instruction  */
/* of every handler is a short jump around the struc-ture to the start of the routine.                              */
/*                                                                                                                  */
/* Except for those residing in adapter ROM, handlers designed for interrupt sharing must use hex 424B as the       */
/* signature to avoid cor-rupting the chain due to misidentification of an interrupt handler. Because each handler's*/
/* chaining structure is known, the forward pointers can be updated when unlinking.                                 */
/*                                                                                                                  */
/* The flag indicates that the handler is first in the chain and is used only with interrupt 7. The Reset routine   */
/* disables the adapter's inter-rupt and then does a Far Return to the operating system.                            */
/* ---------------------------------------------------------------------------------------------------------------- */
/* ROM Considerations:                                                                                              */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Adapters with interrupt handlers residing in ROM must store the forward pointer in latches or ports on the       */
/* adapter. If the adapter is sharing interrupt 7, it must also store a First. Storing this flag is nec-essary      */
/* because its position in the chain may not always be first.                                                       */
/*                                                                                                                  */
/* Because the forward pointer is not stored in the third byte, these han-dlers must contain a signature of hex 00. */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Examples:                                                                                                        */
/* ---------------------------------------------------------------------------------------------------------------- */
/* In the following examples, note that interrupts are disabled before passing control to the next handler on the   */
/* chain. The next handler receives control as if a hardware interrupt had cause it to receive control. Note also   */
/* that the interrupts are disabled before the nonspe-cific EOI is issued, and are not reenabled in the interrupt   */
/* handler. This ensures that the IRET is executed (at which point the flags are restored and the interrupts        */
/* reenabled) before another interrupt is ser-viced. This protectes the stack from excessive buildup.               */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Interrupt Handler Example:                                                                                       */
/* ---------------------------------------------------------------------------------------------------------------- */
/* OUT_CARD  EQU     xxxx              ; Location of our card's interrupt                                           */
/* ISB       EQU     xx                ; Interrupt bit in our cards interrupt control/status register               */
/* REARM     EQU     2F7H              ; Global Rearm location for interrupt 7                                      */
/* SPC_EOI   EQU     67H               ; Specific EOI for interrupt 7                                               */
/* EOI       EQU     20H               ; Nonspecific EOI                                                            */
/* OCR       EQU     20H              ; Location of interrupt controller operational control register               */
/* IMR       EQU     21H               ; Location of interrupt mask register                                        */
/*                                                                                                                  */
/* MYSEG     SEGMENT PARA                                                                                           */
/*           ASSUME  CS:MYSEG,DS:DSEG                                                                               */
/* ENTRY     PROC    FAR                                                                                            */
/*           JMP     SHORT PAST        ; Entry point of handler                                                     */
/* FPTR      DD      0                 ; Forward Pointer                                                            */
/* SIGNATURE DW      424BH             ; Used when unlinking to identify compatible interrupt handlers              */
/*           FLAGS   DB      0         ; Flags                                                                      */
/*           FIRST   EQU     80H                                                                                    */
/*           JMP     SHORT   RESET                                                                                  */
/* RES_BYTES DB      DUP 7(0)          ; Expansion                                                                  */
/* PAST:     STI                       ; Actual start of handler code                                               */
/*           PUSH    ...               ; Save needed registers                                                      */
/*           MOV     DX,OUR_CARD       ; Select our status register                                                 */
/*           IN      AL,DX             ; Read the status register                                                   */
/*           TEST    AL,ISB            ; Our card caused the interrupt?                                             */
/*           JNE     SERVICE           ; Yes, branch to service logic                                               */
/*           TEST    CS:FLAGS,FIRST    ; Are we the first ones in?                                                  */
/*           JNZ     EXIT              ; If yes, branch for EOI and Rearm                                           */
/*           POP     ...               ; Restore registers                                                          */
/*           CLI                       ; Disable interrupts                                                         */
/*           JMP     DWORD PTR CS:FPTR ; Pass control to next handler on chain                                      */
/*                                                                                                                  */
/* SERVICE   ...                       ; Service the interrupt                                                      */
/* EXIT:                                                                                                            */
/*           CLI                       ; Disable the interrupts                                                     */
/*           MOV     AL,EOI                                                                                         */
/*           OUT     OCR,AL            ; Issue nonspecific EOI                                                      */
/*           MOV     DX,REARM          ; Rearm our card                                                             */
/*           OUT     DX,AL                                                                                          */
/*           POP     ...               ; Restore registers                                                          */
/*           IRET                                                                                                   */
/* RESET:    ...                       ; Disable our card                                                           */
/*           RET                       ; Return Far to operating system                                             */
/* ENTRY:    ENDP                                                                                                   */
/*           MYCSEG  ENDS                                                                                           */
/*           END     ENTRY                                                                                          */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Linking Code Example                                                                                             */
/* ---------------------------------------------------------------------------------------------------------------- */
/*           PUSH     ES                                                                                            */
/*           CLI                          ; Disable interrupts                                                      */
/* ; Set forward pointer to the value of the interrupt vector in low memory                                         */
/*           ASSUME   CS:CODESEG,DS:CODESEG                                                                         */
/*           PUSH     ES                                                                                            */
/*           MOV      AX,350FH            ; DOS get interrupt vector                                                */
/*           INT      21H                 ;                                                                         */
/*           MOV      SI,OFFSET CS:FPTR   ; Set offset of our forward pointer in an indexable register              */
/*           MOV      CS:[SI],BX          ; Store the old interrupt vector                                          */
/*           MOV      CS:[SI+2],ES        ;  in our forward pointer                                                 */
/*           CMP      ES:BYTE PTR[BX],CFH ; Test for IRET                                                           */
/*           JNZ      SERVECTR                                                                                      */
/*           MOV      CS:FLAGS,FIRST      ; Set up first in chain flag                                              */
/* SERVECTR: POP      ES                                                                                            */
/*           PUSH     DS                                                                                            */
/* ; Make interrupt vector in low memory point to our handler                                                       */
/*           MOV      DX,OFFSET ENTRY     ; Make interrupt vector point to our interrupt handler                    */
/*           MOV      AX,SEG ENTRY        ; If DS not = CS, get it out and                                          */
/*           MOV      DS,AX               ;  put it in DS                                                           */
/*           MOV      AX,250FH            ; DOS set interrupt vector                                                */
/*           INT      21H                 ;                                                                         */
/*           POP      DS                                                                                            */
/* ; Unmask (enable) interrupt for our level                                                                        */
/* SET7:     IN       AL,IMR              ; Read interrupt mask register                                            */
/*           AND      AL,07FH             ; Unmask interrupt level 7                                                */
/*           OUT      IMR,AL              ; Write new interrupt mask                                                */
/*           MOV      AL,SPC_EOI          ; Issue specific EOI for level 7                                          */
/*           OUT      OCR,AL              ;  to allow pending level 7 interrupts (if any) to be serviced            */
/*           STI                          ; Enable interrupts                                                       */
/*           POP      ES                                                                                            */
/* ---------------------------------------------------------------------------------------------------------------- */
/* Unlinking Code Example                                                                                           */
/* ---------------------------------------------------------------------------------------------------------------- */
/*           PUSH     DS                                                                                            */
/*           PUSH     ES                                                                                            */
/*           CLI                      ; Disable interrupts                                                          */
/*           MOV      AX,350FH        ; DOS get interrupt vector                                                    */
/*           INT      21H             ; ES:BX points to the first in the chain                                      */
/*           MOV      CX,ES           ; Pickup segment part of interrupt vector                                     */
/* ; Are we the first handler in the chain?                                                                         */
/*           MOV      AX,CS           ; Get code seg into comparable register                                       */
/*           CMP      BX,OFFSET ENTRY ; Interrupt vector in low memory pointing to our handlers offset?             */
/*           JNE      UNCHAIN_A       ; No, branch                                                                  */
/*           CMP      AX,CX           ; Vector pointing to our handler's segment?                                   */
/*           JNE      UNCHAIN_A       ; No, branch                                                                  */
/*; Set interrupt vector in low memory to point to the handler pointed to by our pointer                            */
/*           PUSH     DS                                                                                            */
/*           MOV      AX,CS:FPTR                                                                                    */
/*           MOV      DX,WORD PTR CS:FPTR ; Set offset of interrupt vector                                          */
/*           MOV      DS,WORD PTR CS:FPTR[2] ; Set segment of interrupt vector                                      */
/*           MOV      AX,250FH        ; DOS set interrupt vector                                                    */
/*           INT      21H                                                                                           */
/*           POP      DS                                                                                            */
/*           JMP      UNCHAIN_X                                                                                     */
/*UNCHAIN_A: ; CX = FPTR segment, BX = FPTR offset                                                                  */
/*           CMP      ES:[BX+6],4B42H ; Is handler using the appropriate conventions (is SIGNATURE = 424BH?)        */
/*           JNE      exception       ; No, invoke error exception handler                                          */
/*           LDS      SI,ES:[BX+2]    ; Get FPTR's segment and offset                                               */
/*           CMP      SI,OFFSET ENTRY ; Is this forward pointer pointing to our handler's offset?                   */
/*           JNE      UNCHAIN_B       ; No, branch                                                                  */
/*           MOV      CX,DS           ; Is this forward pointer pointing to                                         */
/*           CMP      AX,CX           ;  our handler's segment?                                                     */
/*           JNE      UNCHAIN_B       ; No, branch                                                                  */
/*; Located our handler in the chain                                                                                */
/*           MOV      AX,WORD PTR CS:FPTR ; Get our FPTR's offset                                                   */
/*           MOV      ES:[BX+2],AX    ; Replace FPTR offset pointing to us                                          */
/*           MOV      AX,WORD PTR CS:FPTR[2] ; Get our FPTR's segment                                               */
/*           MOV      ES:[BX+4],AX    ; Replace FPTR segment pointing to us                                         */
/*           MOV      AL,CS:FLAGS                                                                                   */
/*           AND      AL,FIRST                                                                                      */
/*           OR       ES:[BX+6],AX    ; Replace offset of FPTR of handler                                           */
/*           JMP      UNCHAIN_X                                                                                     */
/*UNCHAIN_B: MOV      BX,SI           ; Move new offset to BX                                                       */
/*           PUSH     DS                                                                                            */
/*           PUSH     ES                                                                                            */
/*           JMP      UNCHAIN_A       ; Examine the next handler in the chain                                       */
/*UNCHAIN_X: STI                      ; Enable interrupts                                                           */
/*           POP      ES                                                                                            */
/*           POP      DS                                                                                            */
/* ---------------------------------------------------------------------------------------------------------------- */

#endif
