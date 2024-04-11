#include <avr/io.h>
#include <util/delay.h>


#define RELAY_PIN   7   // Assuming Pin 7 is used for controlling the relay


void initRelayPin() {
    // Set RELAY_PIN as output
    DDRD |= (1 << RELAY_PIN);
}

// Function to turn on the relay
void turnOnRelay() {
    PORTD |= (1 << RELAY_PIN);
}

// Function to turn off the relay
void turnOffRelay() {
    PORTD &= ~(1 << RELAY_PIN);
}

int main(void) {
    initRelayPin();

    while (1) {
        turnOnRelay();
        _delay_ms(1000); 

        turnOffRelay();
        _delay_ms(1000); 
    }
    return 0;
}