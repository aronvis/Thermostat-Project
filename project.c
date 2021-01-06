/********************************************
 *
 *  Name: Aron Vischjager
 *  Email: vischjag@usc.edu
 *  Section: Wednesday 2pm - 3:20 pm (31292R)
 *  Assignment: Final Project - Thermostat
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "encoder.h"
#include "ds18b20.h"
// Global variable declarations and function headers
#include "project.h"

int main(void) {
	// Initialialize local and global variables
    unsigned char recorded_temp[2];
    unsigned short curr_temp[2] = {0, 0};
    unsigned short prev_temp[2] = {0, 0};
    low = get_low();
    high = get_high();
    temperature = high;

    // Init LCD
    lcd_init();
    lcd_writecommand(1);

    // Write a spash screen to the LCD
    lcd_moveto(0, 2);
    lcd_stringout("EE109 Project");
    lcd_moveto(1, 0);
    lcd_stringout("Aron Vischjager");
    _delay_ms(1000);

    // Setup LED as output
    DDRD |= ((1 << PD2)|(1 << PD3));
   	
    // Init pull resistors for buttons
    PORTC |= ((1 << PC1)|(1 << PC2)); 

    // Init encoder
    encoder_init();

    // Init temperature sensor and return 0 
    // if sensor is not responding
    if (ds_init() == 0) { 
        return(0);
    }
    // Start temperature reading
    ds_convert();
    
    // Print initial screen
    lcd_writecommand(1);
    print_high();

	// Loop forever
    while (1) {                 
		// Turned encoder
        if (encoder_changed) { 
        	// Reset encoder changed flag
		    encoder_changed = 0;  

		    // Update stored value inside low register and print to LCD
		    if(low_state) {
                set_low(low);
		    	print_low();
		    } 
            // Update stored value inside high register and print to LCD
            else {
                set_high(high);
		    	print_high();
		    }
	    }
	    // Change to low/high state
	    if (change_low()){
	    	// Button debouncing
	    	while(change_low()){}
    		low_state = 1;
            temperature = low;
    		print_low();
	    } 
	    // Change to high state
	    else if(change_high()){ 
	   		// Button debouncing
	   		while(change_high()){}
    		low_state = 0;
            temperature = high;
    		print_high();
	   	}
        // Temperature reading is complete
        if(ds_temp(recorded_temp)){
            convert_temp(recorded_temp, curr_temp);
            if(changed_temp(prev_temp, curr_temp)){
                print_room_temp(curr_temp);
            }
            manage_temp(curr_temp);
            ds_convert();
            update_prev_temp(prev_temp, curr_temp);
        }
    }
}

// -------- LCD Functions --------
// Updates the low temperature value on the LCD
void print_low(){
	char buffer[17];
	lcd_moveto(1, 0);
	snprintf(buffer, 17, "Low?%3d High=%3d", low, high);
	lcd_stringout(buffer);
}

// Updates the high temperature value on the LCD
void print_high(){
	char buffer[17];
	lcd_moveto(1, 0);
	snprintf(buffer, 17, "Low=%3d High?%3d", low, high);
	lcd_stringout(buffer);
}

// Updates the room temperature farenheit value on the LCD
void print_room_temp(unsigned short * curr_temp){
    char buffer[17];
    lcd_moveto(0, 0);
    snprintf(buffer, 17, "Temp: %d.%d", curr_temp[1], curr_temp[0]);
    lcd_stringout(buffer);
}

// Converts the values in recorded_temp to farenheit and stores its result in curr_temp
void convert_temp(unsigned char * recorded_temp, unsigned short * curr_temp){
    // Calculation Note:
    // C = scaled_C/16 
    unsigned int scaled_C = recorded_temp[1] << 8;
    scaled_C += recorded_temp[0];

    // Calculation Note:
    // F = 9/5 * C + 32
    // F = 9/5 * scaled_C/16 + 32
    // F = 9/80 * scaled_C + 32
    // scaled_F = 10 * F
    // scaled_F = 9/8 * scaled_C + 320
    unsigned int scaled_F = scaled_C * 9;
    scaled_F /=8;
    scaled_F += 320;
    unsigned int whole_F = scaled_F / 10;
    unsigned int frac_F = scaled_F % 10;
    curr_temp[1] = whole_F;
    curr_temp[0] = frac_F;
}

// Returns true if the temperature has changed compared to the previous reading
unsigned char changed_temp(unsigned short * prev_temp, unsigned short * curr_temp){
    return (prev_temp[0] != curr_temp[0]) || (prev_temp[1] != curr_temp[1]);
}

// Updates the value of prev_temp to curr_temp
void update_prev_temp(unsigned short * prev_temp, unsigned short * curr_temp){
    prev_temp[0] = curr_temp[0];
    prev_temp[1] = curr_temp[1];
}

// -------- Button Functions --------
// Returns true if the low button was pressed
unsigned char change_low(){
	return (PINC & (1 << PC1)) == 0;
}

// Returns true if the high button was pressed
unsigned char change_high(){
	return (PINC & (1 << PC2)) == 0;
}

// -------- LED Functions ----------
// Turns the cooler and heater on and off to make
// sure the room temp is within low and high
void manage_temp(unsigned short * curr_temp){
    unsigned char temp = curr_temp[1];
    // Temp falls below low limit
    if(temp < low && !heating()){
        heat_on();
    } 
    // Temp increases to low limit
    else if(temp >= low && heating()){
        heat_off();
    }
     // Temp decreases to high limit
    else if(temp <= high && cooling()){
        cool_off();
    }
    // Temp goes above high limit
    else if(temp > high && !cooling()){
        cool_on();
    }
}
// Returns true if the AC is on
unsigned char cooling(){
    return (PIND &= (1 << PD3)) == 1;
}

// Returns true if the heater is on
unsigned char heating(){
    return (PIND &= (1 << PD2)) == 1;
}

// Turns on the AC
void cool_on(){
    PIND |= (1 << PD3);
}

// Turns off the AC
void cool_off(){
    PIND &= ~(1 << PD3);
}

// Turns on the heater
void heat_on(){
    PIND |= (1 << PD2);
}

// Turns off the heater
void heat_off(){
    PIND &= ~(1 << PD2);
}
