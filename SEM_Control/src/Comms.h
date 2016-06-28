/// This system uses I2C, and requires the relevant pins to be available to connect to the HUD module.
/// Include this file, and uncomment the next line on the side that sends data (i.e. if you're reading this and aren't me)
#define MASTER
#pragma once
#include <Arduino.h>

enum CommsField
{
  /// The current speed
  F_SPEED = 0x0,
  /// GPS X coordinate
  F_GPSX = 0x1,
  /// GPS Y coordinate
  F_GPSY = 0x2,
  /// Cell voltages (0-n)
  F_VOLTAGE = 0x3,
  /// Ideal speed at the i'th timestep from now (0-n)
  F_IDEALSPEED = 0x10,
};
#define DATA_SIZE 0x20

/// Call this in `setup` to initialize communications
void CommsSetup();

/// Sets data values
/// e.g. to set the 3rd cell voltage:
///    CommsSetValue(F_VOLTAGE + 2, 0.5);
/// In the final code, all the fields defined in CommsField should be set somewhere
void CommsSetValue(byte field, float value);

/// Gets data values from the array
float CommsGetValue(byte field);

#ifdef MASTER
/// Sends the whole data block to the slave/recipient. Call this as frequently as possible
void HUDUpdate();
#else
/// Called automatically on the slave side to receive the data
void CommsReceive(int bytes);
#endif

