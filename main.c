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

// Roughly 60Hz duty cycle
const int duty = 16000;

// Create 128 step log look-up table for PWM stages
// log_step[i] = duty*[1-log(i,duty)]
const int num_steps = 127;
const int log_step[128] = {	
	0, 26, 52, 79, 106, 133, 160, 187, 215, 243, 271, 299, 328, 357, 386, 415, 445, 475, 505, 535, 566, 597, 628, 660, 692, 724, 757, 789, 823, 856, 890, 924, 959, 994, 1029, 1065, 1101, 1137, 1174, 1212, 1249, 1288, 1326, 1365, 1405, 1445, 1485, 1526, 1568, 1610, 1653, 1696, 1740, 1784, 1829, 1874, 1921, 1968, 2015, 2063, 2112, 2162, 2212, 2264, 2316, 2368, 2422, 2477, 2532, 2589, 2646, 2705, 2764, 2825, 2886, 2949, 3013, 3079, 3146, 3214, 3283, 3354, 3427, 3501, 3577, 3655, 3734, 3816, 3900, 3985, 4073, 4164, 4257, 4353, 4451, 4553, 4658, 4766, 4878, 4994, 5114, 5239, 5368, 5503, 5644, 5791, 5944, 6105, 6275, 6453, 6642, 6842, 7056, 7283, 7528, 7793, 8080, 8395, 8743, 9132, 9573, 10082, 10684, 11421, 12371, 13711, 16000};


// Begin left-to-right
// Declare as static so larson() knows which way to shift the register
static int ltr = 1;			


/*===========================================================================*/


int main(void) {

	powerup();
	
	
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

// Powerup routine with a logarithmic step change.
// Each loop runs until timer count == 1/duty seconds (ie each loop takes 1/60s)
// Thus, for 256 steps, the loop takes approx 4.27s to reach peak brightness.
int powerup(void) {

	// Initialise PortD as output
	DDRD = 0xFF;
	
	// Set to no prescaling
	TCCR1B |= (1 << CS10);
	
	// Begin with LED lit for 0 ticks
	int i = 0;

	// PWM loop until we are at full brightness
	while (i<num_steps) {
		
		if (TCNT1 < log_step[i]) {		// When timer count is within the "on" time
			PORTD |= 0x08;				// Light the LED
		}
		else if (TCNT1 < duty) {		// When timer count is outside the "on" time
			PORTD &= !0x08;				// Dim the LED
		}
		else {							// When the count has reached the duty cycle limit
			TCNT1 = 0;					// Reset the timer count
			i++;						// Move to next PWM width
		}	
	} 
	
	_delay_ms(2000);					// TODO - interrupt here
										// TODO - sort out flicker here
	
}

/*===========================================================================*/


int larson(void) {
// TODO - rewrite this to shift a PWM pattern - use sine wave?

	if (ltr==1) 						// Scan left to right
	{
		if (PORTD<64) {
			PORTD = (PORTD << 1);			// Left-shift to light next LED
		}
		else {				 		// Right-most position		
			ltr = 0;				// Set the direction to right-to-left
		}
	}
	else {							// Scan right to left 
		if (PORTD>1) {
			PORTD = (PORTD >> 1);			// Right-shift to light next LED
		}
		else {						// Left-most position
			ltr = 1;				// Set the direction to left-to-right
		}
	}

}
