#include "rtc.h"
#include "twi_master.h"

#define DS1307_ADDRESS 0x68
#define READ_ADDRESS 0xD1
#define WRITE_ADDRESS 0xD0
#define DS1307_CONTROL 0x07 ///< Control register
#define DS1307_NVRAM 0x08   ///< Start of RAM registers - 56 bytes, 0x08 to 0x3f

struct time get_current_time()
{
	tw_init(TW_FREQ_50K, false); // set I2C Frequency, enable internal pull-up
	bool repeat_start = false;
	uint8_t read_bit[1];
	struct time ret_val;
	read_bit[0] = 0;
	uint8_t buffer[7];

	tw_master_transmit(DS1307_ADDRESS, read_bit, sizeof(read_bit), repeat_start);
	tw_master_receive(DS1307_ADDRESS, buffer, sizeof(buffer));

	ret_val.year = ((buffer[6] >> 4) * 10) + (buffer[6] & 0x0F) + 2000U;
	ret_val.month = ((buffer[5] >> 4) * 10) + (buffer[5] & 0x0F);
	ret_val.date = ((buffer[4] >> 4) * 10) + (buffer[4] & 0x0F);
	ret_val.hour = (((buffer[2] & 0b00110000) >> 4) * 10) + (buffer[2] & 0x0F);
	ret_val.min = ((buffer[1] >> 4) * 10) + (buffer[1] & 0x0F);
	ret_val.sec = (((buffer[0] & 0b01110000) >> 4) * 10) + (buffer[0] & 0x0F);

	return ret_val;
}