/*
    Testing Code for the RHT03 (DHT22) Humidity and Temperature Sensor
  
    Apapted from DS18B20 Temperature Sensor Code

    WARNING: These routines assume you are using Port D, bit 1
    to communicate with the DS18B20.  If you use a different I/O bit,
    the routines below will have to be changed.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "rht03.h"
#include "lcd.h"

char ds_reset(void);
void ds_write1bit(void);
void ds_write0bit(void);
unsigned char ds_readbit(void);
void ds_writebyte(unsigned char);
unsigned char ds_readbyte(void);
void get_temp(unsigned char *tdata);

/* Grace's Ideas.... 
    ds_init(): let bus go high (ddr to zero) and initialize port to 0
    // start_transfer(): 1 function call to pull bus high then low and then wait for sensor to pull bus low 
    // read_data(): 1 function call to read all 5 bits (call ds_readbyte() 5 times)
    get_temp(): function that starts it all and called start_transfer() adn read_data()

    We make the 1-Wire bus high by setting port bit for input
    and the external pull-up resistor will make it go high.
    DDR bit = 0, PORT bit = 0

    We make the 1-Wire bus low by setting port bit for output,
    and with a zero in the PORT bit it will go low.
    DDR bit = 1, PORT bit = 0
*/ 


/* ds_init - Initialize the port bit used for the 1-Wire bus. */
char ds_init(void)
{
    DDRD &= ~(1<<PD1); // set bus to high 
    PORTD &= ~(1 <<PD1); // set output on port to zero 

    // when you want to pull bus low, set DDRD to output (output val is automatically 0)
    // when you want to pull bus high or to rest state, set DDRD to input (base case is 1 bc of pull up)

    return(1);
}

/*
  get_temp: start temp conversion, wait for sensor to pull bus low then read in data
*/
void get_temp(unsigned char *tdata)
{

    // Pull bus low for 1 - 10 ms 
    DDRD |= (1 << PD1);         // Pull bus low
    _delay_ms(5);               // delay 1-10 ms 
    DDRD &= ~(1 << PD1);        // Let bus go high

    // let bus go high until sensor pulls down 
    _delay_us(5);              // wait 20-40 us for sensor to pull bus low
    while((PIND & (1<<PD1))) { // // wait while PD1 is high; wait here until low
    };   

    // sensor pulls bus low for 80 us to signify that the signal was recieved
    _delay_us(5);
    while((PIND & (1<<PD1)) == 0) {   };   // // sensor pulls PD1 low; wait here while low; code is here ~75 times
    
    // char buf1[20];
    // lcd_cursormoveto(1,0);
    // snprintf(buf1, 20, "cnt: %d", count);
    // lcd_writestring(buf1);
    
    
    //_delay_us(80); // Data transmission should start 80 us from when bus goes back to high (+10 us as a buffer)

    // READ DATA
    char buftemp[40];
    for(int i = 0; i < 40; i++)
    {
        buftemp[i] = ds_readbit(); 
    }

    // for(int i = 0; i < 40; i++)
    // {
    //     char print1[2];
    //     snprintf(print1,2,"%01d", buftemp[i]);
    //     lcd_cursormoveto(i/20, i%20);
	//     lcd_writestring(print1);
    // }

    for(int buf = 0; buf < 5; buf++)
    {
        for(int i = 0; i < 8; i++)
        {
            int offset = 8*buf;
            int idx_buf = offset + i;
            tdata[buf] |= (buftemp[idx_buf] << (7-i));
        }
    }

    // while(1){};

    // tdata[0] = ds_readbyte();   // Read the first byte (LSB) - SUM
    // tdata[1] = ds_readbyte();   // Read the second byte  - temp data decimal
    // tdata[2] = ds_readbyte();   // Read the third byte - temp data whole number
    // tdata[3] = ds_readbyte();   // Read the fourth byte - humidity data decimal
    // tdata[4] = ds_readbyte();   // Read the fifth byte (MSB) - humidity data whole number

    // DISPLAY
    int humid = (((tdata[0])<<8) | tdata[1]); // make humidity reading into one int
    int temp_c = (((tdata[2])<<8) | tdata[3]); // make temperature reading into one int
    char buf0[20];
    char buf1[20];
    char buf2[20];
    snprintf(buf0, 20, "H: %d.%d T: %d.%d", humid/10, humid%10, temp_c/10, temp_c%10);
	snprintf(buf1, 20, "0: %d 1: %d", tdata[0], tdata[1]);
	snprintf(buf2, 20, "2: %d 3: %d 4: %d", tdata[2], tdata[3], tdata[4]);
	lcd_cursormoveto(0, 0);
	lcd_writestring(buf0);
    lcd_cursormoveto(1, 0);
	lcd_writestring(buf1);
	lcd_cursormoveto(2, 0);
	lcd_writestring(buf2);   
    _delay_ms(1000);
}


