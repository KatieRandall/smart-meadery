#include "lcd.h"
#include "rht03.h"
#include "rtc.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define STOP_BREW_PIN   PD0
#define TEMP_SENSOR_PIN   PD1
#define ERROR_RST_PIN   PD2
#define ROTARY_A   PC2
#define ROTARY_B   PC1
#define UP_BUTTON   PD4
#define DOWN_BUTTON   PD5
#define LEFT_BUTTON   PD6
#define RIGHT_BUTTON   PD7
#define RED_LED   PB0
#define PHOTOTRAN_PIN   PC3
#define GREEN_LED   PB7
#define ORANGE_LED   PB2
#define RELAY_PIN   PB1

enum LCD_STATES { TEMP, TIME, BUBBLES };
enum LED_STATES { OFF, ON };
enum BREWING_STATES { ERROR, IN_PROGRESS, DONE };

char is_pressed(char bit);
void set_led(char bit, char value);
void determine_initial_rotary_state();
void rotary_inc();
void rotary_dec();

volatile int temp_thresh = 230;
volatile int hours = 0;
volatile int hours_inc = 1;
volatile int lcd_state = TEMP;
volatile unsigned char new_state, old_state;
volatile unsigned char changed;

int main(void)
{
	int lcd_state;
	int brewing_state;
	struct time initial_time;
	struct time current_time;
	int elapsed_hours = 0;
	bool starting = true;

	DDRB |= ((1 << RED_LED) | (1 << ORANGE_LED) | (1 << GREEN_LED));
	PORTD |= ((1 << UP_BUTTON) | (1 << DOWN_BUTTON) | (1 << LEFT_BUTTON) | (1 << RIGHT_BUTTON));
	PORTC |= ((1 << ROTARY_A) | (1 << ROTARY_B));

	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT10 | 1 << PCINT9);
	sei();

	determine_initial_rotary_state();
	lcd_cursormoveto(0, 0);
	lcd_writestring("                    ");
	lcd_cursormoveto(1, 0);
	lcd_writestring("                    ");
	lcd_cursormoveto(2, 0);
	lcd_writestring("                    ");

	while (1)
	{
		if (starting)
		{
			initial_time = get_current_time();
			starting = false;
			brewing_state = IN_PROGRESS;
		}
		if (get_current_time().hour != current_time.hour)
		{
			elapsed_hours++;
		}
		current_time = get_current_time();

		// Check temperature
		unsigned char tdata[5] = { 0 };
		get_temp(tdata);
		uint16_t temp = 0;
		temp = (tdata[2] << 8) | tdata[3];
		if (temp > temp_thresh)
		{
			//fridge on
		}
		else
		{
			//fridge off
		}

		// Generate LCD display
		if (lcd_state == TEMP)
		{
			lcd_cursormoveto(0,0);
			lcd_writestring("Actual: Thresh:    f");
			char temp_print[20];
			snprintf(temp_print, 20, "%2d.%1d  %2d.%1d    c", temp / 10, temp % 10, temp_thresh / 10, temp_thresh % 10);
			lcd_cursormoveto(1, 0);
			lcd_writestring(temp_print);
			if (is_pressed(LEFT_BUTTON))
			{
				lcd_state = TIME;
				lcd_clearscreen();
			}

		}
		else if (lcd_state == TIME)
		{
			lcd_cursormoveto(0, 0);
			lcd_writestring("Started: Elapsed:    ");
			char time_print[20];
			snprintf(time_print, 20, "%02d/%02d/%02d  %2dd %2dh", initial_time.month, initial_time.date, initial_time.year, elapsed_hours/24, elapsed_hours%24);
			lcd_cursormoveto(1, 0);
			lcd_writestring(time_print);
			if (is_pressed(LEFT_BUTTON))
			{
				lcd_state = TEMP;
				lcd_clearscreen();
			}
			if (is_pressed(RIGHT_BUTTON)) {
				lcd_state = BUBBLES;
				lcd_clearscreen();
			}
		}
		else if (lcd_state == BUBBLES)
		{
			lcd_cursormoveto(0, 0);
			lcd_writestring("Bubbles:    ");
			char bubbles_print[20];
			snprintf(bubbles_print, 20, "%2dd %2dh", hours/24, hours%24);
			if (is_pressed(RIGHT_BUTTON)) {
				lcd_state = TIME;
				lcd_clearscreen();
			}
		}

		// Update LEDs
		if (brewing_state == ERROR)
		{
			set_led(RED_LED, ON);
		}
		if (brewing_state == IN_PROGRESS)
		{
			set_led(ORANGE_LED, ON);
			set_led(RED_LED, OFF);

		}
		if (brewing_state == DONE)
		{
			set_led(GREEN_LED, ON);
			set_led(ORANGE_LED, OFF);
		}
	}
	return 0;
}
ISR(PCINT1_vect)
{
	unsigned char bits, a, b;

	bits = PINC;		// Read the two encoder input at the same time
	a = bits & (1 << ROTARY_A);
	b = bits & (1 << ROTARY_B);

	if (old_state == 0) {
		if (a) {
			new_state = 1;
			rotary_inc();
		}
		else if (b) {
			new_state = 2;
			rotary_dec();
		}
	}
	else if (old_state == 1) {
		if (!a) {
			new_state = 0;
			rotary_dec();
		}
		else if (b) {
			new_state = 3;
			rotary_inc();
		}
	}
	else if (old_state == 2) {
		if (a) {
			new_state = 3;
			rotary_dec();
		}
		else if (!b) {
			new_state = 0;
			rotary_inc();
		}
	}
	else {   // old_state = 3
		if (!a) {
			new_state = 2;
			rotary_inc();
		}
		else if (!b) {
			new_state = 1;
			rotary_dec();
		}
	}

	if (new_state != old_state) {
		changed = 1;
		old_state = new_state;
	}
}
char is_pressed(char bit)
{
	if ((PIND & (1 << bit)) == 0) {
		_delay_ms(5);
		while ((PIND & (1 << bit)) == 0)
		{
		}
		_delay_ms(5);
		return 1;
	};
	return 0;
}
void set_led(char bit, char value)
{
	if (value)
	{
		PINB |= (1 << bit);
	}
	else
	{
		PINB &= ~(1 << bit);
	}
}
void determine_initial_rotary_state()
{
	unsigned char bits, a, b;
	bits = PINC;
	a = bits & (1 << ROTARY_A);
	b = bits & (1 << ROTARY_B);

	if (!b && !a)
		old_state = 0;
	else if (!b && a)
		old_state = 1;
	else if (b && !a)
		old_state = 2;
	else
		old_state = 3;

	new_state = old_state;
}
void rotary_inc()
{
	if (lcd_state == TEMP)
	{
		temp_thresh++;
	}
	else if (lcd_state == BUBBLES)
	{
		hours++;
	}
}
void rotary_dec()
{
	if (lcd_state == TEMP)
	{
		temp_thresh--;
	}
	else if (lcd_state == BUBBLES)
	{
		hours--;
	}
}