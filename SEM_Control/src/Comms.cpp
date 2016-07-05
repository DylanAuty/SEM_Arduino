#include "Comms.h"
#include <Wire.h>
#include <I2C.h>

#define ADDRESS 8
static float data[DATA_SIZE];

void CommsSetup()
{
  #ifdef MASTER
  //Wire.begin();
  //I2c.begin();	// Commented out since this is done in the main setup
  #else
  Wire.begin(ADDRESS);
  Wire.onReceive(CommsReceive);
  #endif
}

void CommsSetValue(byte field, float value)
{
  if(field >= 0 && field < DATA_SIZE)
    data[field] = value;
}

float CommsGetValue(byte field)
{
  if(field >= 0 && field < DATA_SIZE)
    return data[field];
  return -1;
}

#ifdef MASTER
void HUDUpdate()
{
  /*
  Wire.beginTransmission(ADDRESS);
  Wire.write((byte*)data, DATA_SIZE * sizeof(float));
  Wire.endTransmission();
  */
	I2c.write(ADDRESS, DATA_SIZE * sizeof(float));
}
#else
void CommsReceive(int bytes)
{
  if(bytes > DATA_SIZE*sizeof(float))
    bytes = DATA_SIZE*sizeof(float);
  byte* ptr = (byte*)data;
  while(bytes-- > 0)
    *(ptr++) = Wire.read();
}
#endif

