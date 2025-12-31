// ESP32 HW-specific parts for MobaTools
    //--------------------------------------------------------------------------------------------------------------
#ifndef ESP32S3_DRIVER_H
#define ESP32S3_DRIVER_H

#include "esp32-hal-timer.h"
#include "esp32-hal-spi.h"
#ifdef __cplusplus
extern "C" {
#endif
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  ESP32S3  vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
#define IS_32BIT
#define IS_ESP32S
//#define HAS_PWM_HW		// will not be used
#define MOTOSOFTLED32    // use 32-bit version of SoftLed class
#define servoCmp_t uint64_t	// values for 64-bit timer (default is uint16_t, because most timers are 16 bit )

// ----------------   stepper related defines   ---------------------------------
// use of SPI interface. Pinnumbers can be overriden by attach Method
// Default pinnumbers defined here are for Arduino Nano ESP32 
#define SPI_USED    FSPI	// Arduino Nano uses Arduino pin numbers by default
#define MISO		12 		//GPIO47	
#define MOSI        11 		//GPIO38	
#define SCK         13 		//GPIO48	
#define SS          10 		//GPIO21	

struct timerConfig_t {
  union {
    struct {
      uint32_t reserved0:   10;
      uint32_t alarm_en:     1;             /*When set  alarm is enabled*/
      uint32_t level_int_en: 1;             /*When set  level type interrupt will be generated during alarm*/
      uint32_t edge_int_en:  1;             /*When set  edge type interrupt will be generated during alarm*/
      uint32_t divider:     16;             /*Timer clock (T0/1_clk) pre-scale value.*/
      uint32_t autoreload:   1;             /*When set  timer 0/1 auto-reload at alarming is enabled*/
      uint32_t increase:     1;             /*When set  timer 0/1 time-base counter increment. When cleared timer 0 time-base counter decrement.*/
      uint32_t enable:       1;             /*When set  timer 0/1 time-base counter is enabled*/
    };
    uint32_t val;
  };
} ;
extern bool timerInitialized;
#define CYCLETIME       1                   // Cycle count in µs on 32Bit processors
// Prescaler for 64-Bit Timer ( input is 
#define DIVIDER     APB_CLK_FREQ/2/1000000  // 0,5µs Timertic ( 80MHz input freq )
#define TICS_PER_MICROSECOND 2              // bei 0,5 µs Timertic
// Mutexes für Zugriff auf Daten, die in ISR verändert werden
extern portMUX_TYPE stepperMux;

#if (ESP_ARDUINO_VERSION_MAJOR == 2)
// Only used for Core 2.X
#define STEPPER_TIMER     3  // Timer 1, Group 1 Wird in V3 der IDF automatisch gewählt??
#define SERVO_TIMER     2  // Timer 0, Group 1 Wird in V3 der IDF automatisch gewählt??
#endif
extern hw_timer_t * stepTimer;
extern hw_timer_t * servoTimer;
void seizeTimer1();		// überflüssig?? - wird wohl nicht mehr gebraucht

// --------------   defines for servo and softled ( timer hardware on ESP32S is used ) -----------------------

#ifdef COMPILING_MOTOSERVO_CPP
    //#warning compiling servo.cpp for ESP32
    #undef interrupts
    #undef noInterrupts
    #define interrupts()    portEXIT_CRITICAL(&servoMux);
    #define noInterrupts()  portENTER_CRITICAL(&servoMux);
#endif
#ifdef COMPILING_MOTOSOFTLED_CPP
    //#warning compiling softled.cpp for ESP32
    #undef interrupts
    #undef noInterrupts
    #define interrupts()    portEXIT_CRITICAL(&softledMux);
    #define noInterrupts()  portENTER_CRITICAL(&softledMux);
#endif
#ifdef COMPILING_MOTOSTEPPER_CPP
    //#warning compiling stepper.cpp for ESP32
    #undef interrupts
    #undef noInterrupts
    #define interrupts()    portEXIT_CRITICAL(&stepperMux);
    #define noInterrupts()  portENTER_CRITICAL(&stepperMux);
#endif

extern portMUX_TYPE softledMux;
extern portMUX_TYPE servoMux;
extern portMUX_TYPE timerMux;

    //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ end of ESP32 ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifdef __cplusplus
}
#endif
#define ARCHITECT_INCLUDE <esp32S3/MoToESP32S3.h>
#endif /* ESP32S3_DRIVER_H */

