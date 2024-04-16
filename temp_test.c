#include "twi_master.h"
#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>
#include "rht03.h"


#define LCD_ADDR	0x28

int main(void)
{

	tw_init(TW_FREQ_50K, false); // set I2C Frequency, enable internal pull-up

	// temperature init
	ds_init(); 

	// initRelayPin();

//     while (1) {
//         int temperature = readTemperature();

//         if (temperature > 30) {
//             turnOnRelay();
//         } else {
//             turnOffRelay();
//         }
//         _delay_ms(1000); 
//     }
//     return 0;

	// Initialize array t which will store 5 bytes of temp sensor outputs 
	unsigned char tdata[5];

	// set led debugging pin to be an output (ddr to 1)
	DDRC |= (1 << PC0);

	// set button to be an input and enable pull up (ddr to 0)
	DDRD &= ~(1 << PD2);
	PORTD |= (1 << PD2);

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

	tw_master_transmit(LCD_ADDR, prefix, sizeof(prefix), repeat_start);
	tw_master_transmit(LCD_ADDR, CURSER_HOME, sizeof(CURSER_HOME), repeat_start);

	tw_master_transmit(LCD_ADDR, prefix, sizeof(prefix), repeat_start);
	tw_master_transmit(LCD_ADDR, CLEAR_SCREEN, sizeof(CLEAR_SCREEN), repeat_start);

	tw_master_transmit(LCD_ADDR, prefix, sizeof(prefix), repeat_start);
	tw_master_transmit(LCD_ADDR, CURSER_HOME, sizeof(CURSER_HOME), repeat_start);

	// Splash Screen
	// lcd_clearscreen();
	lcd_cursormoveto(0, 2);
	lcd_writestring("Temp Test Code");
	// lcd_cursormoveto(1, 4);
	// lcd_writestring("Spring 2024  abc");
	// lcd_cursormoveto(2, 6);
	// lcd_writestring("KR GB GT");

	_delay_ms(2000);
	// lcd_clearscreen();

	// get_temp(tdata);

	while (1)
	{
		if((PIND & (1<<PD2)) == 0) // if button is pressed, start temperature reading!
		{
			tw_master_transmit(LCD_ADDR, prefix, sizeof(prefix), repeat_start);
			tw_master_transmit(LCD_ADDR, CLEAR_SCREEN, sizeof(CLEAR_SCREEN), repeat_start);
			// lcd_cursormoveto(0, 2);
			// lcd_writestring("Temp Test Code");

			PORTC |= (1 <<PC0); // led on 
			_delay_us(100);
			while((PIND & (1<<PD2)) == 0) {} // debounce button
			get_temp(tdata);
			PORTC &= ~(1 << PC0); // led off 

			// char buf1[20];
			// char buf2[20];
			// snprintf(buf1, 20, "t[0]: %d   t[1]: %d", tdata[0], tdata[1]);
			// snprintf(buf2, 20, "t[2]: %d   t[3]: %d", tdata[2], tdata[3]);
			// lcd_cursormoveto(1, 0);
			// lcd_writestring(buf1);
			// lcd_cursormoveto(2, 0);
			// lcd_writestring(buf2);
			// _delay_ms(100);
			
		}	
	}
}