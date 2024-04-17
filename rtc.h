#ifndef RTC_H_
#define RTC_H_
#include <stdint.h>

struct time {
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
};

struct time get_current_time();

#endif /* LCD_H_ */