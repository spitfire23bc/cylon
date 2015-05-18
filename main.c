/*
 * main.c
 *
 *	TODO - figure out PWM
 *	TODO - increase larson LED count to 11(?). Charlieplexing.
 *	TODO - add code for flickering thrusters
 *	TODO - add code for static missile bay lights (may as well drive off the 4313)
 *
 */
/*===========================================================================*/

#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>		// TODO - remove this once delays are done with interrupts

/*===========================================================================*/

// Begin left-to-right
// Declare as static so larson() knows which way to shift the register
static int ltr = 1;			


/*===========================================================================*/

void main(void) {

	// Initialise PortD as output
	DDRD = 0xFF;
	// Start with the middle LED lit
	PORTD = 0x08;
	
	// Wait for dramatic effect
	// TODO - powerup gradual fade-in rather than instant-on
	_delay_ms(500);
	
	
	// Main loop
	while(1) {


		// Delay between LED changes
		// TODO - implement the delay with interrupts
		_delay_ms(100);
		
		// Update the PORTD register to shift the lit LED
		larson();
		
	}		
}

/*===========================================================================*/

void larson(void) {
// TODO - rewrite this to shift a PWM pattern

	if (ltr==1) 					// Scan left to right
	{
		if (PORTD<64)
		{
			PORTD = (PORTD << 1);	// Left-shift to light next LED
		}
		else				 		// Right-most position		
		{
			ltr = 0;				// Set the direction to right-to-left
		}
	}
	else							// Scan right to left 
	{
		if (PORTD>1) 
		{
			PORTD = (PORTD >> 1);	// Right-shift to light next LED
		}
		else 						// Left-most position
		{
			ltr = 1;				// Set the direction to left-to-right
		}
	}

}
