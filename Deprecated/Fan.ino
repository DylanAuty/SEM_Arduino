int Pin =12;      // set ouput pin of fan
unsigned long Time;
unsigned long fanoffTime;
unsigned long fanonTime;
unsigned long currentTime;

void setup() {
  pinMode(Pin, OUTPUT);     // set fan pin as output
  digitalWrite(Pin,0);      // set pin to low to begin 
  fanoffTime = Time;
  fanonTime = Time;
}

// currently fan turning on every minute as 60000ms.
// Alter this number to increase/decrease fan turning on time 
void loop() {
 // int fan = digitalRead(Pin);
  Serial.println("fan");
  
  Time = millis();
  //currentTime = millis();
  
  if (Time >= (fanoffTime + 10000)){        // change 60000 to alter fan turning on time
    digitalWrite(Pin, 1);                   // toggle the fan pin to high (on)
    }
   if (Time >= (fanonTime + 20000)){       // after 2 minutes (1 minute of being on)
     digitalWrite(Pin, 0);                        // toggle fan pin to low (off)
    }

   fanoffTime = Time;    //Update fanoffTime
   fanonTime = Time;      // Update fanonTime
    
  }

