#include "twi_master.h"
#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

#define LCD_ADDR	0x28

int main(void)
{

	lcd_init();
	lcd_clearscreen();
	lcd_cursormoveto(0, 3);
	lcd_writestring("EE459 LCD Test");
	lcd_cursormoveto(1, 4);
	lcd_writestring("Spring 2024");
	lcd_cursormoveto(2, 6);
	lcd_writestring("KR GB GT");
	lcd_cursormoveto(3, 3);
	lcd_writestring("test");

}