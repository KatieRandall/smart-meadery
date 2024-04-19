#include <avr/io.h>
#include <util/delay.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include "lcd.h"
#include "adc.h"

enum states {LIQUID, BUBBLE};
int main(void)
{
	DDRC &= ~(1 << PC3);
	//PORTC |= (1 << PC3);
	DDRC &= ~(1 << PC0);
	lcd_init();
	adc_init();
	lcd_clearscreen();
	lcd_cursormoveto(0, 0);
	lcd_writestring("Bubble Test");
	_delay_ms(500);
	lcd_clearscreen();
	uint8_t last_sample;
	uint8_t curr_sample;
	int bubble_count = 0;
	int state = LIQUID;
	uint8_t threshold = 50;
	bool bubble = false;
	while (1)
	{
		/*
		if (state == LIQUID)
		{
			if ((PINC & (1 << PC3)) != 0)
			{
				state = BUBBLE;
			}
		}
		else
		{
			while ((PINC & (1 << PC3)) != 0) {}
			bubble_count++;
			state = LIQUID;
			char test[21];
			snprintf(test, 21, "%3d", bubble_count);
			lcd_cursormoveto(0, 0);
			lcd_writestring(test);
		}
		*/
		/*
		char test[21];
		snprintf(test, 21, "%3d", (PINC & (1 << PC0)));
		lcd_cursormoveto(0, 0);
		lcd_writestring(test);
		_delay_ms(100);*/

		//uint8_t curr_sample = adc_sample(3);
		//if (curr_sample != last_sample)
		//{
		//	if (curr_sample > threshold)
		//	{
		//		bubble = true;
		//	}
		//	if (bubble)
		//	{
		//		if (curr_sample < threshold)
		//		{
		//			bubble = false;
		//			bubble_count++;
		//		}
		//	}
		//	char test[21];
		//	snprintf(test, 21, "%3d", bubble_count);
		//	lcd_cursormoveto(0, 0);
		//	lcd_writestring(test);
		//}
		//last_sample = curr_sample;
		//_delay_ms(200);

		uint8_t curr_sample = adc_sample(3);
			if (curr_sample > threshold)
			{
				bubble = true;
			}
			if (bubble)
			{
				if (curr_sample < threshold)
				{
					bubble = false;
					bubble_count++;
				}
			}
			char test[21];
			snprintf(test, 21, "%3d", curr_sample);
			lcd_cursormoveto(0, 0);
			lcd_writestring(test);
		_delay_ms(200);
	}
}