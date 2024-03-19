/*
* TWI_Test.c
*
* Created: 08-Jun-19 10:06:47 AM
* Author : TEP SOVICHEA
*/

#include "twi_master.h"
#include <avr/io.h>
#include <util/delay.h>

/************************************************************************/
/*							Initializations                             */
/************************************************************************/

#define LCD_ADDR	0x28

/************************************************************************/
/*							Main application                            */
/************************************************************************/

int main(void)
{

	tw_init(TW_FREQ_50K, false); // set I2C Frequency, enable internal pull-up

	// set pins to be outputs (ddr to 1)
	DDRC |= (1 << PC0);

	// set all outputs to zero (low)
	PORTC &= ~(1 << PC0);

	uint8_t prefix[] = { 0xFE };
	uint8_t DISPLAY_ON[] = { 0x42 };   // Data packet to turn the display on
	uint8_t PRINT_I2c[] = { 0x72 };
	uint8_t CURSER_HOME[] = { 0x46 };
	uint8_t CURSER_RIGHT[] = { 0x47 };
	uint8_t CLEAR_SCREEN[] = { 0x51 };
	uint8_t H[] = { 0x48 };
	uint8_t E[] = { 0x65 };
	uint8_t L[] = { 0x6C };
	uint8_t O[] = { 0x6F };


	bool repeat_start = false;  // Whether to repeat start condition, set to false for most cases

	while (1)
	{
		tw_master_transmit(LCD_ADDR, prefix, sizeof(prefix), repeat_start);
		tw_master_transmit(LCD_ADDR, CURSER_HOME, sizeof(CURSER_HOME), repeat_start);


		tw_master_transmit(LCD_ADDR, prefix, sizeof(prefix), repeat_start);
		tw_master_transmit(LCD_ADDR, CLEAR_SCREEN, sizeof(CLEAR_SCREEN), repeat_start);

		tw_master_transmit(LCD_ADDR, prefix, sizeof(prefix), repeat_start);
		tw_master_transmit(LCD_ADDR, CURSER_HOME, sizeof(CURSER_HOME), repeat_start);

		tw_master_transmit(LCD_ADDR, H, sizeof(H), repeat_start);
		tw_master_transmit(LCD_ADDR, E, sizeof(E), repeat_start);
		tw_master_transmit(LCD_ADDR, L, sizeof(L), repeat_start);
		tw_master_transmit(LCD_ADDR, L, sizeof(L), repeat_start);
		tw_master_transmit(LCD_ADDR, O, sizeof(O), repeat_start);

		_delay_ms(1000);
	}
}