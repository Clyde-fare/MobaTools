/* MobaTools example for boards with WiFiNINA or WiFiS3 compatible chip 
** (e.g. Nano 33 IoT, UNO WiFi Rev 2, UNO R4 Wifi )
*/
#ifdef ARDUINO_UNOR4_WIFI
#include <WiFiS3.h>
#elif defined ARDUINO_RASPBERRY_PI_PICO_2W || defined ARDUINO_RASPBERRY_PI_PICO_W
#include <WiFi.h>
#else
#include <WiFiNINA.h>
#endif
#include <MobaTools.h>  // ab 2.4.0: https://github.com/MicroBahner/MobaTools

// select the language of the website
//#include "websiteDE.h"
#include "websiteEN.h"

//#define OWN_AP // with own Wifi Network ( connects to an existing network if commented out )

#ifdef OWN_AP
  const char *ssid = "MoToTEST";  // nsme of the Access Point, up to 32 chars
  const char *pass = "12345678";  // Password, at least 8 chars, max 64
#else                           // connect to an existing network
  #include "arduino_secrets.h"    // include your name and password here to connect to your existing WiFi
  char ssid[] = SECRET_SSID;  // your network SSID (name)
  char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
#endif
// change connection pins to your needs
const byte dirPin = 5; 
const byte stepPin = 6;
const byte enaPin = 7; 

enum class Aktionen { STOP,
                      LINKS,
                      RECHTS,
                      CONTL,
                      CONTR };      // FSM states
const int STEPS_REVOLUTION = 200; 

int status = WL_IDLE_STATUS;
WiFiServer server(80);

char txtBuf[30];
MoToStepper myStepper(STEPS_REVOLUTION, STEPDIR);  // create stepper object


void setup() {
  Serial.begin(115200);
  while (!Serial)   // needed for native USB
    ;

  //sprintf(txtBuf,"\nSketchname: %s\nBuild: %s\t\tIDE: %d.%d.%d\n\n", __FILE__, __TIMESTAMP__, ARDUINO / 10000, ARDUINO % 10000 / 100, ARDUINO % 100 / 10 ? ARDUINO % 100 : ARDUINO % 10);
  //Serial.print(txtBuf);
  delay(1000);
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

#ifdef OWN_AP
  // -------------- creating a new network (access point) -------------
  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  // WiFi.config(IPAddress(10, 0, 0, 1));

  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // Create open network.
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true)
      ;
  }
  //delay(5000);

#else 
 // --------------- connect to existing network -------------------
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);  // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
#endif

  // start the web server on port 80
  server.begin();

  // you're connected now, so print out the status
  printWiFiStatus();

  //initialize the stepper with default values
  myStepper.attach(stepPin, dirPin);        // Step- und dir-Pin
  myStepper.attachEnable(enaPin, 10, LOW);  // Enable pin  ( LOW=activ )
  myStepper.setSpeedSteps(20000);           // 200 Rev/min
  myStepper.setRampLen(300);                // Ramp length 300 steps

  Serial.println("Starting loop...");
}

long htSpeed = 20000;
long htRamp = 300;

void loop() {
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available();  // listen for incoming clients

  if (client) {
    constexpr byte lineMax = 255;
    byte lineIx = 0;
    char currentLine[lineMax];  // if you get a client,

    //Serial.println("new client");  // print a message out the serial port
    while (client.connected()) {   // loop while the client's connected
      delayMicroseconds(10);       // This is required for the Arduino Nano RP2040 Connect - otherwise it will loop so fast that SPI will never be served.
      if (client.available()) {    // if there's bytes to read from the client,
        char c = client.read();    // read a byte, then
        //Serial.write(c);           // print it out to the serial monitor
        if (c == '\n') {  // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:

          if (lineIx == 0) {
            char htmlTemp[sizeof(HTMLTEXT) + 15];  // Size of website+some extra bytes for 2 variable fields
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            #ifdef ARDUINO_BOARD
            int htmlSize = snprintf(htmlTemp, sizeof(htmlTemp), HTMLTEXT, "MoToStepper @" ARDUINO_BOARD, htSpeed / 10, htRamp);
            #elif defined USB_PRODUCT
            int htmlSize = snprintf(htmlTemp, sizeof(htmlTemp), HTMLTEXT, "MoToStepper @" USB_PRODUCT, htSpeed / 10, htRamp);
            #elif defined ARDUINO_AVR_UNO_WIFI_REV2
            int htmlSize = snprintf(htmlTemp, sizeof(htmlTemp), HTMLTEXT, "MoToStepper @ UNO WiFi Rev2", htSpeed / 10, htRamp);
            #elif defined ARDUINO_UNOR4_WIFI
            int htmlSize = snprintf(htmlTemp, sizeof(htmlTemp), HTMLTEXT, "MoToStepper @ UNO R4 WiFi", htSpeed / 10, htRamp);
            #else
            int htmlSize = snprintf(htmlTemp, sizeof(htmlTemp), HTMLTEXT, "MoToStepper @ unknown boardAP:", htSpeed / 10, htRamp);
            #endif
            client.print(htmlTemp);
            sprintf(txtBuf,"Html-Size = %d Byte\n\r", htmlSize); Serial.println(txtBuf);

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            // first check to see if the client requested a stepper command
            if (strstr(currentLine, "GET")) handleStepper(currentLine);
            lineIx = 0;
            currentLine[0] = '\0';
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          if (lineIx < lineMax) {
            currentLine[lineIx++] = c;  // add it to the end of the currentLine
            currentLine[lineIx] = '\0';
          }
        }
      }
    }
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  Serial.print("IP Address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.flush();
}
