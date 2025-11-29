// SamD21 HW-spcific Functions
#ifdef ARDUINO_ARCH_SAMD

#include <MobaTools.h>
//#define debugTP
//#define debugPrint
#include <utilities/MoToDbg.h>

//#warning "HW specfic - Samd ---"
TcCount16 *MtcP = (TcCount16 *)MT_TIMER;	// Pointer to regs of used timer
uint8_t noStepISR_Cnt = 0;   // Counter for nested StepISr-disable

void stepperISR(int32_t cyclesLastIRQ)  __attribute__ ((weak));
void softledISR(uint32_t cyclesLastIRQ)  __attribute__ ((weak));
void ISR_Stepper();

void MT_Handler() {
	// This is the IRQ-Handler for the timer used by MobaTools ( for Stepper, Softled and Servo  )
    if (MtcP->INTFLAG.reg & STEP_INT_MSK ) 
    {	// was  compare interrupt for steppers
		ISR_Stepper();
        MtcP->INTFLAG.reg = STEP_INT_MSK; // Acknowledge stepper interrupt by setting maskbit
    }
    if (MtcP->INTFLAG.reg & SERVO_INT_MSK) // was  compare interrupt for servos
    {
		ISR_Servo();
        MtcP->INTFLAG.reg = SERVO_INT_MSK; // Acknowledge servo interrupt
    }
	
	
}
nextCycle_t nextCycle;
static nextCycle_t cyclesLastIRQ = 1;  // cycles since last IRQ
void ISR_Stepper() {
    // TCn Channel 1, used for stepper motor and softleds, starts every nextCycle us
    // nextCycle ist set in stepperISR and softledISR
    SET_TP1;
    nextCycle = ISR_IDLETIME  / CYCLETIME ;// min ist one cycle per IDLETIME
    if ( stepperISR ) stepperISR(cyclesLastIRQ);
    //============  End of steppermotor ======================================
    if ( softledISR ) softledISR(cyclesLastIRQ);
    // ======================= end of softleds =====================================
    // set compareregister to next interrupt time;
	// next ISR must be at least MIN_STEP_CYCLE/4 beyond actual counter value ( time between to ISR's )
	uint16_t minOCR = MtcP->COUNT.reg;
	uint16_t nextOCR = MtcP->CC[0].reg;  // cc0 = Step cmp
	//if ( minOCR < nextOCR ) minOCR += 0x10000; // timer had overflow already
    minOCR = minOCR + ( (MIN_STEP_CYCLE/4) * TICS_PER_MICROSECOND ); // minimumvalue for next OCR
	nextOCR = nextOCR + ( nextCycle * TICS_PER_MICROSECOND );
	if ( nextOCR < minOCR ) {
		// time till next ISR ist too short, set to mintime and adjust nextCycle
        SET_TP2;
		nextOCR = minOCR;
		nextCycle = ( nextOCR - MtcP->CC[0].reg  ) / TICS_PER_MICROSECOND;
        CLR_TP2;
	}
    MtcP->CC[0].reg =  (uint16_t)nextOCR ;
    cyclesLastIRQ = nextCycle;
    CLR_TP1; // Oszimessung Dauer der ISR-Routine
}
////////////////////////////////////////////////////////////////////////////////////////////
void seizeTimerAS() {
    static bool timerInitialized = false;
    if ( !timerInitialized ) {
		// first initialize the clock for the timer (GCLOCK)
		// GCLK3 is the 8MHz Clock generator
		REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK3 | GCLK_CLKCTRL_ID(GCM_MT_ID));
		while (GCLK->STATUS.bit.SYNCBUSY == 1) ;
		
		MtcP->CTRLA.reg = 0;                // disable timer for configuration
		MtcP->CTRLA.reg |= TC_CTRLA_SWRST;  // set timer to default values ( 16bit, count up, normal frequency
		while (MtcP->STATUS.bit.SYNCBUSY == 1) ;

		// Divider of 4 results to 2MHz clock
		// Set prescaler
		MtcP->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV4; //  =0.5µS per tic
		while (MtcP->STATUS.bit.SYNCBUSY == 1) ;
		MtcP->CTRLA.reg |= TC_CTRLA_PRESCSYNC_PRESC; // clear timer count on prescaler clock
		while (MtcP->STATUS.bit.SYNCBUSY == 1) ;

		// Enable InterruptVector
		NVIC_EnableIRQ(TCx_IRQn);

		// Enable the compare interrupt
		// Enable TC
		MtcP->CTRLA.reg |= TC_CTRLA_ENABLE;
		while (MtcP->STATUS.bit.SYNCBUSY == 1) ;

		MtcP->CTRLBSET.reg |= TC_CTRLBSET_CMD_RETRIGGER; //  Start
	}

}



extern "C" {
// TODO ------------------------  ISR for SPI-Stepper ------------------------
//static int rxData;
#ifdef USE_SPI2
/*
void __irq_spi2(void) {// STM32  spi2 irq vector
    rxData = spi_rx_reg(SPI2);            // Get dummy data (Clear RXNE-Flag)
    digitalWrite(BOARD_SPI2_NSS_PIN,HIGH);
}
#else
void __irq_spi1(void) {// STM32  spi1 irq vector
    //SET_TP4;
    rxData = spi_rx_reg(SPI1);            // Get dummy data (Clear RXNE-Flag)
    digitalWrite(BOARD_SPI1_NSS_PIN,HIGH);
    //CLR_TP4;
}
*/
#endif

} // end of extern "C"



#endif
