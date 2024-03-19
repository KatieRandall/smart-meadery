#ifndef LCD_H_
#define LCD_H_

#include <avr/io.h>
#include <util/twi.h>
#include <stdbool.h>

void lcd_writechar(unsigned char);
void lcd_writestring(char*);
void lcd_cursormoveto(unsigned char, unsigned char);

#endif /* LCD_H_ */