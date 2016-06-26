/* Reading Analog Voltages from potential dividers
 *  Reads analog input pin, converts to actual voltage of fuel cell 
 *  from Vout of the potential dividers
 *  
 *  Written by Claudia JWhite
 *  05/01/2016
 */
int purgepin = 13;
unsigned long currentTime;
unsigned long previousTime;
const long purgeoffTime = 10000;
const long purgeTime = 10000;


void setup() {
  // intialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  pinMode(purgepin, OUTPUT);  // Set pin 13 as output for purge
  //purgeTime = currentTime;
  
  //initialize Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;   
  
  OCR1A = 31250;  // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12); 
  TCCR1B |= (1 << CS10); // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12);

  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  sei();                    // enable global interrupt
  
}

int R1 = 1000;


void pds(){
   // read the input on analog pin a0:
      int sensorValue0 = analogRead(A0);  
      float voltage0 = sensorValue0 * (5.0 / 1023.0);  
      Serial.print("voltage0 = ");
      Serial.print(voltage0);                           // print voltage value in Serial monitor
      Serial.println();    

      // read the input on analog pin a1:
      int sensorValue1 = analogRead(A1);                 // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
      float R21 = 2200.0; 
      float voltageA = sensorValue1 * (5.0 / 1023.0);    // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
      float voltage1 = voltageA * (R1/(R1+R21));    // This calculation depends on value of resistors in pds as vout=vin(R2 / R2+R1)
      Serial.print("voltage1 = ");
      Serial.print(voltage1);                            // print voltage value in Serial monitor
      Serial.println();
                                             
      // read the input on analog pin A3:
      int sensorValue3 = analogRead(A3);                 // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
      float R23 = 560.0;
      float voltageC = sensorValue3 * (5.0 / 1023.0);    // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
      float voltage3 = voltageC * ( R1 / (R23+R1));   
      Serial.print("voltage3 = ");
      Serial.print(voltage3);                             // print voltage value in Serial monitor
      Serial.println();
         
      // read the input on analog pin 5:
      int sensorValue5 = analogRead(A5);                 // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
      float R25 = 330.0;
      float voltageE = sensorValue5 * (5.0 / 1023.0);    // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
      float voltage5 = voltageE * ((R25 + R1) / R1);    
      Serial.print("voltage5 = ");
      Serial.print(voltage5);                            // print voltage value in Serial monitor
      Serial.println();
      
      // read the input on analog pin 7:
      int sensorValue7 = analogRead(A7);                 // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
      float R27 = 220.0;
      float voltageG = sensorValue7 * (5.0 / 1023.0);    // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
      float voltage7 = voltageG * ((R27 + R1) / R27);    
      Serial.print("voltage7 = ");
      Serial.print(voltage7);                            // print voltage value in Serial monitor
      Serial.println();
        
      // read the input on analog pin 2:
      int sensorValue9 = analogRead(A9);                // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
      float R29 = 180.0;
      float voltageI = sensorValue9 * (5.0 / 1023.0);   // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
      float voltage9 = voltageI * ((R29 + R1) / R29);   
      Serial.print("voltage9 = ");
      Serial.print(voltage9);                           // print voltage value in Serial monitor
      Serial.println();                                 // print onto new line
      // need statment to quash unwanted vin??
      Serial.println();
     

      delay(5000);
  }

void loop() {
  while(1) {       // infinite loop
   pds();           

   // currently purging every minute as 60000ms.
   // Alter this number to increase/decrease purge time 
    currentTime = millis();                           
  
  //if ((currentTime - previousTime) >= purgeoffTime ){       // change 60000 to alter purge time
    //digitalWrite(purgepin, HIGH);             // toggle the purge pin to high (on)
    //previousTime = currentTime;
    //}
   //if (currentTime >= (purgeTime + 120000)){       // after 2 minutes (1 minute of purging)
     //digitalWrite(13, !digitalRead(13));           // toggle purge pin to low (off)
    //}
    
   //purgeoffTime = currentTime;    //Update purgeoffTime
   //purgeTime = currentTime;      // Update purgeTime
     }
      //int purge=digitalRead(13);
      //Serial.print(purge);
      //Serial.println();
      Serial.print(currentTime);
     }
     

    
  
