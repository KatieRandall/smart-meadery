#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>


#define RELAY_PIN   PC7   


void initRelayPin() {
    DDRD |= (1 << RELAY_PIN);
}


void turnOnRelay() {
    PORTD |= (1 << RELAY_PIN);
}


void turnOffRelay() {
    PORTD &= ~(1 << RELAY_PIN);
}

// Function for temp sensor
//Needs to be adjusted
int readTemperature() {
    return 25; 
}

int main(void) {
    initRelayPin();

    while (1) {
        int temperature = readTemperature();

        if (temperature > 30) {
            turnOnRelay();
        } else {
            turnOffRelay();
        }
        _delay_ms(1000); 
    }
    return 0;
}