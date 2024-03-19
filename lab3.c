/*************************************************************
* Lab 3: Rotating through all the IO Ports
*************************************************************/

#include <avr/io.h>
#include <util/delay.h> 

int main(void)
{
	// set pins to be outputs (ddr to 1)
	DDRC |= (1 << PC0);

	// set all outputs to zero (low)
	PORTC &= ~(1 << PC0);

	while (1) {
		// set pins to high 
		PORTC |= (1 << PC0);

		// delay 
		_delay_ms(500);

		// set pins to low
		PORTC &= ~(1 << PC0);

		// delay 
		_delay_ms(100);
	}

	return 0;   /* never reached */
}