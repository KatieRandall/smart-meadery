#include "twi_master.h"
#include "lcd.h"
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define DS1307_ADDRESS 0x68
#define READ_ADDRESS 0xD1
#define WRITE_ADDRESS 0xD0
#define DS1307_CONTROL 0x07 ///< Control register
#define DS1307_NVRAM 0x08   ///< Start of RAM registers - 56 bytes, 0x08 to 0x3f


int main(void)
{

	tw_init(TW_FREQ_50K, false); // set I2C Frequency, enable internal pull-up
	bool repeat_start = false;
	while (1)
	{
		lcd_clearscreen();
		uint8_t read_bit[1];
		uint8_t year_address[] = { 0x06 };
		read_bit[0] = 0;

		uint8_t write_buffer[1];
		write_buffer[0] = 0b00010100;

		tw_master_transmit(WRITE_ADDRESS, year_address, sizeof(read_bit), repeat_start);
		tw_master_transmit(DS1307_ADDRESS, write_buffer, sizeof(write_buffer), repeat_start);


		read_bit[0] = 0;
		uint8_t buffer[7];
		
		tw_master_transmit(DS1307_ADDRESS, read_bit, sizeof(read_bit), repeat_start);
		tw_master_receive(DS1307_ADDRESS, buffer, sizeof(buffer));

		/*DateTime data = DateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5]),
			bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
			bcd2bin(buffer[0] & 0x7F));*/
		uint16_t year = buffer[6] & 0x0F;
		uint8_t month = buffer[5] & 0x0F;
		uint8_t day = buffer[4] & 0x0F;
		uint8_t hour = buffer[2] & 0x0F;
		uint8_t min = buffer[1] & 0x0F;
		uint8_t sec = buffer[0] & 0x0F;

		char buf[20];
		snprintf(buf, 17, "%02d/%02d/%02d", day, month, year);

		lcd_cursormoveto(0, 3);
		lcd_writestring("EE459 LCD Test");
		lcd_cursormoveto(1, 0);
		lcd_writestring(buf);

		_delay_ms(60000);
	}
}