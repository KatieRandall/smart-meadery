// rht03.h
// Provides functions to initialize and take temperature reading on the temp & humidity sensor
// Used in Smart Meadery to monitor the temperature of the brewing fridge
// Credit: Grace True, Adapted from DS18B20 Temperature Sensor Code

char ds_init(void);
void ds_convert(void);
char ds_temp(unsigned char *);
