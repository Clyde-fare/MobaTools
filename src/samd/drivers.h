#ifndef SAMD_DRIVER_H
#define SAMD_DRIVER_H
//////////////////////////////////////// processor dependent defines and declarations //////////////////////////////////////////
    //--------------------------------------------------------------------------------------------------------------
//vvvvvvvvvvvvvvvvvvvvvvvvvv SAMD processors vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
#define __SAMD__
#define IS_32BIT
#define IRAM_ATTR       // delete in .cpp files, because it has no meaning for STM32 processors
#define DRAM_ATTR
#define MOTOSOFTLED32		// use 32-bit version of SoftLed class

#define CYCLETIME       1     // Cycle count in µs on 32Bit processors

// At Samd the internal 8MHz clock is used for the timers. Timer-Prescaler is 4 so 0,5µs per tic
#define TICS_PER_MICROSECOND 2  

// MobaTools-Timer at Samd processor
#define TC5    // Timer used by MobaTools
#define STEP_CHN    1       // OCR channel for Stepper and Leds
#define TIMER_STEPCH_IRQ TIMER_CC1_INTERRUPT
#define SERVO_CHN   2       // OCR channel for Servos
#define TIMER_SERVOCH_IRQ TIMER_CC2_INTERRUPT
#define GET_COUNT timer_get_count(MT_TIMER)

extern bool timerInitialized;
void seizeTimer1();
#define USE_SPI2          // Use SPI1 if not defined
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SAMD ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define ARCHITECT_INCLUDE <samd/MoToSAMD.h>
#endif
