/*****************************************************************************
 * pce                                                                       *
 *****************************************************************************/

/*****************************************************************************
 * File name:   src/arch/ibmpc/speaker.h                                     *
 * Created:     2010-02-24 by Hampa Hug <hampa@hampa.ch>                     *
 * Copyright:   (C) 2010 Hampa Hug <hampa@hampa.ch>                          *
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


#ifndef PCE_IBMPC_IBMPCPSU_H
#define PCE_IBMPC_IBMPCPSU_H 1


/* #include <drivers/sound/sound.h> */


typedef struct {
	unsigned long  psu1_5150;
	unsigned long  psu2_5150;
	unsigned long  psu3_5150;
	unsigned long  psu4_5150;
	unsigned long  psu5_5150;
	unsigned long  psu6_5150;
	unsigned long  psu7_5150;
	unsigned long  psu8_5150;
	/* ------------------------------------------------------------------------------------------ */
	/*Quote from IBM PC Technical Reference (Model 5150) Section 3. Power Supply:                 */
	/* ------------------------------------------------------------------------------------------ */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The system power supply is located at the right rear of the system unit. It is an integral */
	/* part of the system-unit chassis. Its housing provides support for the rear panel, and its  */
	/* fan furnishes cooling for the whole system.                                                */
	/*                                                                                            */
	/* It supplies the power and reset signal necessary for the operation of the system board,    */
	/* installed options, and the keyboard. It also provides a switched ac socket for the IBM     */
	/* Monochrome Display and two separate connectors for power to the 5-1/4 inch diskette drives.*/
	/*                                                                                            */
	/* The two different power supplies available are designed for continuous operation at 63.5   */
	/* Watts. They have a fused 120 Vac or 220/240 Vac input and provide four regulated dc output */
	/* voltages: 7 A at +5 Vdc, 2 A at +12 Vdc, 0.3 A at -5 Vdc, and 0.25 A at -12 Vdc. These     */
	/* outputs are overvoltage, overcurrent, open-circuit, and short-circuit protected. If a dc   */
	/* overload or overvoltage condition occurs, all dc outputs are shut down as long as the      */
	/* condition exists.                                                                          */
	/*                                                                                            */
	/* The +12 Vdc and -12 Vdc power the EIA drivers the EIA drivers and receivers on the         */
	/* asynchronous communications adapter. The +12 Vdc also power's the system's dynamic memory  */
	/* and the two internal 5-1/4 inch diskette drive motors. It is assumed that only one drive is*/
	/* active at a time. The +5 Vdc powers the logic on the system board and diskette drives and  */
	/* allows about 4 A of +5 Vdc for the adapters in the system-unit expansion slots. The -5 Vdc */
	/* is for dynamic memory bias voltage; it tracks the +5 Vdc and +12 Vdc very quickly at power-*/
	/* on and has a longer decay on power-off than the +5 Vdc and +12 Vdc outputs. All four power */
	/* supply dc voltages are bussed across each of the five system-unit expansion slots.         */
	/* ------------------------------------------------------------------------------------------ */
	/* Input Requirements:                                                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* The following are the input requirements for the system unit power supply.                 */
	/*                  ----- Voltage (Vac) -----: Frequency (Hz):      Curent (Amps)  :          */
	/*                  Nominal   : Minimum: Maximum:     +/- 3Hz          Maximum:               */
	/*                   - 120       - 104    - 127         60         - 2.5 at 104 Vac           */
	/*                   - 220/240   - 180    - 259         50         - 1.0 at 180 Vac           */
	/* ------------------------------------------------------------------------------------------ */
	/* Outputs:                                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Vdc Output:                                                                                */
	/* ------------------------------------------------------------------------------------------ */
	/* The following are the dc outputs for the system unit power supply.                         */
	/* ---------------- Voltage (Vdc): Current  (Amps) : Regulation (Tolerance): ---------------- */
	/* -------------------- Nominal :   Minimum: Maximum:      +%:       -%:--------------------- */
	/* --------------------- -  +5.0     - 2.3    - 7.0      -  5      -  4 --------------------- */
	/* --------------------- -  -5.0     - 0.0    - 0.4      - 10      -  8 --------------------- */
	/* --------------------- - +12.0     - 0.4    - 2.0      -  5      -  4 --------------------- */
	/* --------------------- - -12.0     - 0.0    - 0.25     - 10      -  9 --------------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* Vac Output:                                                                                */
	/* ------------------------------------------------------------------------------------------ */
	/* The power supply provides a filtered, fused, ac output that is switched on and off with the*/
	/* main power switch. The maximum current available at this output is 0.75 A. The receptacle  */
	/* provided at the rear of the power supply for this ac output is a nonstandard connector     */
	/* designed to be used only for the IBM Monochrome Display.                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Overvoltage/Overcurrent Protection:                                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* The system power supply employs the protection features which are described below.         */
	/* ------------------------------------------------------------------------------------------ */
	/* Primary (Input):                                                                           */
	/* ------------------------------------------------------------------------------------------ */
	/* The following table describes the primary (input voltage) protection for the system-unit   */
	/* power supply.                                                                              */
	/* ------------------------------------------------------------------------------------------ */
	/* ----------------- Voltage (Nominal Vac): Type Protection: Rating (Amps): ----------------- */
	/* -----------------     -   120              - Fuse             - 2        ----------------- */
	/* -----------------     - 220/240            - Fuse             - 1        ----------------- */
	/* ------------------------------------------------------------------------------------------ */
	/* Secondary (Output):                                                                        */
	/* ------------------------------------------------------------------------------------------ */
	/*On overvoltage, the power supply is designed to shut down all outputs when either the +5 Vdc*/
	/* or the +12 Vdc output exceeds 200% of its maximum rated voltage. On overcurrent, the supply*/
	/* will turn off if any output exceeds 130% of its nominal value.                             */
	/* ------------------------------------------------------------------------------------------ */
	/* Power Good Signal:                                                                         */
	/* ------------------------------------------------------------------------------------------ */
	/* When the power supply is turned on after it has been off for a minimum of 5 seconds, it    */
	/* generates a 'power good' signal that indicates there is adequate power for processing. When*/
	/* the four output voltages are above the minimum sense levels, as described below, the signal*/
	/* sequences to a TTL-compatible up level (2.4 Vdc to 5.5 Vdc), is capable of sourcing 60 uA. */
	/* When any of the four output voltages is below its minimum sense level or above its maximum */
	/* sense level, the 'power good' signal will be TTL-compatible down level (0.0 Vdc to 0.4 Vdc)*/
	/*capable of supplying 500 uA. The 'power good' signal has a turn-on delay of 100-ms after the*/
	/* output voltages have reached their respective minimum sense levels.                        */
	/* ------------------------------------------------------------------------------------------ */
	/* ---Output Voltage: Under-Voltage Nominal Sense Level: Over-Voltage Nominal Sense Level:--- */
	/* --- -  +5 Vdc                     -  +4.0 Vdc                       -  +5.9 Vdc        --- */
	/* --- -  -5 Vdc                     -  -4.0 Vdc                       -  -5.9 Vdc        --- */
	/* --- - +12 Vdc                     -  +9.6 Vdc                       - +14.2 Vdc        --- */
	/* --- - -12 Vdc                     -  -9.6 Vdc                       - -14.2 Vdc        --- */
	/* ------------------------------------------------------------------------------------------ */
	/* Power Supply Connectors and Pin Assignments:                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The power connector on the system board is a 12-pin male connector that plugs into the     */
	/* power-supply connectors. The pin configuration and locations follow.                       */
	/* ------------------------------------------------------------------------------------------ */
	/* Power Supply and Connectors:                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* Power On/Off: IBM Monochrome Display Power Connector (Internally Switched): System Unit    */
	/* 5-1/4 Inch Diskette Drive Power Connectors:                               Power Connector: */
	/*  - Pin 4: + 5 Vdc                                                                          */
	/*  - Pin 3: Ground                                                                           */
	/*  - Pin 2: Ground                                                                           */
	/*  - Pin 1: +12 Vdc                                                                          */
	/* System Board Power Connectors:                              System Board Power Connectors: */
	/*  - Pin 6: + 5 Vdc                                            - Pin 6: Ground               */
	/*  - Pin 5: + 5 Vdc                                            - Pin 5: Ground               */
	/*  - Key  :                                                                                  */
	/*  - Pin 4: + 5 Vdc                                            - Pin 4: -12 Vdc              */
	/*  - Pin 3: - 5 Vdc                                            - Pin 3: +12 Vdc              */
	/*  - Pin 2: Ground                                             - Pin 2: Key                  */
	/*  - Pin 1: Ground                                             - Pin 1: Pwr Good             */
	/*  - Key  :                                                                                  */
	/* ------------------------------------------------------------------------------------------ */	
	unsigned long  psu1_5160;
	unsigned long  psu2_5160;
	unsigned long  psu3_5160;
	unsigned long  psu4_5160;
	unsigned long  psu5_5160;
	unsigned long  psu6_5160;
	unsigned long  psu7_5160;
	unsigned long  psu8_5160;
	/* ------------------------------------------------------------------------------------------ */
	/*Quote from IBM PC Technocal Reference (Model 5160) Section 3. Power Supply: IBM Personal    */
	/* Computer XT Power Supply:----------------------------------------------------------------- */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The system dc power supply is a 130-watt, 4 voltage-level switching regulator. It is       */
	/*integrated into the system unit and supplies power for the system unit, its options, and the*/
	/* keyboard. The supply provides 15 A of +5 Vdc, plus or minus 5%, 4.2 A of +12 Vdc, plus or  */
	/*minus 5%, 300 mA of -5 Vdc, plus or minus 10%, and 250 mA of -12 Vdc, plus or minus 10%. All*/
	/* power levels are regulated with overvoltage and overcurrent protection. There are two power*/
	/* supplies, 120 Vac and 220/240 Vac. Both are fused. If dc overcurrent or overvoltage        */
	/* conditions exist, the supply automatically shuts down until the condition is corrected. The*/
	/* supply is designed for continuous operation at 130 watts.                                  */
	/*                                                                                            */
	/*The system board takes approximately 2 to 4 A of +5 Vdc, thus allowing approximately 11 A of*/
	/* +5 Vdc for the adapters in the system expansion slots. The +12 Vdc power level is designed */
	/* to power the internal 5-1/4 inch diskette drive and the 10M fixed disk drive. The -5 Vdc   */
	/*level is used for analog circuits in the diskette adapter's phase-lock loop. The +12 Vdc and*/
	/* -12 Vdc are used for powering the Electronics Industries Association (EIA) drivers for the */
	/* communications adapters. All four power levels are bussed across the eight system expansion*/
	/* slots.                                                                                     */
	/*                                                                                            */
	/* The IBM Monochrome Display has its own power supply, receiving its ac power from the system*/
	/* unit's power system. The ac output for the display is switched on and off with the Power   */
	/* switch and is a nonstandard connector, so only the IBM Monochrome Display can be connected.*/
	/* ------------------------------------------------------------------------------------------ */
	/* Input Requirements:                                                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* The nominal power requirements and output voltages are listed in the following tables.     */
	/* Input Requirements:                                                                        */
	/* Voltage @ 50/60 Hz:                                                                        */
	/* Nominal Vac: Minimum Vac: Maximum Vac:                                                     */
	/*  - 110:       -  90:       - 137:                                                          */
	/*  - 220/240:   - 180:       - 259:                                                          */
	/* Frequency: 50/60 Hz += 3 Hz                                                                */
	/* Current:   4.1 A max at 90 Vac                                                             */
	/* ------------------------------------------------------------------------------------------ */
	/* Outputs:                                                                                   */
	/* ------------------------------------------------------------------------------------------ */
	/* Vdc Output:                                                                                */
	/* Voltage (Vdc): Current (Amps):    Regulation (Tolerance):                                  */
	/* Nominal:       Minimum: Maximum : + %  : - %  :                                            */
	/*  - + 5.0:       - 2.3:   - 15.0 :  -  5:  -  4:                                            */
	/*  - - 5.0:       - 0.0:   -  0.3 :  - 10:  -  8:                                            */
	/*  - +12.0:       - 0.4:   -  4.2 :  -  5:  -  4:                                            */
	/*  - -12.0:       - 0.0:   -  0.25:  - 10:  -  9:                                            */
	/* Vac Output:                                                                                */
	/* The sense levels of the dc outputs are:                                                    */
	/* Output (Vdc): Minimum (Vdc): Sense Voltage Nominal (Vdc): Maximum (Vdc):                   */
	/*  - + 5:        - + 4.5:       - + 5.0:                     - + 5.5:                        */
	/*  - - 5:        - - 4.3:       - - 5.0:                     - - 5.5:                        */
	/*  - +12:        - +10.8:       - +12.0:                     - +13.2:                        */
	/*  - -12:        - -10.2:       - -12.0:                     - -13.2:                        */
	/* ------------------------------------------------------------------------------------------ */
	/* Overvoltage/Overcurrent Protection:                                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* Voltage and Current Protection:                                                            */
	/* Nominal Voltage (Vac): Type Protection: Rating (A):                                        */
	/*  - 100-125:             - Fused:         - 5   A                                           */
	/*  - 200-240:             - Fused:         - 2.5 A                                           */
	/* ------------------------------------------------------------------------------------------ */
	/* Power Good:                                                                                */
	/* ------------------------------------------------------------------------------------------ */
	/* When the power supply is switched off for a minimum of 1 second and then switched on, the  */
	/* 'power good' signal is regenerated.                                                        */
	/*                                                                                            */
	/* This signal is the logical AND of the dc output-voltage sense signal and the ac input-     */
	/* voltage fail signal. This signal is TTL-compatible up-level for normal operation or down-  */
	/* level for fault conditions. The ac fail signal causes 'power good' to go to a down-level   */
	/* when any output voltage falls below the sense voltage limits.                              */
	/*                                                                                            */
	/* When power is switched on, the dc output-voltage sense signal holds the 'power good' signal*/
	/*at a down level until all output voltages reach their minimum sense levels. The 'power good'*/
	/* signal has a turn-on delay of 100 to 500 milliseconds.                                     */
	/* ------------------------------------------------------------------------------------------ */
	/* Connector Specifications and Pin Assignments:                                              */
	/* ------------------------------------------------------------------------------------------ */
	/* The power connector on the system board is a 12-pin connector that plugs into the power    */
	/* supply connectors, P8 and P9. The Input Voltage Selector switch and the pin assignment     */
	/* locations follow.                                                                          */
	/* Power Supply and Connectors:                                                               */
	/* Power Supply Fan Connector (P13):                                                          */
	/*  - Pin 1: -12 Vdc                                                                          */
	/*  - Pin 2: Ground                                                                           */
	/* IBM Display Connector (P12):                                                               */
	/*  - Pin 1: +12 Vdc                                                                          */
	/*  - Pin 2: Ground                                                                           */
	/* 5-1/4 Inch Diskette Drive Power Connectors (P10/P11):                                      */
	/*  - Pin 1: +12 Vdc                                                                          */
	/*  - Pin 2: Ground                                                                           */
	/*  - Pin 3: Ground                                                                           */
	/*  - Pin 4: +5 Vdc                                                                           */
	/* System Board Power Connectors (P8): System Board Power Connectors (P9):                    */
	/*  - Pin 1. Power Good                 - Pin 1. Ground                                       */
	/*  - Pin 2. Key                        - Pin 2. Ground                                       */
	/*  - Pin 3. +12 Vdc                    - Pin 3. -5 Vdc                                       */
	/*  - Pin 4. -12 Vdc                    - Pin 4. +5 Vdc                                       */
	/*  - Pin 5. Ground                     - Pin 5. +5 Vdc                                       */
	/*  - Pin 6. Ground                     - Pin 6. +5 Vdc                                       */
	/* Voltage Selector Switch: ON/OFF Switch: Power Supply Fan:                                  */
	/* ------------------------------------------------------------------------------------------ */
	/* Overvoltage/Overcurrent Protection:                                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* Voltage Nominal Vac: Type Protection: Rating Amps:                                         */
	/*  - 110:               - Fuse:          - 5.0:                                              */
	/*  - 220/240:           - Fuse:          - 3.5:                                              */
	/* ------------------------------------------------------------------------------------------ */
	/* Power-Good:                                                                                */
	/* ------------------------------------------------------------------------------------------ */
	/* When the supply is switched off for a minimum of 1.0 second, and then switched on, the     */
	/* 'power good' signal will be regenerated.                                                   */
	/*                                                                                            */
	/* The 'power good' signal indicates that there is adequate power to continue processing. If  */
	/* the power goes below the specified levels, the 'power good' signal triggers a system       */
	/* shutdown.                                                                                  */
	/*                                                                                            */
	/* This signal is the logical AND of the dc output-voltage 'sense' signal and the ac input-   */
	/* voltage 'fail' signal. This signal is TTL-compatible up-level for normal operation or down-*/
	/* level for fault conditions. The ac 'fail' signal causes 'power good' to go to a down level */
	/* when any output voltage falls below the regulation limits.                                 */
	/*                                                                                            */
	/* The dc output-voltage 'sense' signal holds the 'power good' signal at a down level (during */
	/* power-on) until all output voltages have reached their respective minimum sense levels. The*/
	/* 'power good' signal has a turn-on delay of at least 100-ms but no greater than 500-ms.     */
	/* ------------------------------------------------------------------------------------------ */
	/* Connector Specifications and Pin Assignments:                                              */
	/* ------------------------------------------------------------------------------------------ */
	/*The power connector on the system board is a 12-pin male connector that plugs into the power*/
	/* -supply connectors. The pin assignments and locations are shown on the following page.     */
	/*                                                                                            */
	/* Power Supply and Connectors:                                                               */
	/* 5-1/4 Inch Diskette Drive Power Connector:                                                 */
	/*  - Pin 1: +12 Vdc                                                                          */
	/*  - Pin 2: Ground                                                                           */
	/*  - Pin 3: Ground                                                                           */
	/*  - Pin 4: + 5 Vdc                                                                          */
	/* Fixed      Disk     Drive Power Connector:                                                 */
	/*  - Pin 1: +12 Vdc                                                                          */
	/*  - Pin 2: Ground                                                                           */
	/*  - Pin 3: Ground                                                                           */
	/*  - Pin 4: + 5 Vdc                                                                          */
	/* System Board Power Connectors: System Board Power Connectors:                              */
	/*  - Pin 1: Pwr Good              - Pin 1: Ground                                            */
	/*  - Pin 2: Key                   - Pin 2: Ground                                            */
	/*  - Pin 3: +12 Vdc               - Pin 3: -5 Vdc                                            */
	/*  - Pin 4: -12 Vdc               - Pin 4: +5 Vdc                                            */
	/*  - Pin 5: Ground                - Pin 5: +5 Vdc                                            */
	/*  - Pin 6: Ground                - Pin 6: +5 Vdc                                            */
	/* Power On/Off: System Unit Power Connector: IBM Monochrome Display Power Connector:         */
	/* ------------------------------------------------------------------------------------------ */
	unsigned long  psu1_5155;
	unsigned long  psu2_5155;
	unsigned long  psu3_5155;
	unsigned long  psu4_5155;
	unsigned long  psu5_5155;
	unsigned long  psu6_5155;
	unsigned long  psu7_5155;
	unsigned long  psu8_5155;
	/* ------------------------------------------------------------------------------------------ */
	/*Quote from IBM PC Technical Reference (Model 5155) Section 3. IBM Portable Personal Computer*/
	/* Power Supply:----------------------------------------------------------------------------- */
	/* Description:                                                                               */
	/* ------------------------------------------------------------------------------------------ */
	/* The system unit's power supply is a 114-watt, switching regulator that provides five       */
	/* outputs. It supplies power for the system unit and its options, the power supply fan, the  */
	/* diskette drive, the composite display, and the keyboard. All power levels are protected    */
	/* against overvoltage and overcurrent conditions. The input voltage selector switch has 115  */
	/* Vac and 230 Vac positions. If a dc overload or overvoltage condition exists, the power     */
	/* supply automatically shuts down until the condition is corrected, and the power supply is  */
	/* switched off and then on.                                                                  */
	/*                                                                                            */
	/* The internal 5-1/4 inch diskette drive uses the +5 Vdc and the +12 Vdc power levels. Both  */
	/* the +12 Vdc and -12 Vdc power levels are used in the drivers and receivers of the optional */
	/* communications adapters. The display uses a separate +12 Vdc power level. The +5 Vdc, -5   */
	/* Vdc, +12 Vdc, and -12 Vdc power levels are bussed across the system expansion slots.       */
	/* ------------------------------------------------------------------------------------------ */
	/* Input Requirements:                                                                        */
	/* ------------------------------------------------------------------------------------------ */
	/* Nominal Voltage (Vac): Minimum Voltage (Vac): Maximum Voltage (Vac):                       */
	/*  - 100-125:             -  90:                 - 137:                                      */
	/*  - 200-240:             - 180:                 - 259:                                      */
	/* Note: Input voltage to be 50 or 60 hertz, += 3 hertz.                                      */
	/* ------------------------------------------------------------------------------------------ */
	/* Amperage Output:                                                                           */
	/* ------------------------------------------------------------------------------------------ */
	/* Amperage Output:                                                                           */
	/* Nominal Voltage (Vdc): Minimum Current (A): Maximum Current (A):                           */
	/*  - + 5:                 - 2.3 :              - 11.2 :                                      */
	/*  - - 5:                 - 0.0 :              -  0.3 :                                      */
	/*  - -12:                 - 0.0 :              -  0.25:                                      */
	/*  - +12:                 - 0.04:              -  2.9 :                                      */
	/*  - +12 (display):       - 0.5 :              -  1.5 :                                      */
	/* Note: Maximum current is 3.5 amperes at 90 Vac.                                            */
	/* ------------------------------------------------------------------------------------------ */
	/* Vdc Sense Voltage Output:                                                                  */
	/* ------------------------------------------------------------------------------------------ */
	/* Vdc Sense Voltage Tolerance:                                                               */
	/* Nominal Voltage (Vdc): Minimum Sense Voltage (Vdc): Maximum Sense Voltage (Vdc):           */
	/*  - + 5:                 - + 4.5:                     - + 6.5:                              */
	/*  - - 5:                 - - 4.3:                     - - 6.5:                              */
	/*  - -12:                 - -10.2:                     - -15.6:                              */
	/*  - +12:                 - +10.8:                     - +15.6:                              */
	/*  - +12 (display):       - +10.8:                     - +15.6:                              */
	/* ------------------------------------------------------------------------------------------ */
} pc_ibmpcpsu_t;


/* void pc_speaker_init (pc_speaker_t *spk);
void pc_speaker_free (pc_speaker_t *cas);

pc_speaker_t *pc_speaker_new (void);
void pc_speaker_del (pc_speaker_t *spk);

void pc_speaker_set_clk_fct (pc_speaker_t *spk, void *ext, void *fct);

int pc_speaker_set_driver (pc_speaker_t *spk, const char *driver, unsigned long srate);

void pc_speaker_set_lowpass (pc_speaker_t *spk, unsigned long freq);

void pc_speaker_set_volume (pc_speaker_t *spk, unsigned vol);

void pc_speaker_set_msk (pc_speaker_t *spk, unsigned char val);
void pc_speaker_set_out (pc_speaker_t *spk, unsigned char val);

void pc_speaker_clock (pc_speaker_t *spk, unsigned long cnt); */


#endif
