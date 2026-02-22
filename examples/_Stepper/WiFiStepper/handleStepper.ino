void handleStepper(char *clientCommand) {        // Reaktion auf Eingaben und Html-Seite als Antwort an Browser zurückschicken
  Serial.print("Command: "); Serial.println(clientCommand);
  char *cmdPtr;
  long tmpSpeed = 20000;
  long tmpRamp = 300;

  if (strstr(clientCommand,"links=")) esp32Stepper(Aktionen::LINKS);   // http://192.168.4.1/stepper?links
  if (strstr(clientCommand,"rechts=")) esp32Stepper(Aktionen::RECHTS); // http://192.168.4.1/stepper?rechts
  if (strstr(clientCommand,"contl=")) esp32Stepper(Aktionen::CONTL);   // http://192.168.4.1/stepper?contl
  if (strstr(clientCommand,"contr=")) esp32Stepper(Aktionen::CONTR);   // http://192.168.4.1/stepper?contr
  if (strstr(clientCommand,"stop="))  esp32Stepper(Aktionen::STOP);     // http://192.168.4.1/stepper?stop
  cmdPtr = strstr(clientCommand,"speed=");
  if (cmdPtr) tmpSpeed = atoi( strchr(cmdPtr, '=')+1 );  
  cmdPtr = strstr(clientCommand,"ramp=");
  if (cmdPtr) tmpRamp = atoi( strchr(cmdPtr, '=')+1 );  

  if (strstr(clientCommand,"setSpeed")) {
    // seting speed and autoramp
    Serial.print("New speed: "); Serial.print(tmpSpeed ); Serial.println(" steps/sec");
    htSpeed = tmpSpeed*10;
    htRamp = myStepper.setSpeedSteps(htSpeed);
  }
  if (strstr(clientCommand,"setRamp")) {
    // seting ramplength
    Serial.print("New ramp: "); Serial.print(tmpRamp ); Serial.println(" steps");
    htRamp = tmpRamp;
    htRamp = myStepper.setRampLen(htRamp);
  }

}

void esp32Stepper(const Aktionen aktion) {

  switch (aktion) {
    case Aktionen::LINKS:
      myStepper.doSteps(-STEPS_REVOLUTION); // Stepper dreht eine Umdrehung links
      Serial.println(">>>>> One rev. left");
      break;
    case Aktionen::RECHTS:
      myStepper.doSteps(STEPS_REVOLUTION); // Stepper dreht eine Umdrehung rechts
      Serial.println(">>>>> One rev. right");
      break;
    case Aktionen::CONTL:
      myStepper.rotate( -1 ); // Stepper dreht links
      Serial.println(">>>>> Continuously left");
      break;
    case Aktionen::CONTR:
      myStepper.rotate( 1 ); // Stepper dreht rechts
      Serial.println(">>>>> Continuously right");
      break;
    case Aktionen::STOP:
      myStepper.rotate( 0 ); // Stepper stoppt
      Serial.println(">>>>> Stopping");
      break;
  }
  
}
