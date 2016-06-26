/* Reading Analog Voltages from potential dividers
 *  Reads analog input pin, converts to actual voltage of fuel cell 
 *  from Vout of the potential dividers
 *  
 *  Written by Claudia JWhite
 *  05/01/2016
 */



const int PurgePin =  13;      // the number of the Purge pin
unsigned long previousMillis = 0;        // will store last time LED was updated
const long purgeoninterval = 5000;           // time at which to blink (milliseconds)
const long purgeoffinterval = 5000;          // length of time to purge (millisecond)

void setup() {
  // intialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  pinMode(PurgePin, OUTPUT);  // Set pin 13 as output for purge
  digitalWrite(PurgePin, LOW); // Initial state of purge pin
}

float R1 = 1000.0;


void pds() {
  
  unsigned long TimerA = millis();

  float R1 = 1000.0;
if (TimerA - updatedTimer >= pdmeasurment) {
    // read the input on analog pin a1:
    int sensorValue1 = analogRead(A2);                 // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
    float R21 = 2200.0;
    float voltageA = sensorValue1 * (5.0 / 1023.0);    // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
    float voltage1 = voltageA * ((R21 + R1) / R21); // This calculation depends on value of resistors in pds as vout=vin(R2 / R2+R1)
    Serial.print("voltage1 = ");
    Serial.print(voltage1);                            // print voltage value in Serial monitor
    Serial.println();
  
    // read the input on analog pin A3:
    int sensorValue3 = analogRead(A3);                 // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
    float R23 = 560.0;
    float voltageC = sensorValue3 * (5.0 / 1023.0);    // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
    float voltage2 = voltageC * ((R23 + R1) / R23);
    Serial.print("voltage2 = ");
    Serial.print(voltage2);                             // print voltage value in Serial monitor
    Serial.println();
  
    // read the input on analog pin 5:
    int sensorValue5 = analogRead(A4);                 // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
    float R25 = 330.0;
    float voltageE = sensorValue5 * (5.0 / 1023.0);    // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
    float voltage3 = voltageE * ((R25 + R1) / R25);
    Serial.print("voltage3 = ");
    Serial.print(voltage3);                            // print voltage value in Serial monitor
    Serial.println();
  
    // read the input on analog pin 7:
    int sensorValue7 = analogRead(A5);                 // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
    float R27 = 220.0;
    float voltageG = sensorValue7 * (5.0 / 1023.0);    // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
    float voltage4 = voltageG * ((R27 + R1) / R27);
    Serial.print("voltage4 = ");
    Serial.print(voltage4);                            // print voltage value in Serial monitor
    Serial.println();
  
    // read the input on analog pin 9:
    int sensorValue9 = analogRead(A6);                // Convert the analog reading (0-1023) to a Arduino voltage (0-5V)
    float R29 = 180.0;
    float voltageI = sensorValue9 * (5.0 / 1023.0);   // Convert the Arduino voltage to the real fuel cell Voltage (0-30V)
    float voltage5 = voltageI * ((R29 + R1) / R29);
    Serial.print("voltage5 = ");
    Serial.print(voltage5);                           // print voltage value in Serial monitor
    Serial.println();                                 // print onto new line
    // need statment to quash unwanted vin??
    Serial.println();
  
    String dataString = String(id) + ", " + String(voltage1) + ", " + String(voltage2) + ", " + String(voltage3)+ ", " + String(voltage4)+ ", " + String(voltage5);
    
        FuelCellData = SD.open("FCdata.csv", FILE_WRITE);
      if (FuelCellData) {
        FuelCellData.println(dataString);
        FuelCellData.close();
  
       id++;
       
    }
    
    updatedTimer = TimerA;  
}
}

void loop() {
  while(1) {       // infinite loop
    pds();
      unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= purgeoffinterval) {
    digitalWrite(PurgePin, HIGH);
  }
  if (currentMillis - previousMillis >= purgeoninterval + purgeoffinterval){
    previousMillis = currentMillis;
    digitalWrite(PurgePin, LOW);
  }
    
  }
              
     }
     

    
  
