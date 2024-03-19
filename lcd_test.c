#include "twi_master.h"
#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

#define LCD_ADDR	0x28

int main(void)
{

	tw_init(TW_FREQ_50K, false); // set I2C Frequency, enable internal pull-up

	// set debugging pin to be an output (ddr to 1)
	DDRC |= (1 << PC0);

	// set all outputs to zero (low)
	PORTC &= ~(1 << PC0);

	uint8_t prefix[] = { 0xFE };
	uint8_t DISPLAY_ON[] = { 0x42 };
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

		lcd_cursormoveto(0, 3);
		lcd_writestring("EE459 LCD Test");
		lcd_cursormoveto(1, 4);
		lcd_writestring("Spring 2024");
		lcd_cursormoveto(2, 6);
		lcd_writestring("KR GT GB");
		//TODO: Fix error with 3rd row

		_delay_ms(1000);
	}
}