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

// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>
 
#define LEDPIN 13
#define PURGE_CLOSED_SECONDS 5.0
#define PURGE_OPEN_SECONDS 2.0

// Global variable for timing, accessible from ISR.
volatile int time1Counter = 0;		// Timer 1, to check time between purge valve openings.
volatile byte valveOpen = 0;		// Flag to signal whether purge valve is open or not.
volatile int valveOpenCounter = 0;	// Timer 1, to check how long the purge valve has been open for.

void setup(){
    pinMode(LEDPIN, OUTPUT);
 
    // initialize Timer1
    cli();          // disable global interrupts
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
	
    // set compare match register to desired timer count:
    // Arduino Mega 2560 has 16MHz clock
	// Divide by 1024 to get 15625 Hz.
	// Desired resolution of purge valve is 0.5s => 7812 counts until interrupt.
	OCR1A = 7812;
    // turn on CTC mode:
    TCCR1B |= (1 << WGM12);
    // Set CS10 and CS12 bits for 1024 prescaler, 16MHz clock -> 15.625kHz increments.:
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    // enable global interrupts:
    sei();
	
	// Set up serial connection
	Serial.begin(9600);
}
 
void loop(){
	
}
 
ISR(TIMER1_COMPA_vect){
	if(valveOpen == 1){
		valveOpenCounter++;
		if(valveOpenCounter >= (2 * PURGE_OPEN_SECONDS)){
			Serial.println("CLOSE VALVE");
			digitalWrite(LEDPIN, LOW);
			valveOpenCounter = 0;		// Valve open timer reset to 0.
			valveOpen = 0;				// Valve status reset to closed.
		}
	}
	else{
		time1Counter++;
		if(time1Counter >= (2 * PURGE_CLOSED_SECONDS)){
			Serial.println("OPEN VALVE");
    		digitalWrite(LEDPIN, HIGH);
			time1Counter = 0;			// Valve closed timer reset to 0.
			valveOpen = 1;				// Valve status set to open.
		}
	}
}