// /*
//   ds_writebyte - Write an 8-bit byte out the bus
// */

// void ds_writebyte(unsigned char x)
// {
//     unsigned char i;
//     i = 8;
//     while (i != 0) {
//         if (x & 1)              // Check the LSB
//             ds_write1bit();     // Send a one
//         else
//             ds_write0bit();     // Send a zero
//         x >>= 1;                // Shift x over towards the LSB
//         i--;
//     }
// }

/*
  ds_readbyte - Read an 8-bit byte from some device on the bus
*/
unsigned char ds_readbyte()
{
    unsigned char x, m, i;

    x = 0;
    m = 1;                      // m = mask for sticking bits in x
    i = 8;

    while (i != 0) {
        if (ds_readbit())       // Get a bit
            x |= m;             // If a one, put a one in x
        m <<= 1;                // Shift the mask towards the MSB
        i--;
    }
    return(x);

}

/*
  ds_write1bit - Write a single 1 bit out the bus
*/
// void ds_write1bit(void)
// {
//     // Add code to write a 1 to the remote device
//     // See page 15 and 16 of DS128B20 datasheet
//     DDRD |= (1 << PD1);         // Pull bus low
//     _delay_us(2); 
//     DDRD &= ~(1 << PD1);        // Let bus go high
//     _delay_us(60);
// }

/*
  ds_write0bit - Write a single 0 bit out the bus
*/
// void ds_write0bit(void)
// {
//     // Add code to write a 0 to the remote device
//     // See page 15 and 16 of DS128B20 datasheet
//     DDRD |= (1 << PD1);         // Pull bus low
//     _delay_us(60);
//     DDRD &= ~(1 << PD1);        // Let bus go high
//     _delay_us(2);
// }

/*
  ds_readbit - Read a single bit from the bus.  
    if bit is a 0, PD1 is pulled high for 26-28 us 
    if bit is a 1, PD1 is pulled high for 70 us
*/
unsigned char ds_readbit(void)
{
    //int val = 0;  

    // BETWEEN BYTE DELAY
    _delay_us(5);
    while(PIND & (1<<PD1)) {}; // wait while bus is high until it goes low indicating the next bit will soon be sent 
    
    // wait while PD1 is pulled low; indicates we are between bytes
    _delay_us(5);
    while((PIND & (1<<PD1)) == 0) // stay here while signal is pulled low; here ~3 times
    {} 

    /******/
    // int i = 0;
    // while(PIND & (1<<PD1))
    // {
    //     i++;
    // }

    /******/

    // char buf1[20];
    // lcd_cursormoveto(1,0);
    // snprintf(buf1, 20, "cnt: %d", count);
    // lcd_writestring(buf1);

    // BYTE DELAY + CHECK VALUE
    // check if bit is high after 50 us (should be low if bit is a zero, high if bit is a one)
    _delay_us(40);
   // return (PIND & (1<<PD1));
    if(PIND & (1<<PD1))
    {
        return 1;
    }
    else{
        return 0;
    }

   // return (i > 30);

}
