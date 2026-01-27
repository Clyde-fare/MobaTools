#ifndef MOBATOOLS_H
#define MOBATOOLS_H
/*
  MobaTools.h - a library for model makers - and others too ;-) 
  Author: Franz-Peter Müller, f-pm+gh@mailbox.org
  Copyright (c) 2025 All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  MobaTools V3.0.0
   
  History:
  V3.0 x-2026
  - Old method names (compatibility to V1 ) are no longer available
  - Synced stepper movement with acceleration
  - support of Samd21 MCU
  - support of Arduino Nano ESP32
  - support of LGT8F328 with 32MHz
  - stepper pins can be inverted
  V2.7.1 x-2025
	- RP204 processers: priority of timer IRQ can be set in MobaTools.h
  V2.7.0 3-2025
    - New class for synced move of steppers ( without acceleration )
	- Example for synced move
	- New example: servos for turnouts
	- MoToTimer: new ( optional) parameter for setting the initial time
	- Support for rp2040/rp2350 processor ( raspberry pi pico )
	- The enable function for steppers can be dynamically switched on/offseveral bugfixes
  V2.6.2 9-2024
    - ESP32 core version 3.x is supported ( V2.x is still supported too )
	- fixed endless rotating when setting moveTo very quickly. (issue#34 on github) 
  V2.6.1 12-2023
    - bugfix with UNO R4Wifi and steppers (with Wifi active)
    - 2 more examples ( button matrix and UNO R4 Wifi and stepper )
  V2.6.0 12-2023
    - Support UNO R4 (Minima and WiFi)
	- MoToStepper.read allows reading the angle in fractions
	- internal optimizations
  For older history see MobaToolsRelNotes.txt
   
*/
// defines that may be changed by the user
    
// CYCLETIME or MIN_STEP_CYCLE define the maximum step rate for steppers ( which also depends on the overall load )
// default values are processordependent, change only if you know what you are doing ).

////////////////////////// Atmel AVR 386P(B), 2560, LGT8F328 (UNO/Nano R3, Mega ...)  ///////////////////////////////
#if defined ARDUINO_ARCH_AVR 
	//#define NO_TIMER3             // never use Timer 3
	//#define CYCLETIME       200   // Min. irq-periode in us ( default is 200 or 100 for LGT8Fx with 32MHz), 
	#define FASTSPI                 // only for devices with USI Interface ( instead of SPI HW )
									// if defined SPI clock ist CPU clock / 2
									// if not defined, SPI clock ist CPU clock / 4
	//#define USI_SS  7               // defines the SS - Pin with USI-SPI-Stepper
									// if not defined the core-default (SS) is used
									
///////////////////// ATMega 4809 ( Nano Every, UNO R2 Wifi )  ///////////////////////////////////
#elif defined ARDUINO_ARCH_MEGAAVR 
	//#define CYCLETIME       200 	// Min. irq-periode in us ( default is 200 ), 
	//#define MoToSS SS				// SPI-SS standard is different regarding the board ;

//////////////////// UNO R4 Minima/Wifi, Nano R4  ///////////////////////////////////
#elif defined ARDUINO_ARCH_RENESAS_UNO 
	//#define MIN_STEP_CYCLE  50       // Minimum number of µsec  per Step ( 50 = default )

/////////////////////SamD MCU, MKR-Series, Nano 33 IoT ...   ///////////////////////////////
#elif defined ARDUINO_ARCH_SAMD 
	//#define SPI_SS_PIN 11		// to overwrite the default
	//define MIN_STEP_CYCLE  50       // Minimum number of µsec  per Step ( 50=default )
	//#define MT_TIMER 5			// Timer used by MobaTools ( 3,4,5 selecting TC3, TC4 or TC5  is possible)
								// TC5 is default if the number is invalid or not set
								// Be aware that always two timer ( TCC2/TC3 and TC4/TC5 ) share a common clock source

////////////////////////// pi Pico, Nano RP2040 connect ..  /////////////////////////////////
#elif defined ARDUINO_ARCH_RP2040 && !defined ARDUINO_ARCH_MBED  
	//#define MIN_STEP_CYCLE 20       // Minimum number of µsec  per Step
	//#define USE_SPI1				// if SPI 1 for SPI-Stepper should be used ( not possible on nano RP2040 )
	//#define STP_TIMR_NBR 1        // use Timer 1 - only possible at RP2350 ( Pico 2 )

/////////////////////// ESP32, ESP32S3 (Nano ESP32 ) and other derivatives (S2,C3,C6) ... //////////////////
#elif defined ARDUINO_ARCH_ESP32 
	//#define USE_VSPI              // default is HSPI ( for SPI-Stepper )
									// on Nano ESP32 it is always FSPI
	//#define MIN_STEP_CYCLE 20     // Minimum number of µsec  per Step
		
////////////////////////// ESP8266  //////////////////////////////////////////////////////
#elif defined  ARDUINO_ARCH_ESP8266 
	//define CYCLETIME       60     // Min. irq-periode in us ( ESP-default is 60 )
									// = high time of Steppulse

////////////////////////// STM32F1  //////////////////////////////////////////////////////
#elif defined ARDUINO_ARCH_STM32F1 
	//#define MIN_STEP_CYCLE  25   	// Minimum number of µsec  per step, default is 25
	//#define USE_SPI2				// default is using SPI1

////////////////////////// STM32F4 (experimental )  //////////////////////////////////////
#elif defined ARDUINO_ARCH_STM32F4 
	#define MIN_STEP_CYCLE  25   // Minimum number of µsec  per step 

/////////////////////////////// not supported //////////////////////////////
#else
    #error Processor not supported
#endif 
//////////////////////////////////////////////////////////////////////////////////

// stepper related defines
#define MAX_STEPPER     6       // 
#define DEF_SPEEDSTEPS  3000    // default speed after attach
#define DEF_RAMP        0       // default ramp after attach 
#define RAMPOFFSET      16      // startvalue of rampcounter

// servo related defines
#if defined ARDUINO_ARCH_ESP32 || defined ARDUINO_ARCH_ESP8266 || defined ARDUINO_ARCH_RP2040
#define MINPULSEWIDTH   550U     // there is no general limit on ESP / RP2040
#define MAXPULSEWIDTH   2600U    // there is no general limit on ESP / RP2040
#else // all other 
#define MINPULSEWIDTH   600U      // don't make it shorter than 550 µs  (lower values mean less time between 
								  // servo-interrupts, even if you don't really use that min pulse length)
#define MAXPULSEWIDTH   2600U     // don't make it longer than 3000 µs. Higher values may lead to more than 20ms
								  // between pulses
#endif

// softled related defines
#define LED_DEFAULT_RISETIME   50

//  !!!!!!!!!!!!  Don't change anything after tis line !!!!!!!!!!!!!!!!!!!!
 
#include <utilities/MoToBase.h>

#include <MoToButtons.h>
#include <MoToTimer.h>

#if 0
#define XSTR(x) STR(x)
#define STR(x) #x   // encloses x in ".."
#define GCC_Version XSTR(__cplusplus)
#pragma message "using GCC-Version: " GCC_Version
#endif

#endif

