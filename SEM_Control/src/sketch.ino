/* SEM_Control 
* Arduino code to handle:
*	- Opening and closing the purge valve periodically
*	- Reading of mean temperature data from 4 thermistors
*		- Storing these values to SD card
*		- Sending these values over XBee to another Arduino
*	- Using temperature data to linearly control the fan system (via a PWM fan controller)
*	- Reading battery voltages from I2C (another system does the measurement)
*	- Updating the HUD.
*	
* Dylan Auty, June 2016
*/

#define LED_PIN 13

void setup()
{
    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(900);
}
