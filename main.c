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

int powerup(void);
int larson(void);
int led_control(void);

/*===========================================================================*/

// Roughly 60Hz duty cycle
const int duty = 16000;

// Create 128 step log look-up table for PWM stages
// log_step[i] = duty*[1-log(i,num_steps+1)]
const int num_steps = 32;
const uint16_t log_step[32] = {	
	0, 147, 298, 454, 616, 784, 959, 1140, 
	1328, 1525, 1730, 1945, 2170, 2407, 2656, 2920, 
	3200, 3498, 3816, 4159, 4528, 4930, 5370, 5856, 
	6400, 7016, 7728, 8570, 9600, 10928, 12800, 16000};


// Begin left-to-right
// Declare as static so larson() knows which way to shift the register
static int ltr = 1;			

volatile uint8_t led;

/*===========================================================================*/


int main(void) {

	sei();
	TIMSK = (1 << OCIE1A) | (1 << OCIE1B);

	// Initialise PortD as output
	DDRD = 0xFF;


	//powerup();
	
	led = 0x08;
	led_control();
	// Main loop
	while(1) {
		

		// Delay between LED changes
		// TODO - implement the delay with interrupts
		_delay_ms(100);
		
		
		PORTD = led;
		
		// Update the PORTD register to shift the lit LED
		larson();

		
	}	
	
		
}


/*===========================================================================*/
int led_control(void) {
	
	// Set to no prescaling
	TCCR1B |= (1 << CS10);
	TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode
	OCR1A = 16000;
	OCR1B = 14000;
	
	return 1;
	
}


ISR(TIMER1_COMPA_vect)
{
	PORTD &= ~((led<<1) | (led>>1));		// LEDs off
	PORTD |= led;
}

ISR(TIMER1_COMPB_vect)
{
	PORTD |= (led<<1) | (led>>1);		// LEDs on
	PORTD |= led;
}
/*===========================================================================*/

// Powerup routine with a logarithmic step change.
// Each loop runs until timer count == 1/duty seconds (ie each loop takes 1/60s)
// Thus, for 256 steps, the loop takes approx 4.27s to reach peak brightness.
int powerup(void) {
	
	// Set to no prescaling
	TCCR1B |= (1 << CS10);
	
	// Begin with LED lit for 0 ticks
	int i = 0;
	int j = 0;

	// PWM loop until we are at full brightness
	while (i<num_steps) {
		
		if (TCNT1 < log_step[i]) {		// When timer count is within the "on" time
			PORTD |= 0x08;				// Light the LED
		}
		else if (TCNT1 < duty) {		// When timer count is outside the "on" time
			PORTD &= ~0x08;				// Dim the LED
		}
		else {							// When the count has reached the duty cycle limit
			TCNT1 = 0;					// Reset the timer count
			j++;
			if (j>=5) {
				i++;					// Move to next PWM width
				j=0;
			}						
		}	
	} 
	
	_delay_ms(2000);					// TODO - interrupt here
										// TODO - sort out flicker here
	
	return 1;
}

/*===========================================================================*/


int larson(void) {
// TODO - rewrite this to shift a PWM pattern - use sine wave?

	if (ltr==1) 						// Scan left to right
	{
		if (led<64) {
			led = (led << 1);			// Left-shift to light next LED
		}
		else {				 		// Right-most position		
			ltr = 0;				// Set the direction to right-to-left
		}
	}
	else {							// Scan right to left 
		if (led>1) {
			led = (led >> 1);			// Right-shift to light next LED
		}
		else {						// Left-most position
			ltr = 1;				// Set the direction to left-to-right
		}
	}

	return 1;

}
