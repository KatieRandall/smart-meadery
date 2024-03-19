#include "lcd.h"
#include "twi_master.h"
#include <avr/io.h>

#define ADDRESS	0x28

void lcd_writechar(unsigned char c)
{
	uint8_t letter[] = { c };
	tw_master_transmit(ADDRESS, letter, sizeof(letter), false);
}
void lcd_writestring(char* str)
{
	int i = 0;
	while (str[i] != '\0') {
		lcd_writechar(str[i]);
		i++;
	}
}
void lcd_cursormoveto(unsigned char row, unsigned char col)
{
	uint8_t prefix[] = { 0xFE };
	uint8_t SET_CURSOR[] = { 0x45 };
	uint8_t basePosition;

	switch (row) {
	case 0:
		basePosition = 0; // Row 0: 0-19
		break;
	case 1:
		basePosition = 40; // Row 1: 40-59
		break;
	case 2:
		basePosition = 20; // Row 2: 20-39
		break;
	case 3:
		basePosition = 60; // Row 3: 60-79
		break;
	}

	uint8_t position = basePosition + col;
	uint8_t cursor_pos[] = { position };

	tw_master_transmit(ADDRESS, prefix, sizeof(prefix), false);
	tw_master_transmit(ADDRESS, SET_CURSOR, sizeof(SET_CURSOR), false);
	tw_master_transmit(ADDRESS, cursor_pos, sizeof(cursor_pos), false);
}