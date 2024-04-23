#include "lcd.h"
#include "rht03.h"
#include "rtc.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#define STOP_BREW_PIN   PD2
#define TEMP_SENSOR_PIN   PD1
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
#define FRIDGE_INDICATOR PD3

enum LCD_STATES { TEMP, TIME, BUBBLES };
enum LED_STATES { OFF, ON };
enum BREWING_STATES { ERROR, IN_PROGRESS, DONE };

char is_pressed(char bit);
void set_led(char bit, char value);
void determine_initial_rotary_state();
void rotary_inc();
void rotary_dec();
void update_temp();
void update_fridge();
void temp_print();
void time_print();
void bubbles_print();
void reset_device();

// Rotary variables
volatile unsigned char new_state, old_state;
volatile unsigned char changed;

// State variables
volatile int lcd_state = TEMP;
int brewing_state = IN_PROGRESS;
uint8_t starting = 0x01;
bool is_currently_bubble = false;
uint8_t is_fahrenheight = 0x00;
bool is_paused = false;

// Timing variables
struct time initial_time;
struct time current_time;
int elapsed_hours = 0;
int delay_count = 60;

// Temperature variables
uint16_t celsius_temp = 0;
uint16_t current_temp = 0;
volatile int temp_thresh = 230;

// Bubble variables
int bubble_history[400];
int bubbles_this_hour = 0;
uint8_t phototran_sample;
const uint8_t bubble_threshold = 80;
volatile int view_hour = 0;
volatile int hours_inc = 1;
int idx = 16; // index of next bubble data being stored in eeprom

