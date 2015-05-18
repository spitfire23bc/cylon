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


int main(void) {

	powerup_2();
	
	
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

// Second attempt at a powerup routine. This one uses an inelegant PWM to
// gradually increase the perceived brightness of a single LED before main()
// calls larson() to begin scanning.
int powerup_2(void) {

	// Initialise PortD as output
	DDRD = 0xFF;
	
	// Set to no prescaling
	TCCR1B |= (1 << CS10);
	
	// Duty cycle is approximately 60Hz
	// Begin with LED lit for 0 ticks
	// Step size of 50 ticks	TODO - implement logarithmic step?
	int duty = 16000;
	int on = 0;
	int step = 50;
	
	// PWM loop until we are at full brightness
	while (on<=duty) {
	
		if (TCNT1 < on) 			// When count is within the "on" time
		{
			PORTD |= 0x08;			// Light the LED
		}
		else if (TCNT1 < duty) 		// When count is outside the "on" time
		{		
			PORTD &= !0x08;			// Dim the LED
		}
		else						// When the count has reached the duty cycle limit
		{
			TCNT1 = 0;				// Reset the counter
			on = on + step;			// Increase the time for which the LED is to be lit
		}
	}

}

/*===========================================================================*/

// Initial powerup routing to switch on a single LED and wait before
// starting to scan.
int powerup_1(void) {

	// Initialise PortD as output
	DDRD = 0xFF;
	// Start with the middle LED lit
	PORTD = 0x08;
	
	// Wait for dramatic effect
	// TODO - powerup gradual fade-in rather than instant-on
	_delay_ms(500);	

}


/*===========================================================================*/


int larson(void) {
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
