// ESP32 HW-spcific Functions
//#define debugTP
//#define debugPrint
#include <MobaTools.h>
#ifdef CONFIG_IDF_TARGET_ESP32S3

#pragma message "compilingMoToESP32S3"

//#warning "HW specfic - ESP32S3 ---"

bool spiInitialized = false;
void IRAM_ATTR stepperISR(nextCycle_t cyclesLastIRQ)  __attribute__ ((weak));
void softledISR(nextCycle_t cyclesLastIRQ)  __attribute__ ((weak));
nextCycle_t nextCycle;
static nextCycle_t cyclesLastIRQ = 1;  // cycles since last IRQ
uint64_t lastAlarm, aktAlarm;

void IRAM_ATTR ISR_Stepper(void) {
    // Timer running up, used for stepper motor. No reload of timer
    SET_TP1;
    nextCycle = ISR_IDLETIME  / CYCLETIME ;// min ist one cycle per IDLETIME
    portENTER_CRITICAL_ISR(&stepperMux);
    cyclesLastIRQ = (aktAlarm - lastAlarm) / TICS_PER_MICROSECOND;
    if ( stepperISR ) stepperISR(cyclesLastIRQ);
    //============  End of steppermotor ======================================
	CLR_TP1;
    if ( softledISR ) softledISR(cyclesLastIRQ);
    // ======================= end of softleds =====================================
	// next alarm ISR must be at least MIN_STEP_CYCLE/2 beyond last alarm value ( time between to ISR's )
    lastAlarm = aktAlarm;
    aktAlarm = lastAlarm+(nextCycle*TICS_PER_MICROSECOND); // minimumtime until next Interrupt
    uint64_t minNextAlarm = lastAlarm + (MIN_STEP_CYCLE*TICS_PER_MICROSECOND/2);
	if ( aktAlarm < minNextAlarm ) {
		// time till next ISR ist too short, set to mintime and adjust nextCycle
        CLR_TP1;
		aktAlarm =  minNextAlarm;
	}
	#if (ESP_ARDUINO_VERSION_MAJOR == 2)
     timerAlarmWrite(stepTimer, aktAlarm , false); // no autorelaod
     timerAlarmEnable(stepTimer);
	#elif (ESP_ARDUINO_VERSION_MAJOR == 3)
    //3.0.3 void timerAlarm(hw_timer_t * timer, uint64_t alarm_value, bool autoreload, uint64_t reload_count);
    timerAlarm(stepTimer, aktAlarm , false, 0); // no autorelaod, 0=unlimited - zs6buj
	#else
		 #error "ESP-core version unsupported"
    #endif
    SET_TP1;
    portEXIT_CRITICAL_ISR(&stepperMux);
    CLR_TP1; // Oszimessung Dauer der ISR-Routine
}
////////////////////////////////////////////////////////////////////////////////////////////
timerConfig_t timerConfig;
portMUX_TYPE stepperMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE servoMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE softledMux = portMUX_INITIALIZER_UNLOCKED;
// Because there is only one Alarmreg per timer, we need two distinct timer for stepper and servo
hw_timer_t * stepTimer = NULL;
hw_timer_t * servoTimer = NULL;



void enableServoIsrAS() {
}


void enableSoftLedIsrAS() {

}


#endif
