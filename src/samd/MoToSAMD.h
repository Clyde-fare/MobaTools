#ifndef MOTOSAMD_H
#define MOTOSAMD_H
// SAMD specific defines for Cpp files

//#warning SAMD specific cpp includes
extern uint8_t noStepISR_Cnt;   // Counter for nested StepISr-disable
extern TcCount16 *MtcP;	// Pointer to timerstruct of used timer

void seizeTimerAS();

static inline __attribute__((__always_inline__)) void _noStepIRQ() {
    //it is only possible to disable the whole timer IRQ ( there are no different vectors for the 2 compare )
    NVIC_DisableIRQ(TCx_IRQn);
    noStepISR_Cnt++;
}
static inline __attribute__((__always_inline__)) void  _stepIRQ(bool force = false) {
    //timer_enable_irq(MT_TIMER, TIMER_STEPCH_IRQ) cannot be used, because this also clears pending irq's
    if ( force ) noStepISR_Cnt = 1; //enable IRQ immediately
    if ( noStepISR_Cnt > 0 ) noStepISR_Cnt -= 1; // don't decrease if already 0 ( if enabling IRQ is called too often )
    if ( noStepISR_Cnt == 0 ) {
        NVIC_EnableIRQ(TCx_IRQn);
    }
    //Serial.println(noStepISR_Cnt);
}

void ISR_Servo();

/////////////////////////////////////////////////////////////////////////////////////////////////
#if defined COMPILING_MOTOSERVO_CPP
// Values for Servo: -------------------------------------------------------
constexpr uint8_t INC_PER_MICROSECOND = 8;		// one speed increment is 0.125 µs
constexpr uint8_t  COMPAT_FACT = 1; // no compatibility mode for Samd                    
// defaults for macros that are not defined in architecture dependend includes
constexpr uint8_t INC_PER_TIC = INC_PER_MICROSECOND / TICS_PER_MICROSECOND;
#define time2tic(pulse)  ( (pulse) *  INC_PER_MICROSECOND )
#define tic2time(inc)  ( ( inc ) / INC_PER_MICROSECOND )
#define AS_Speed2Inc(speed) (speed)
//-----------------------------------------------------------------



static inline __attribute__((__always_inline__)) void enableServoIsrAS() {
	// Enable timer compare IRQ for servos
	MtcP->INTENSET.reg = SERVO_INT_MSK;
}

static inline __attribute__((__always_inline__)) void setServoCmpAS(uint16_t cmpValue) {
	// Set compare-Register for next servo IRQ
	MtcP->CC[1].reg = cmpValue;
}	

#endif // COMPILING_MOTOSERVO_CPP

/////////////////////////////////////////////////////////////////////////////////////////////////
#if defined COMPILING_MOTOSOFTLED32_CPP
static inline __attribute__((__always_inline__)) void enableSoftLedIsrAS() {
	MtcP->INTENSET.reg = STEP_INT_MSK;
}

#endif // COMPILING_MOTOSOFTLED_CPP

//////////////////////////////////////////////////////////////////////////////////////////////////
#if defined COMPILING_MOTOSTEPPER_CPP

/*static const spi_pins_t board_spi_pins[BOARD_NR_SPI] __FLASH__ =
{
};
*/

static inline __attribute__((__always_inline__)) void enableStepperIsrAS() {
	MtcP->INTENSET.reg = STEP_INT_MSK;
}

static uint8_t spiInitialized = false;
static inline __attribute__((__always_inline__)) void initSpiAS() {
    if ( spiInitialized ) return;
    // initialize SPI hardware.
    // MSB first, default Clk Level is 0, shift on leading edge
	//TODO - initialize Samd SPI
    spiInitialized = true;  
}

static inline __attribute__((__always_inline__)) void startSpiWriteAS( uint8_t spiData[] ) {
	// TODO write step pattern over SPI
}    
    

#endif // COMPILING_MOTOSTEPPER_CPP


#endif