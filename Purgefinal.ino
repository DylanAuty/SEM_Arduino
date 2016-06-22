/* Purgefinal.ino
* Set purge valve to open for a set time every 20 seconds.
* Written Claudia Jackaman-White, edited Dylan Auty.
*/

// constants won't change. Used here to set a pin number :
const int PurgePin =  13;      // the number of the Purge pin

int PurgeState = LOW;             // PurgeState used to set the pin

unsigned long previousMillis = 0;        // will store last time purge valve opened

// constants won't change :
const long purgeoffinterval = 20000;           // time at which purge valve closed (ms)
const long purgeoninterval = 500;          // length of time to purge (ms)

void setup() {
  // set the digital pin as output:
  pinMode(PurgePin, OUTPUT);
}

void loop() {
  // here is where you'd put code that needs to be running all the time.

  // If the difference between the current time and last
  // time purge valve closed than the interval at
  // which yu close the valv.
  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= purgeoninterval) {
    // save the last time you blinked the LED
    PurgeState = HIGH;
    // set the LED with the ledState of the variable:
    digitalWrite(PurgePin, PurgeState);
  }
  if (currentMillis - previousMillis >= purgeoninterval + purgeoffinterval){
    previousMillis = currentMillis;
    PurgeState = LOW;
    digitalWrite(PurgePin, PurgeState);
  }
 
}

