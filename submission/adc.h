// adc.h
// Provides functions to initialize and take samples on the Atmega328p's ADC channels
// Used in Smart Meadery to read in the phototransistor input value as a 0-255 ADC value for more precise bubble counting
// Credit: USC EE109

void adc_init(void);
uint8_t adc_sample(uint8_t);