//Software Serial used to communicate with XBee Pro
#include <SoftwareSerial.h> 

SoftwareSerial XBee(19,18); // Pins connected to Arduino RX1, TX1 (XBee Dout, Din);

void setup() {
  XBee.begin(9600);       // Make sure that both ports are at the same baud (9600)
  Serial.begin(9600);     // Make sure baud rate matches config settings of XBee

}

void loop() {
 // If data is being sent to the serial monitor, 
 // it will send it out to XBee
 if (Serial.available()){
  XBee.write(Serial.read());
 }
 // If data comes from the XBee,
 // will send it to serial monitor
 if (XBee.available()){
  Serial.write(XBee.read());
 }

}
