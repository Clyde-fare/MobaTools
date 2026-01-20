#ifndef SAMD_DRIVER_H
#define SAMD_DRIVER_H
//////////////////////////////////////// processor dependent defines and declarations //////////////////////////////////////////
    //--------------------------------------------------------------------------------------------------------------
//vvvvvvvvvvvvvvvvvvvvvvvvvv SAMD processors vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
#define __SAMD__
#define IS_32BIT
#define IRAM_ATTR       // delete in .cpp files, because it has no meaning for STM32 processors
#define DRAM_ATTR
#define MOTOSOFTLED32    // use 32-bit version of SoftLed class

#define CYCLETIME       1     // Cycle count is µs on 32Bit processors
#ifndef MIN_STEP_CYCLE
	#define MIN_STEP_CYCLE  50       // Minimum number of µsec  per Step
#endif
	#define FAST_STEPRESET	1		// reset the step impulseat End of IRQ ( pulse may be short!!)
	#define IRQ_PRIO 3				// NVIC priority. There are only 4 different level (0...3)
									// Lower priority = higher value
									// with WiFi active
#ifndef MT_TIMER
	#define MT_TIMER 5			// Timer used by MobaTools ( 3,4,5 selecting TC3, TC4 or TC5  is possible)
								// TC5 is default if the number is invalid or not set
								// Be aware that always two timer ( TCC2/TC3 and TC4/TC5 ) share a common clock source
#endif
#ifndef PIN_SPI_SS
#define PIN_SPI_SS 10		// Selct pin for SPI - if not defined in core files
#endif

// At Samd the internal 8MHz clock is used for the timers. Timer-Prescaler is 4 so 0,5µs per tic
#define TICS_PER_MICROSECOND 2  

// Selecting Timer for MobaTools 
// There is only one IRQ per timer. Which event created the interrupt must checked within ISR
#if ( MT_TIMER == 3 )
  #define TCx TC3
  #define TCx_Handler TC3_Handler   // NVIC IRQ-Entrypoint of timer
  #define TCx_IRQn   TC3_IRQn  // NVIC IRQ number  
  #define GCM_MT_ID GCM_TCC2_TC3    // ID of used timer for GCLK
#elif  ( MT_TIMER == 4 )
  #define TCx TC4
  #define TCx_Handler TC4_Handler
  #define TCx_IRQn    TC4_IRQn  // NVIC IRQ number  
  #define GCM_MT_ID GCM_TC4_TC5    // ID of used timer for GCLK
#else // TC5 is default
  #undef MT_TIMER
  #define MT_TIMER 5
  #define TCx TC5
  #define TCx_Handler TC5_Handler
  #define TCx_IRQn  TC5_IRQn  // NVIC IRQ number  
  #define GCM_MT_ID GCM_TC4_TC5    // ID of used timer for GCLK
#endif

// Flag to decide which compare created the IRQ
// This is a bit in TC->INTFLAG or TC->INTFLAG
#define STEP_INT_MSK    TC_INTFLAG_MC0       // channel for Stepper and Leds ( mask in int-register )
#define SERVO_INT_MSK   TC_INTFLAG_MC1       // CC1 channel for Servos
#define GET_COUNT TCx->COUNT16.COUNT.reg)
extern bool timerInitialized;
void seizeTimer1();
#define USE_SPI2          // Use SPI1 if not defined
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SAMD ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define ARCHITECT_INCLUDE <samd/MoToSAMD.h>
#endif