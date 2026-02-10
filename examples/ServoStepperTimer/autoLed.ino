void autoLedFSM() {
  static bool initialCall = true;
 static byte intervallCount = 0;
  const byte intMax = 4; // riseTime is changed
  static int ledRisetime = 1500;

  if ( initialCall ) {
    // first call of function, initiate everything
    initialCall = false;
    ledTimer.setTime(ledRisetime+100); // timer 2sec
    blinkLed.attach(led1Pin);
    blinkLed.riseTime(ledRisetime);
  } else {
    if ( ledTimer.expired() ) {
      blinkLed.toggle();
      if ( ++intervallCount >= intMax ) {
        // change direction
        if ( ledRisetime > 1000) {
          ledRisetime = 500;
        } else {
          ledRisetime = 1500;
        }
        	blinkLed.riseTime(ledRisetime);
        intervallCount = 0;
      } 
      ledTimer.setTime(ledRisetime+100);
    }
  }

}
