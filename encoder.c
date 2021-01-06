#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "encoder.h"

// Global variables for encoder declared in project.c 
// Flag for state change
extern volatile unsigned char encoder_changed; 
extern volatile unsigned char new_state, old_state;
extern volatile unsigned char a, b;
extern volatile unsigned char inputBits;
extern volatile int temperature; 
extern volatile unsigned char low_state;
extern volatile unsigned char low;
extern volatile unsigned char high;


// ------------ Temperature Functions -----------
// Returns the stored low temperature or 68 degrees otherwise
unsigned char get_low(){ 
	unsigned short lowAddr = 0;
	unsigned char low = eeprom_read_byte((void *) lowAddr); 
	// No data stored
	if(low == 255){
		init_low(68);
		low = 68;
	}
	return low;
}

// Returns the stored high temperature or 72 degrees otherwise
unsigned char get_high(){
	unsigned short highAddr = 1;
	unsigned char high = eeprom_read_byte((void *) highAddr);
	// No data stored
	if(high == 255){
		init_high(72);
		high = 72;
	}
	return high;

}

// Updates the stored low temperature
void set_low(unsigned char temperature){
	unsigned short lowAddr = 0;
	eeprom_update_byte((void *) lowAddr, temperature);

}

// Updates the stored high temperature
void set_high(unsigned char temperature){
	unsigned short highAddr = 1;
	eeprom_update_byte((void *) highAddr, temperature);
}

// Stores default low threshold value 68 into low temperature
void init_low(unsigned char temperature){
	set_low(temperature);
}

// Stores default high threshold value 68 into low temperature
void init_high(unsigned char temperature){
	set_high(temperature);
}

// ------------ Encoder Functions -----------
// Intializes encoder registers to enable interrupts 
void encoder_init(){
	// Init interrupt registers and enable global interrupts
    PCICR |= (1 << PCIE0);
    PCMSK0 |= ((1 << PCINT3)|(1 << PCINT4));
    sei();

    // Init pull resistor for rotary encoder
    PORTB |= ((1 << PB3)|(1 << PB4));

    // Sets initial state of encoder
    set_encoder_state();
}

// Read the A and B inputs to determine the intial state
void set_encoder_state(){
    inputBits = PINB & ((1 << PB3)|(1 << PB4));
    a = inputBits & (1 << PB3);
    b = inputBits & (1 << PB4);

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

/*
  Interupts main program when encoder gets moved by the user
  Updates the state and temperature level variable during interupt 
*/
ISR(PCINT0_vect) {
	// Track previous temperature
    int prev_temperature = temperature;

    // ------- Encoder state machine ---------- 
	// Read the input bits and determine A and B
	inputBits = PINB & ((1 << PB3)|(1 << PB4));
    a = inputBits & (1 << PB3);
    b = inputBits & (1 << PB4);

	// For each state, examine the two input bits to see if state
	// has changed, and if so set "new_state" to the new state,
	// and adjust the temperature value.
	if (old_state == 0) {
	    // Handle A and B inputs for state 0
	    if (!b && a) {
	    	new_state = 1;
	    	temperature += 1;
	    } else if (b && !a) {
	    	new_state = 2;
	    	temperature -= 1;
	    }
	}
	else if (old_state == 1) {
	    // Handle A and B inputs for state 1
	    if (!b && !a) {
	    	new_state = 0;
	    	temperature -= 1;
	    } else if (a && b) {
	    	new_state = 3;
	    	temperature += 1;
	    }
	}
	else if (old_state == 2) {
	    // Handle A and B inputs for state 2
	    if (!b && !a) {
	    	new_state = 0;
	    	temperature += 1;
	    } else if (a && b) {
	    	new_state = 3;
	    	temperature -= 1;
	    }
	}
	else {   // old_state = 3
	    // Handle A and B inputs for state 3
	    if (!b && a) {
	    	new_state = 1;
	    	temperature -= 1;
	    } else if (b && !a) {
	    	new_state = 2;
	    	temperature += 1;
	    }
	}

	// ------------ Update low/high temperature --------
	// Update low or high temperature if the value is valid
	// revert the new estate and the temperature otherwise
	if(prev_temperature != temperature){
		// Low temperature state
		if(low_state){
			// Invalid temperature change
			if(temperature < 40 || temperature > high){
				temperature = prev_temperature;
			} 
			// Valid temperature change
			else {
				low = temperature;
			}
		} 
		// High temperature state
		else {
			// Invalid temperature change
			if (temperature > 100 || temperature < low){
				temperature = prev_temperature;
			}
			// Valid 
			else {
				high = temperature;
			}
		}
	}

	// ------------- Update changed variable ----------
	// If state changed, update the value of old_state,
	// and set a flag that the encoder state has changed.
	if (new_state != old_state) {
	    old_state = new_state;
	    // Temperature change did not get reverted
	    if (prev_temperature != temperature){
	    	encoder_changed = 1;
	    }
	}
}
