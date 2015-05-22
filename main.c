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
volatile uint8_t i = 0;
uint8_t j = 0;
/*===========================================================================*/


int main(void) {

	// Enable gloabl interrupts
	sei();
	
	// Enable timer0 interrupts
	//		  compare A   |	  compare B   |	  overflow
	TIMSK = (1 << OCIE0A) | (1 << OCIE0B) | (1 << TOIE0);
	
	/*-----------------------------------------------------------------------*/

	// Initialise PortD as output
	DDRD = 0xFF;
	led = 0x08;
	//PORTD = led;
	/*-----------------------------------------------------------------------*/

	// Setup timer0
	//	            64x prescale
	TCCR0B |= (1 << CS01) | (1 << CS00);
	

	/*-----------------------------------------------------------------------*/

	// Main loop
	while(1) {
		
	}	
		
}

/*===========================================================================*/

// timer0 PWM on for all three LEDs on overflow
ISR(TIMER0_OVF_vect) {

	

	if (OCR0B<241) {
		OCR0A++;
		OCR0B = OCR0B + 15;
	}
	else {
		// Enable timer0 interrupts
		//		  compare A   |	  compare B   |	  overflow
		TIMSK = (1 << OCIE0A) | (0 << OCIE0B) | (1 << TOIE0);
		i++;
	}


	if (i>=5) {
		// Update the PORTD register to shift the lit LED
		larson();
		//PORTD = led;
		i=0;
	}
	PORTD |= (led<<1) | led | (led>>1);		// LEDs on

}

/*===========================================================================*/

// timer0 PWM off for outer LEDs
ISR(TIMER0_COMPA_vect) {

	PORTD &= ~((led<<1) | (led>>1));		// Outer LEDs off
}

/*===========================================================================*/

// timer0 PWM off for inner LED (disabled after startup)
ISR(TIMER0_COMPB_vect) {

	PORTD &= ~led;						// Mid LED off
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
