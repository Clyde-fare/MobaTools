// SamD21 HW-spcific Functions
#ifdef ARDUINO_ARCH_SAMD

#define debugTP

#include <MobaTools.h>

//#warning "HW specfic - Samd ---"
TcCount16 *MtcP = (TcCount16 *)TCx;	// Pointer to regs of used timer
uint8_t noStepISR_Cnt = 0;   // Counter for nested StepISr-disable

void stepperISR(nextCycle_t cyclesLastIRQ)  __attribute__ ((weak));
void softledISR(nextCycle_t cyclesLastIRQ)  __attribute__ ((weak));
void ISR_Stepper();

void TCx_Handler() {
	// This is the IRQ-Handler for the timer used by MobaTools ( for Stepper, Softled and Servo  )
	SET_TP1; // to measure time in ISR
    if (MtcP->INTFLAG.reg & STEP_INT_MSK ) 
    {	// was  compare interrupt for steppers and Softleds
		ISR_Stepper();
        MtcP->INTFLAG.reg = STEP_INT_MSK; // Acknowledge stepper interrupt by setting maskbit
    }
    if (MtcP->INTFLAG.reg & SERVO_INT_MSK) // was  compare interrupt for servos
    {
		//ISR_Servo();
        MtcP->INTFLAG.reg = SERVO_INT_MSK; // Acknowledge servo interrupt
    }
	/*// only debugging:
    if (MtcP->INTFLAG.reg & TC_INTFLAG_OVF) // was timer overflow 
    {	
		SET_TP2;
		MtcP->INTENSET.reg = TC_INTFLAG_OVF;
		MtcP->INTFLAG.reg = TC_INTFLAG_OVF; // Acknowledge servo interrupt
		CLR_TP2;
    }
	*/
	CLR_TP1; 
	
}
nextCycle_t nextCycle;
static nextCycle_t cyclesLastIRQ = 1;  // cycles since last IRQ
void ISR_Stepper() {
    // TCn Channel 1, used for stepper motor and softleds, starts every nextCycle us
    // nextCycle ist set in stepperISR and softledISR
    nextCycle = ISR_IDLETIME  / CYCLETIME ;// min ist one cycle per IDLETIME
    if ( stepperISR ) stepperISR(cyclesLastIRQ);
    //============  End of steppermotor ======================================
    if ( softledISR ) softledISR(cyclesLastIRQ);
    // ======================= end of softleds =====================================
    // set compareregister to next interrupt time;
	// next ISR must be at least MIN_TIC_DIFF beyond actual counter value ( time between two ISR's )
	uint16_t add2Ocr = nextCycle * TICS_PER_MICROSECOND; // tics to add to current compare reg
	CLR_TP1;
	uint16_t minDiff = (MtcP->COUNT.reg+MIN_TIC_DIFF) - MtcP->CC[0].reg;
	if (  minDiff >= add2Ocr ) {
		// counter is already too far
        SET_TP2;
		add2Ocr = minDiff;
		nextCycle = add2Ocr / TICS_PER_MICROSECOND;
        CLR_TP2;
	}
    MtcP->CC[0].reg =  MtcP->CC[0].reg + add2Ocr ;
    cyclesLastIRQ = nextCycle;
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
		NVIC_SetPriority(TCx_IRQn,0);

		// Enable TC
		MtcP->CTRLA.reg |= TC_CTRLA_ENABLE;
		while (MtcP->STATUS.bit.SYNCBUSY == 1) ;
		
		timerInitialized = true;
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
