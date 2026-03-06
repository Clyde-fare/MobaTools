#include <MobaTools.h>

const uint32_t BLENDZEIT = 750;
const byte button = 10;
const byte ledPins[] = { 2, 3, 4, 5, 6, 7, 8, 9 };
const byte pinCount = sizeof(ledPins);
enum { LAUF,
       AUSSCHALTEN,
       AUS };
byte schritt = LAUF;


MoToSoftLed meineLeds[pinCount];
MoToTimer myTimer;

void setup() {
  //Serial.begin(115200);
  //while(!Serial);
  //Serial.println("Anfang");
  pinMode(button, INPUT_PULLUP);

  for (byte led = 0; led < pinCount; led++) {
    meineLeds[led].attach(ledPins[led]);
    meineLeds[led].riseTime(BLENDZEIT);  // Aufblendzeit in ms
  }
}

void loop() {
  switch (schritt) {
    case LAUF:
      lauflicht();
      if (digitalRead(button) == LOW) {
        schritt = AUSSCHALTEN;
      }
      break;
    case AUSSCHALTEN:
      for (byte led = 0; led < pinCount; led++) {
        meineLeds[led].off();
      }
      delay(BLENDZEIT);
        schritt = AUS;
        break;
        case AUS:
          if (digitalRead(button) == LOW) {
            schritt = LAUF;
            delay(BLENDZEIT);
          }
          break;
        default:
          schritt = LAUF;
      }
  }
  void lauflicht() {
    static byte led = 0;
    static bool hin = true;

    if (!myTimer.running()) {
      meineLeds[hin ? led : pinCount - 1 - led].off();
      led = (1 + led) % (pinCount - 1);
      if (!led) hin = !hin;
      meineLeds[hin ? led : pinCount - 1 - led].on();
      myTimer.setTime(BLENDZEIT / 2);
    }
  }
