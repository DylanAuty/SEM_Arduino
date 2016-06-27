/* SEM_Pit_Monitor.ino
* Receives from XBee and prints to serial monitor
* 
* Dylan Auty, 27/06/16
*/

void setup(){
	Serial.begin(9600);
}

void loop(){
	if(Serial.available() > 0){
		Serial.print(char(Serial.read()));
	}
}