int main(void)
{

	// Initialize pins
	DDRB |= ((1 << RED_LED) | (1 << ORANGE_LED) | (1 << GREEN_LED));
	DDRB |= (1 << RELAY_PIN);
	DDRD |= (1 << FRIDGE_INDICATOR);
	PORTD |= ((1 << UP_BUTTON) | (1 << DOWN_BUTTON) | (1 << LEFT_BUTTON) | (1 << RIGHT_BUTTON) | (1 << STOP_BREW_PIN));
	PORTC |= ((1 << ROTARY_A) | (1 << ROTARY_B));
	PORTD &= ~(1 << PD0);

	// Enable pin change interrupt (rotary encoder)
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT10 | 1 << PCINT9);
	sei();
	determine_initial_rotary_state();

	// Initialize LCD & ADC
	lcd_init();
	adc_init();

	// Display welcome screen
	lcd_clearscreen();
	lcd_cursormoveto(0, 5);
	lcd_writestring("Welcome");
	lcd_cursormoveto(1, 3);
	lcd_writestring("Smart Meadery");
	_delay_ms(1000);
	lcd_clearscreen();
	
	starting = eeprom_read_byte((uint8_t*) 14);
	// Start brew
	if (starting)
	{
		initial_time = get_current_time();
		current_time = initial_time;
		// update eeprom with initial time
		eeprom_update_byte((uint8_t*) 1, initial_time.date);
		eeprom_update_byte((uint8_t*) 2, initial_time.month);
		eeprom_update_word((uint16_t*) 3, initial_time.year);
	}
	else
	{
		// read eeprom initial time in
		initial_time.date = eeprom_read_byte((uint8_t*)1);
		initial_time.month = eeprom_read_byte((uint8_t*)2);
		initial_time.year = eeprom_read_word((uint16_t*)3);

		elapsed_hours = eeprom_read_dword((uint32_t*)6); // eeprom read 
		temp_thresh = eeprom_read_dword((uint32_t*)10); // eeprom read
		is_fahrenheight = eeprom_read_byte((uint8_t*)0); // eeprom read

		int i = 0;
		 for(i = 0; i < elapsed_hours; i++)
		 {
		 	idx = 16 + (sizeof(int) * i); 
		 	bubble_history[i] = eeprom_read_dword((uint32_t*)idx);// eeprom read
		 }
		 //bubble_history = eeprom_read_byte((uint8_t*)16);// eeprom read
		current_time = get_current_time();
	}
	starting = 0x00;
	eeprom_update_byte((uint8_t*) 14, starting);
	_delay_ms(100); 
	eeprom_update_dword((uint32_t*) 6, elapsed_hours);
	eeprom_update_dword((uint32_t*) 10, temp_thresh);
	while (1)
	{

		// Operations to do about once a minute
		if (delay_count >= 60)
		{
			//If new hour
			if (get_current_time().min != current_time.min)
			{
				bubble_history[elapsed_hours] = bubbles_this_hour;
				if (bubbles_this_hour == 0)
				{
					brewing_state = DONE;
				}
				// Update eeprom with elapsed hours and bubble history
				idx = 16 + (sizeof(int) * elapsed_hours); // idx = 16 + 4;
				eeprom_update_dword((uint32_t*)idx, bubbles_this_hour);// eeprom read
				elapsed_hours++;
				eeprom_update_dword((uint32_t*) 6, elapsed_hours);
				bubbles_this_hour = 0;
			}
			current_time = get_current_time();

			// Check temperature
			update_temp();

			// Turn on/off fridge
			update_fridge();

			// Update LCD screen
			if (lcd_state == TEMP) temp_print();
			if (lcd_state == TIME) time_print();
			if (lcd_state == BUBBLES) bubbles_print();

			delay_count = 0;
		}

		// Count bubbles
		phototran_sample = adc_sample(PHOTOTRAN_PIN);
		if (phototran_sample > bubble_threshold)
		{
			is_currently_bubble = true;
		}
		if (is_currently_bubble)
		{
			if (phototran_sample < bubble_threshold)
			{
				is_currently_bubble = false;
				bubbles_this_hour++;
				if (lcd_state == BUBBLES) bubbles_print();
			}
		}

		if (changed)
		{
			update_fridge();
			if (lcd_state == TEMP)
			{ 
				temp_print(); // in this if update temp thresh eeprom 
				eeprom_update_dword((uint32_t*) 10, temp_thresh);
			}
			if (lcd_state == BUBBLES) bubbles_print();
			changed = false;
			
		}

		// Update states
		if (lcd_state == TEMP)
		{
			if (is_pressed(UP_BUTTON))
			{
				if (!is_fahrenheight)
				{
					is_fahrenheight = 0x01;
					starting = eeprom_read_byte((uint8_t*) 0);
					temp_thresh = (temp_thresh * 9) / 5 + 320;
					eeprom_update_dword((uint32_t*) 10, temp_thresh);
					update_temp();
					temp_print();
				}
			}
			else if (is_pressed(DOWN_BUTTON))
			{
				if (is_fahrenheight)
				{
					is_fahrenheight = 0x00;
					starting = eeprom_read_byte((uint8_t*) 0);
					temp_thresh = (temp_thresh - 320) * 5 / 9;
					eeprom_update_dword((uint32_t*) 10, temp_thresh);
					update_temp();
					temp_print();
				}
			}
			else if (is_pressed(RIGHT_BUTTON))
			{
				lcd_state = TIME;
				time_print();
			}

		}
		else if (lcd_state == TIME)
		{
			if (is_pressed(LEFT_BUTTON))
			{
				lcd_state = TEMP;
				temp_print();
			}
			else if (is_pressed(RIGHT_BUTTON)) {
				lcd_state = BUBBLES;
				bubbles_print();
			}
		}
		else if (lcd_state == BUBBLES)
		{
			if (is_pressed(UP_BUTTON)) {
				hours_inc++;
				bubbles_print();
			}
			else if (is_pressed(DOWN_BUTTON)) {
				if (hours_inc > 0)
				{
					hours_inc--;
				}
				bubbles_print();
			}
			else if (is_pressed(LEFT_BUTTON)) {
				lcd_state = TIME;
				time_print();
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

		if ((PIND & (1 << STOP_BREW_PIN)) == 0)
		{
			_delay_ms(5);
			int i = 0;
			while ((PIND & (1 << STOP_BREW_PIN)) == 0)
			{
				_delay_ms(100);
				i++;
			}
			if (i >= 50)
			{
				lcd_clearscreen();
				lcd_cursormoveto(0, 0);
				lcd_writestring("Starting new brew");
				_delay_ms(750);
				starting = 0x01;
				eeprom_update_byte((uint8_t*) 14, starting);
				_delay_ms(100);
				reset_device();
			}
			else
			{
				lcd_clearscreen();
				lcd_cursormoveto(0, 0);
				lcd_writestring("Saving brew");
				_delay_ms(750);
				starting = 0x00;
				eeprom_update_byte((uint8_t*) 14, starting);
				_delay_ms(100);
				reset_device();
			}
			_delay_ms(5);
		}
		// Delay .1s
		delay_count++;
		_delay_ms(100);
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
		PORTB |= (1 << bit);
	}
	else
	{
		PORTB &= ~(1 << bit);
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
	if (lcd_state == BUBBLES)
	{
		view_hour += hours_inc;
	}
}
void rotary_dec()
{
	if (lcd_state == TEMP)
	{
		temp_thresh--;
	}
	if (lcd_state == BUBBLES)
	{
		view_hour -= hours_inc;
		if (view_hour < 0)
		{
			view_hour = 0;
		}
	}
}
void update_temp()
{
	unsigned char temp_data[5] = { 0 };
	get_temp(temp_data);
	celsius_temp = 0;
	celsius_temp = (temp_data[2] << 8) | temp_data[3];
	if (is_fahrenheight)
	{
		current_temp = ((celsius_temp * 9)/5) + 320;
	}
	else
	{
		current_temp = celsius_temp;
	}
}
void update_fridge()
{
	if (current_temp > temp_thresh)
	{
		PORTB |= (1 << RELAY_PIN);
		PORTD |= (1 << FRIDGE_INDICATOR);
	}
	else
	{
		PORTB &= ~(1 << RELAY_PIN);
		PORTD &= ~(1 << FRIDGE_INDICATOR);

	}
}
void temp_print()
{
	lcd_cursormoveto(0, 0);
	lcd_writestring("Actual:  Thresh:   f");
	char temp_print[21];
	snprintf(temp_print, 21, "%2d.%1d     %2d.%1d      c", current_temp / 10, current_temp % 10, temp_thresh / 10, temp_thresh % 10);
	lcd_cursormoveto(1, 0);
	lcd_writestring(temp_print);
	lcd_cursormoveto(3, 0);
	lcd_writestring(">TEMP  TIME  BUBBLES");
	if (is_fahrenheight)
	{
		lcd_cursormoveto(0, 18);
		lcd_writestring(">");
	}
	else
	{
		lcd_cursormoveto(1, 18);
		lcd_writestring(">");
	}
}
void time_print()
{
	lcd_cursormoveto(0, 0);
	lcd_writestring("Started:   Elapsed:  ");
	char time_print[21];
	snprintf(time_print, 21, "%02d/%02d/%02d  %2dd %2dh   ", initial_time.month, initial_time.date, initial_time.year - 2000U, elapsed_hours / 24, elapsed_hours % 24);
	lcd_cursormoveto(1, 0);
	lcd_writestring(time_print);
	lcd_cursormoveto(3, 0);
	lcd_writestring(" TEMP >TIME  BUBBLES");
}
void bubbles_print()
{
	lcd_cursormoveto(0, 0);
	char bubbles_print_0[21];
	snprintf(bubbles_print_0, 21, "Curr: @%2dd%2dh   inc: ", view_hour / 24, view_hour % 24);
	lcd_writestring(bubbles_print_0);
	lcd_cursormoveto(1, 0);
	char bubbles_print_1[21];
	snprintf(bubbles_print_1, 21, "%4d  %4d      %2d", bubbles_this_hour, bubble_history[view_hour], hours_inc);
	lcd_writestring(bubbles_print_1);
	lcd_cursormoveto(3, 0);
	lcd_writestring(" TEMP  TIME >BUBBLES");
}
void reset_device()
{
	DDRD |= (1 << PD0);
	PORTD &= ~(1 << PD0);
	_delay_us(3);
	PORTD |= (1 << PD0);
}