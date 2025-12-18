#ifndef MOTOESP32S3_H
#define MOTOESP32S3_H
// ESP32 specific defines for Cpp files
#define TODO 	// ignore TODO-marker

#pragma message "ESP32S3 (Nano-ESP32) specific cpp includes"

void ISR_Servo();
void ISR_Stepper();

inline __attribute__((__always_inline__)) void _noStepIRQ() {
    portENTER_CRITICAL(&stepperMux);
    #if defined COMPILING_MOTOSTEPPER_CPP
    SET_TP3;
    #endif
}
inline __attribute__((__always_inline__)) void  _stepIRQ(bool force = true) { 
    // paramter force needed for compatibility with other architectures
        #if defined COMPILING_MOTOSTEPPER_CPP
            CLR_TP3;
        #endif
    portEXIT_CRITICAL(&stepperMux);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined COMPILING_MOTOSERVO_CPP
// Values for Servo: -------------------------------------------------------
constexpr uint8_t INC_PER_MICROSECOND = 8;		// one speed increment is 0.125 µs
constexpr uint8_t  COMPAT_FACT = 1; // no compatibility mode for ESP                   
// defaults for macros that are not defined in architecture dependend includes
constexpr uint8_t INC_PER_TIC = INC_PER_MICROSECOND / TICS_PER_MICROSECOND;
#define time2tic(pulse)  ( (pulse) *  INC_PER_MICROSECOND )
#define tic2time(inc)  ( ( inc ) / INC_PER_MICROSECOND )
#define AS_Speed2Inc(speed) (speed)
//-----------------------------------------------------------------
/* Bei Stepper.cpp und servo.cpp wird das gleiche seizeTimerAS(); zur initiierung aufgerufen,
   da bisher für Servos und Stepper der gleiche Timer verwendet wurde ( nur unterchiedliche CMP-Register )
   Der ESP hat aber je Timer nur ein Alarmregister, deshalb muss für Stepper und Servos jeweils ein
   eigener Timer genutzt ( und initiiert ) werden. Deshalb wird hier im .h File je nachdem
   was gerade kompiliert wird ein eigener Timer-Init eingerichtet. Dies muss als 'inline' gemacht
   werden, damit der Linker dies nicht sieht. Da der Aufruf in beiden Fällen den gleichen Namen
   hat, könnte der Linker das nicht unterscheiden.
   Weiteres Problem: Für Stepper und Softleds muss der gleiche Timer initiiert werden.
*/
static bool servoTimerInitialized = false;

static inline __attribute__((__always_inline__))void seizeTimerAS() {
    // Initiieren des Servo Timers ------------------------
    if ( !servoTimerInitialized ) {
		#if (ESP_ARDUINO_VERSION_MAJOR == 2)
			#pragma message "Info: using esp core 2.x.x"
        servoTimer = timerBegin(SERVO_TIMER, DIVIDER, true); // true= countup
        timerAttachInterrupt(servoTimer, &ISR_Servo, true);  // true= edge Interrupt
        timerAlarmWrite(servoTimer, ISR_IDLETIME*TICS_PER_MICROSECOND , false); // false = no autoreload );
        timerAlarmEnable(servoTimer);
		#elif (ESP_ARDUINO_VERSION_MAJOR == 3)
			#pragma message "Info: using esp core 3.x.x"
        // core 3.0.3 hw_timer_t * timerBegin(uint32_t frequency);   // frequency in Hz    
		servoTimer = timerBegin(2000000);  // frequency
        // core 3.0.3void timerAttachInterrupt(hw_timer_t * timer, void (*userFunc)(void));
        timerAttachInterrupt(servoTimer, &ISR_Servo); // assume edge - zs6buj
        timerAlarm(servoTimer, ISR_IDLETIME*TICS_PER_MICROSECOND , false, 0); // false = no autoreload );
		#else
		 #error "ESP-core version unsupported"
		#endif
			
        servoTimerInitialized = true;  
        MODE_TP1;   // set debug-pins to Output
        MODE_TP2;
        MODE_TP3;
        MODE_TP4;
    }
}

static inline __attribute__((__always_inline__)) void enableServoIsrAS() {
	// Enable timer compare IRQ for servos
	// at ESP32Sx a timer must be initialized here 
	//( it's not only a different CMP-reg of the stepper timer )
	TODO
}

static inline __attribute__((__always_inline__)) void setServoCmpAS(servoCmp_t aktAlarm) {
	// Set compare-Register for next servo IRQ
	#if (ESP_ARDUINO_VERSION_MAJOR == 2)
     timerAlarmWrite(servoTimer, aktAlarm , false); // no autorelaod
     timerAlarmEnable(servoTimer);
	#elif (ESP_ARDUINO_VERSION_MAJOR == 3)
    //3.0.3 void timerAlarm(hw_timer_t * timer, uint64_t alarm_value, bool autoreload, uint64_t reload_count);
    timerAlarm(servoTimer, aktAlarm , false, 0); // no autorelaod, 0=unlimited - zs6buj
	#else
		 #error "ESP-core version unsupported"
    #endif
}	

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef COMPILING_MOTOSOFTLED32_CPP

void seizeTimerAS();

static inline __attribute__((__always_inline__)) void enableSoftLedIsrAS() {
}



#endif  

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined COMPILING_MOTOSTEPPER_CPP
extern uint64_t lastAlarm, aktAlarm;

void seizeTimerAS() {
    // Initiieren des Stepper + Softled Timers ------------------------
	static bool stepperTimerInitialized = false;
	DB_PRINT("Servotimer initialize = %d\n\r", stepperTimerInitialized);
    if ( !stepperTimerInitialized ) {
		#if (ESP_ARDUINO_VERSION_MAJOR == 2)
			#pragma message "Info: using esp core 2.x.x"
        stepTimer = timerBegin(STEPPER_TIMER, DIVIDER, true); // true= countup
        timerAttachInterrupt(stepTimer, &ISR_Stepper, true);  // true= edge Interrupt
        timerAlarmWrite(stepTimer, ISR_IDLETIME*TICS_PER_MICROSECOND , false); // false = no autoreload );
        timerAlarmEnable(stepTimer);
		#elif (ESP_ARDUINO_VERSION_MAJOR == 3)
			#pragma message "Info: using esp core 3.x.x"
        // core 3.0.3 hw_timer_t * timerBegin(uint32_t frequency);   // frequency in Hz    
		stepTimer = timerBegin(2000000);  // frequency
        // core 3.0.3void timerAttachInterrupt(hw_timer_t * timer, void (*userFunc)(void));
        timerAttachInterrupt(stepTimer, &ISR_Stepper); // assume edge - zs6buj
        timerAlarm(stepTimer, ISR_IDLETIME*TICS_PER_MICROSECOND , false, 0); // false = no autoreload );
		#else
		 #error "ESP-core version unsupported"
		#endif
		aktAlarm = ISR_IDLETIME*TICS_PER_MICROSECOND; // time of first alarm
		DB_PRINT("aA=%ld, lA=%ld\n\r",aktAlarm, lastAlarm);
		DB_PRINT("Step-Timer eingerichtet\n\r");	
        stepperTimerInitialized = true;  
        MODE_TP1;   // set debug-pins to Output
        MODE_TP2;
        MODE_TP3;
        MODE_TP4;
    }
}



    static inline __attribute__((__always_inline__)) void enableStepperIsrAS() {
        // dummy
    }

    spi_t *spiHs = NULL;
    static uint8_t spiInitialized = false;
    static inline __attribute__((__always_inline__)) void initSpiAS() {
        if ( spiInitialized ) return;
        // initialize SPI hardware.
        // MSB first, default Clk Level is 0, shift on leading edge
        spiHs = spiStartBus(SPI_USED, SPI_CLOCK_DIV4, SPI_MODE0, SPI_MSBFIRST);
        //if ( spiHs == NULL ) Serial.println( "Init SPI failed");
        spiAttachSCK(spiHs, SCK);
        // MISO is not used, only serial output
        spiAttachMOSI(spiHs, MOSI);
        spiAttachSS(spiHs, 0, SS);
        spiSSEnable(spiHs);

        spiInitialized = true;  
    }

    static inline __attribute__((__always_inline__)) void startSpiWriteAS( uint8_t spiData[] ) {
       spiWriteShortNL(spiHs, (spiData[1]<<8) + spiData[0] );
    }    
    

#endif
  
#endif