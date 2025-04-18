#ifndef MEGATINYAVR_DRIVER_H
#define MEGATINYAVR_DRIVER_H
//////////////////////////////////////// processor dependent defines and declarations //////////////////////////////////////////
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvv  megaTinyAVR ATTiny 2 Series (3226 / 3227) vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
/* MobaTools uses Timer/Counter 0 (TCA0) ATTiny 2 Series. MegaTinyCore allows the prescaler of TCA0 to be independent from TCB1, 
   used for millis() (set at /2), so we can use a prescaler of 8 = 0.5µs per tic.
*/

//#include <avr/interrupt.h>
#define IRAM_ATTR       // delete in .cpp files, because it has no meaning for megaAVR processors
#define DRAM_ATTR


#define FAST_PORTWRT        // if this is defined, ports are written directly in IRQ-Routines,
                            // not with 'digitalWrite' functions
#if defined(MEGATINYCORE) && defined(MILLIS_USE_TIMERB1)
// For ATtiny with megaTinyCore - can use prescaler of 8 = 0.5µs per tic
#define TICS_PER_MICROSECOND (F_CPU / 1000000.0 / 8) // the clockCyclesPerMicrosecond() call inhibits compiletime evaluation
#if F_CPU != 16000000
    #error "Only 16MHz is supported. Other frequcies are not tested."
#endif
#else
    #error "Not supported"
#endif

// define timer to use
// defines specially für ATTiny 2 Series.
// Timer TCA0 is used, prescaler fixed at 8
// CMP0: for servos
// CMP1: for stepper and softleds
// CMP2: possible own ISR for softleds? ( not yet implemented )

#define OCRxA   TCA0.SINGLE.CMP0
#define OCRxB   TCA0.SINGLE.CMP1
#define GET_COUNT   TCA0.SINGLE.CNT
#define TIMERx_COMPA_vect   TCA0.CMP0.vect
#define TIMERx_COMPB_vect   TCA0.CMP1.vect
#define TIMERx_COMPC_vect   TCA0.CMP2.vect
#define TIMSKx     TCA0.SINGLE.INTCTRL
#define OCIExB

#define ARCHITECT_INCLUDE <megaTinyAVR/MoToMegaTinyAVR.h>
#endif
