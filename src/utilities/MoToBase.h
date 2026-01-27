#ifndef MOTOBASE_H
#define MOTOBASE_H
/*
  MobaTools.h - a library for model railroaders
  Author: fpm, fpm@mnet-mail.de
  Copyright (c) 2020 All right reserved.

  Base definitions and declarations for all parts of MobaTools
*/
#include <Arduino.h>
#include <inttypes.h>
#include <limits.h>

//architecture specific includes
#ifdef ARDUINO_ARCH_AVR
	#include <avr/drivers.h>
#elif defined ARDUINO_ARCH_MEGAAVR
	#if defined __AVR_TINY_2__
		#include <megaTinyAVR/drivers.h>
	#else
		#include <megaAVR/drivers.h>
	#endif	
#elif defined ARDUINO_ARCH_STM32F1
	#include <stm32f1/drivers.h>
#elif defined ARDUINO_ARCH_STM32F4
	#include <stm32f4/drivers.h>
#elif defined ARDUINO_ARCH_ESP8266
	#include <esp8266/drivers.h>
#elif CONFIG_IDF_TARGET_ESP32		// ==1 if MCU is ESP32
	#include <esp32/drivers.h>
#elif ARDUINO_ARCH_ESP32 // all other ESP32 MCU's (ESP32Xn)
	#include <esp32S3/drivers.h>
#elif defined ARDUINO_ARCH_RENESAS_UNO 
	#include <ra4m1/drivers.h>
#elif defined ARDUINO_ARCH_RP2040
	#include <rp2040/drivers.h>
#elif defined ARDUINO_ARCH_SAMD 
	#include <samd/drivers.h>
#else
    #error "Processor not supported"
#endif


#if !defined IS_ESP && !defined IS_ESP32S
    #define  IRAM_ATTR  // Attribut IRAM_ATTR entfernen wenn nicht definiert
#else
	#ifdef IRAM_ATTR  //IRAM_ATTR
		// don't use ICACHE_RAM_ATTR in newer versions of ESP8266 core
		#undef  ICACHE_RAM_ATTR
		#define ICACHE_RAM_ATTR IRAM_ATTR
	#endif
#endif
////////////////////////////////////////////////// general defines for all plattforms  //////////////////////////////////////////
// Calling compatible architecture specific functions.
// These functions are defined in architecture specific files, but can be called independend from actual architecture

// type definitions for the Servo class.
#ifndef servoPos_t
	// special definitions ( e.g. for special PWM HW ) are defined in MCU specific drivers.h
	#define servoPos_t uint16_t // default fo rmost boards ( timing with 16-bit timer )
#endif
// type definitions ( if they are different for 8bit and 32bit platforms)
#ifdef IS_32BIT
	#define uintxx_t uint32_t
	#define intxx_t	int32_t
	#define uintx8_t uint32_t
	#define intx8_t	int32_t
    #define nextCycle_t uint32_t	// in CYCLETIME units ( = 1µs on 32Bit MCUs )
	#define MIN_TIC_DIFF TICS_PER_MICROSECOND * 10 // minimal time between 2 interrupts ( in tics )
	#define USE_JITTER
	#define MAX_JITTER MIN_STEP_CYCLE/4
	#ifndef FAST_STEPRESET
	#define FAST_STEPRESET	0	// fast MCU's by default reset the step impulse in the following IRQ
	#endif

#else
	#define uintxx_t	uint16_t
	#define  intxx_t	int16_t
	#define uintx8_t uint8_t
	#define intx8_t	int8_t
    #define nextCycle_t uint8_t	// in CYCLETIME units
	#ifndef FAST_STEPRESET
	#define FAST_STEPRESET	1	// slow MCU's always reset the step impulse in same ISR as they set it
	#endif
    //extern uint8_t nextCycle;
#endif
extern nextCycle_t nextCycle;   // to be used in ISR for stepper and softled

#define ISR_IDLETIME    5000        // max time between two Stepper/Softled ISRs ( µsec )

// internal defines

//#define TIMERPERIODE    20000   // Timer Overflow in µs
//#define TIMER_OVL_TICS  ( TIMERPERIODE*TICS_PER_MICROSECOND )
// Starting with V3.0 the timer ( allways 16-bits - apart from ESP32/RP2040 ) overflows at max (0xFFFF)
constexpr uint16_t TIMER_OVL_TICS = 0xffff;
#define TIMERPERIODE   (TIMER_OVL_TICS / TICS_PER_MICROSECOND) // Timer Overflow in µs

typedef struct {    // portaddress and bitmask for direkt pin set/reset ( only used in AVR )
   volatile uint8_t* Adr;
   volatile uint8_t Mask;
} portBits_t;

// Include class specific .h files
#include <utilities/MoToStepper.h>
#ifndef ARDUINO_ARCH_ESP8266
// synced moves are not possible at ESP8266
#include <utilities/MoToSyncStepper.h>
#endif
#include <utilities/MoToServo.h>
#include <utilities/MoToSoftled.h>
#include <utilities/MoToPwm.h>

#include <utilities/MoToDbg.h>

#ifdef ARCHITECT_INCLUDE		// Defined in MoToBase.h if MCU-specific includes are required.
#include ARCHITECT_INCLUDE
#endif


#endif



