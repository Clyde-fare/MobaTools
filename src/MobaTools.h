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
  V2.5.1 10-2023
    - Fix bug when setting stepper speed to 0 multiple times. The stepper stopped immediately when setting
      speed to 0 again while the stepper was ramping down from the first setting to 0.
	- Fix some bugs when setting low delay times on stepper enable.
  V2.5.0 09-2023
	- ESP32 board manager V2.x is supported, but the new HW variants (S2,S3,C3) are not yet supported
	- ATmega4809 is supported ( Nano Every, UNO WiFi Rev2 )
	- .setSpeedSteps(0) is allowed now and stops the stepper without loosing the target position
	- .getSpeedSteps() indicates direction of movement ( negative values means moving backwards )
	- .attachEnable( int delayTime ) allows disabling of 4-pin steppers (FULLSTEP/HALFSTEP) without
	  an extra enable pin ( all outputs are set to 0 ). This also works when connected via SPI.
	- some more examples
  V2.4.3 04-2022
	 - bugfix for setZero(position) for steppers in FULLSTEP mode
	 - bugfix with AccelStepper like method names ( compiler error if both libs have been included )
	 - 2 additional timer examples commented in english
	 - 1 additional stepper example
  V2.4.2 12-2021
	 - fix bug in MoToStepper.setSpeedSteps ( was possible divide by zero ), ESP crashed
  V2.4.1 11-2021
     - fix typo ( arduino.h -> Arduino.h ). This created an error on linux.
     - some fixes in MoToButtons and documentation
  V2.4.0 05-2021
     - ESP32 prozessors are supported
     - some ATtiny are supported ( needs a 16bit-timer1 amd a SPI or USI hardware )
     - Step-tuning for 32Bit prozessors ( except ESP8266 ) for higher steprates
     - more examples
  V2.3.1 11-2020
     - Fix error with doSteps(0) and no ramp. Motor did not stop
  V2.3 07-2020
     - New class MoToTimebase to create events in regular intervals
     - MoToButton: The longpress event is already triggered when the time for longpress expires, not when the button is released
     - MoToStepper: steps per rotation can be changed together with setting a new reference point.
     
  V2.2 03-2020
  V2.1 02-2020
      - new class 'MoToButtons' to manage up to 32 buttons/switches
        ( debounce, events 'pressed', 'released', 'short press' and 'long press'
      - MoToTimer: new method 'expired', which is only 'true' at the first call 
        after timer expiry.
  V2.0 01-2020
      - classnames changed ( the old names can still be used for compatibility, 
        but should not be used in new sketches)
      - ESP8266 is supported
      - it is possible to define an enable pin for steppers. This is active
        only when the stepper is moving.
      - new method 'getSpeedSteps' returns actual speed
  V1.1.5 12-2019
      - Servo: fix error when 1st and 2nd write ( after attach ) are too close together
  V1.1.4 09-2019
      - speed = 0 is not allowsed( it is set to 1 internally )
	  - fix error when repeatedly setting target position very fast
	  - allow higher steprates up to 2500 steps/sec.
	    ( relative jitter increases with higher rates, abs. jitter is 200µs )
	  - typo corrected in MoToBase.h 
  V1.1.3 08-2019
      - no more warnings when compiling
      - fix error (overflow) when converting angle to steps
      - fix error when converting angle to microseconds in servo class
      - reworked softleds. Risetimes unitl 10 sec are possible.
  V1.1.2 08-2019
      - fix error when only servo objects are defined ( sketch crashed )
      - two more stepper examples ( thanks to 'agmue' from german arduino.cc forum )
      - detach() configures used pins back to INPUT
  V1.1 07-2019
      - stepper now supports ramps (accelerating, decelerating )
      - stepper speed has better resolution with high steprates
      - split source into several fuction-specific .cpp files. This
        saves flash space if only part of the functionality is used.
  V1.0  11-2017 Use of Timer 3 if available ( on AtMega32u4 and AtMega2560 )
  V0.9  03-2017
        Better resolution for the 'speed' servo-paramter (programm starts in compatibility mode)
        outputs for softleds can be inverted
        MobaTools run on STM32F1 platform
        
  V0.8 02-2017
        Enable Softleds an all digital outputs
  V0.7 01-2017
		Allow nested Interrupts with the servos. This allows more precise other
        interrupts e.g. for NmraDCC Library.
		A4988 stepper driver IC is supported (needs only 2 ports: step and direction)

   
*/



// defines that may be changed by the user

    
// default CYCLETIME is processordependent, change only if you know what you are doing ).
// If it is not defined here it is 1 ( µs ). 
// CYCLETIME and MIN_STEP_CYCLE define the maximum step rate for steppers ( which also depends on the overall load )

#ifdef  ARDUINO_ARCH_ESP8266 ///////////////////////////////////////////////////////////
	#define CYCLETIME       60      // Min. irq-periode in us ( ESP-default is 60 )
									// = high time of Steppulse
	#define MIN_STEP_CYCLE  2       // Minimum number of cycles per step. 
									// = min low time of steppulse is CYCLETIME
	#define MAX_GPIO        10      // max number of usable gpios
	// at max 10 gpio's can be used at an ESP12: gpio 0,1,2,3,4,5,12,13,14,15
	// gpio 6-10 is internally used for flash
	// gpio16 has no interrupt capability ( but can be used as dir-pin for a stepper)

#elif defined ARDUINO_ARCH_STM32F1 /////////////////////////////////////////////////////
	#define MIN_STEP_CYCLE  25   // Minimum number of µsec  per step 
	//#define USE_SPI2
	#elif defined ARDUINO_ARCH_STM32F4 /////////////////////////////////////////////////////
	#define MIN_STEP_CYCLE  25   // Minimum number of µsec  per step 

#elif defined ARDUINO_ARCH_ESP32 ///////////////////////////////////////////////////////
		//#define USE_VSPI              // default is HSPI ( for SPI-Stepper )
										// on Nano ESP32 it is always FSPI
		#define MIN_STEP_CYCLE 20       // Minimum number of µsec  per Step
	//#endif
#elif defined ARDUINO_ARCH_AVR ////////////////////////////////////////////////////////
	//#define NO_TIMER3             // never use Timer 3
	//#define CYCLETIME       200   // Min. irq-periode in us ( default is 200 or 100 for LGT8Fx with 32MHz), 
	#define FASTSPI                 // only for devices with USI Interface ( instead of SPI HW )
									// if defined SPI clock ist CPU clock / 2
									// if not defined, SPI clock ist CPU clock / 4
	//#define USI_SS  7               // defines the SS - Pin with USI-SPI-Stepper
									// if not defined the core-default (SS) is used
#elif defined ARDUINO_ARCH_MEGAAVR ////////////////////////////////////////////////////////
	#define CYCLETIME       200     // Min. irq-periode in us ( default is 200 ), 
	#define MIN_STEP_CYCLE  2       // Minimum number of cycles per step. 
	#ifdef ARDUINO_AVR_NANO_EVERY
		// SPI SS for Nano Every 
		//#define MoToSS SS	// Standard for Every is pin 8;
		#define MoToSS 10	// Set it like other AVR-boards ( most Nano boards don't have a default SS )
	#elif defined ARDUINO_AVR_UNO_WIFI_REV2
		// SPI-SS for UNO Rev2 WiFi 
		#define MoToSS 10		// Rev2 has no standard SS ( standard is pin22, which is not connected to anything )
  #elif defined __AVR_TINY_2__
    // SPI SS for ATtiny 2 series
    #define MoToSS PIN_PA4	// Standard for ATtiny 2 series is pin PA4(0);
  #else
		// default for other ( are there any?) boards or megaCoreX core
		#define MoToSS 10		// standard for other boards
	#endif
#elif defined ARDUINO_ARCH_RENESAS_UNO ////////////////////////////////////////////////////////
	#define MIN_STEP_CYCLE  50       // Minimum number of µsec  per Step
	#define IRQ_PRIO 12				// NVIC priority. Servo irq is always one prio higher.
									// Lower priority ( higher value) will lead to problems on R4 WiFi 
									// with WiFi active
#elif defined ARDUINO_ARCH_RP2040 && !defined ARDUINO_ARCH_MBED  ///////////////////////////////////////////////////////////
	#define MIN_STEP_CYCLE 20       // Minimum number of µsec  per Step
	#define IRQ_PRIO 64				// NVIC priority of timer IRQ ( stepper&softleds) - 128 is default for SDK
									// lower value = higher priority
	//#define USE_SPI1				// if SPI 1 for SPI-Stepper should be used ( not possible on nano RP2040 )
	#define STP_TIMR_NBR 0          // can be set to 1 on RP2350 ( Pico 2 )
	#define SPI_CLOCK 2000000L
#elif defined ARDUINO_ARCH_SAMD ////////////////////////////////////////////////////////
	#define MIN_STEP_CYCLE  50       // Minimum number of µsec  per Step
	#define FAST_STEPRESET	1		// reset the step impulseat End of IRQ ( pulse may be short!!)
	#define IRQ_PRIO 3				// NVIC priority. There are only 4 different level (0...3)
									// Lower priority = higher value
									// with WiFi active
	#define MT_TIMER 5			// Timer used by MobaTools ( 3,4,5 selecting TC3, TC4 or TC5  is possible)
								// TC5 is default if the number is invalid or not set
								// Be aware that always two timer ( TCC2/TC3 and TC4/TC5 ) share a common clock source
	#ifndef PIN_SPI_SS
	#define PIN_SPI_SS 10		// Selct pin for SPI - if not defined in core files
	#endif
#else ///////////////////////////////////////////////////////////////////////////////////
    #error Processor not supported
#endif //////////////////////////////////////////////////////////////////////////////////

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

