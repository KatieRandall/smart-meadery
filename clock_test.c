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

void set_time();
int main(void)
{
	//while (1)
	//{
	//	// set debugging pin to be an output (ddr to 1)
	//	DDRC |= (1 << PC0);

	//	// set all outputs to zero (low)
	//	PORTC &= ~(1 << PC0);

	//	_delay_ms(1000);

	//	PORTC |= (1 << PC0);

	//	_delay_ms(1000);


	//}
	tw_init(TW_FREQ_50K, false); // set I2C Frequency, enable internal pull-up
	lcd_clearscreen();
	bool repeat_start = false;
	uint8_t read_bit[1];
	read_bit[0] = 0;
	//set_time();
	while (1)
	{
		read_bit[0] = 0;
		uint8_t buffer[7];
		
		tw_master_transmit(DS1307_ADDRESS, read_bit, sizeof(read_bit), repeat_start);
		tw_master_receive(DS1307_ADDRESS, buffer, sizeof(buffer));

		uint16_t year = ((buffer[6] >> 4) * 10) + (buffer[6] & 0x0F) + 2000U;
		uint8_t month = ((buffer[5] >> 4) * 10) + (buffer[5] & 0x0F);
		uint8_t date = ((buffer[4] >> 4) * 10) + (buffer[4] & 0x0F);
		uint8_t hour = (((buffer[2] & 0b00110000) >> 4) * 10) + (buffer[2] & 0x0F);
		uint8_t min = ((buffer[1] >> 4) * 10) + (buffer[1] & 0x0F);
		uint8_t sec = (((buffer[0] & 0b01110000) >> 4) * 10) + (buffer[0] & 0x0F); //Sec calc not correct atm

		char buf[20];
		snprintf(buf, 17, "%02d/%02d/%02d", month, date, year);

		char buf2[20];
		snprintf(buf2, 17, "%02d:%02d:%02d", hour, min, sec);

		lcd_cursormoveto(0, 0);
		lcd_writestring("EE459 Clock Test");
		lcd_cursormoveto(1, 0);
		lcd_writestring(buf);
		lcd_cursormoveto(2, 0);
		lcd_writestring(buf2);

		_delay_ms(1000);
	}
}
void set_time()
{
	bool repeat_start = false;
	lcd_clearscreen();
	uint8_t read_bit[1];
	read_bit[0] = 0;

	//Secs
	uint8_t write_buffer[2];
	write_buffer[0] = 0x00;
	write_buffer[1] = 0x00;
	tw_master_transmit(DS1307_ADDRESS, write_buffer, sizeof(write_buffer), repeat_start);


	//Mins
	write_buffer[0] = 0x01;
	write_buffer[1] = 0x12;
	tw_master_transmit(DS1307_ADDRESS, write_buffer, sizeof(write_buffer), repeat_start);


	//Hours
	write_buffer[0] = 0x02;
	write_buffer[1] = 0x22;
	tw_master_transmit(DS1307_ADDRESS, write_buffer, sizeof(write_buffer), repeat_start);

	//Day
	write_buffer[0] = 0x03;
	write_buffer[1] = 0x02;
	tw_master_transmit(DS1307_ADDRESS, write_buffer, sizeof(write_buffer), repeat_start);


	//Date
	write_buffer[0] = 0x04;
	write_buffer[1] = 0x16;
	tw_master_transmit(DS1307_ADDRESS, write_buffer, sizeof(write_buffer), repeat_start);


	//Month
	write_buffer[0] = 0x05;
	write_buffer[1] = 0x04;
	tw_master_transmit(DS1307_ADDRESS, write_buffer, sizeof(write_buffer), repeat_start);

	//year
	write_buffer[0] = 0x06;
	write_buffer[1] = 0x24;
	tw_master_transmit(DS1307_ADDRESS, write_buffer, sizeof(write_buffer), repeat_start);
}