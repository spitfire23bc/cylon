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



// Create 256 step log look-up table for PWM stages
// log_step[i] = duty*[1-log(i,duty)]
const int num_steps = 256;
const int log_step[num_steps] = {	
	0, 11, 23, 34, 45, 57, 68, 80, 92, 103, 115, 127, 139, 150, 162, 174, 
	186, 198, 210, 223, 235, 247, 259, 272, 284, 297, 309, 322, 334, 347, 360, 372, 
	385, 398, 411, 424, 437, 450, 464, 477, 490, 504, 517, 531, 544, 558, 572, 585, 
	599, 613, 627, 641, 655, 669, 684, 698, 712, 727, 741, 756, 771, 785, 800, 815, 
	830, 845, 860, 876, 891, 906, 922, 937, 953, 969, 984, 1000, 1016, 1032, 1049, 1065, 
	1081, 1098, 1114, 1131, 1147, 1164, 1181, 1198, 1215, 1233, 1250, 1267, 1285, 1303, 1320, 1338, 
	1356, 1374, 1392, 1411, 1429, 1448, 1466, 1485, 1504, 1523, 1542, 1562, 1581, 1601, 1620, 1640, 
	1660, 1680, 1701, 1721, 1741, 1762, 1783, 1804, 1825, 1846, 1868, 1889, 1911, 1933, 1955, 1978, 
	2000, 2023, 2045, 2068, 2092, 2115, 2139, 2162, 2186, 2210, 2235, 2259, 2284, 2309, 2334, 2360, 
	2385, 2411, 2437, 2464, 2490, 2517, 2544, 2572, 2599, 2627, 2655, 2684, 2712, 2741, 2771, 2800, 
	2830, 2860, 2891, 2922, 2953, 2984, 3016, 3049, 3081, 3114, 3147, 3181, 3215, 3250, 3285, 3320, 
	3356, 3392, 3429, 3466, 3504, 3542, 3581, 3620, 3660, 3701, 3741, 3783, 3825, 3868, 3911, 3955, 
	4000, 4045, 4092, 4139, 4186, 4235, 4284, 4334, 4385, 4437, 4490, 4544, 4599, 4655, 4712, 4771, 
	4830, 4891, 4953, 5016, 5081, 5147, 5215, 5285, 5356, 5429, 5504, 5581, 5660, 5741, 5825, 5911, 
	6000, 6092, 6186, 6284, 6385, 6490, 6599, 6712, 6830, 6953, 7081, 7215, 7356, 7504, 7660, 7825, 
	8000, 8186, 8385, 8599, 8830, 9081, 9356, 9660, 10000, 10385, 10830, 11356, 12000, 12830, 14000, 16000};


// Begin left-to-right
// Declare as static so larson() knows which way to shift the register
static int ltr = 1;			


/*===========================================================================*/


int main(void) {

	powerup_3();
	
	
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

// Third attempt at a powerup routine with a logarithmic step change.
// Each loop runs until timer count == 1/duty seconds (ie each loop takes 1/60s)
// Thus, for 256 steps, the loop takes approx 4.27s to reach peak brightness.
int powerup_3(void) {

	// Initialise PortD as output
	DDRD = 0xFF;
	
	// Set to no prescaling
	TCCR1B |= (1 << CS10);
	
	// Begin with LED lit for 0 ticks
	int i = 0;

	// PWM loop until we are at full brightness
	while (i<num_steps) {
		
		if (TCNT1 < log_step[i]) {		// When timer count is within the "on" time
			PORTD |= 0x08;			// Light the LED
		}
		else if (TCNT1 < duty) {		// When timer count is outside the "on" time
			PORTD &= !0x08;			// Dim the LED
		}
		else {					// When the count has reached the duty cycle limit
			TCNT1 = 0;			// Reset the timer count
			i++;				// Move to next PWM width
		}
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
	int step = 10;
	
	// PWM loop until we are at full brightness
	while (on<=duty) {
	
		if (TCNT1 < on) {			// When count is within the "on" time
			PORTD |= 0x08;			// Light the LED
		}
		else if (TCNT1 < duty) {		// When count is outside the "on" time
			PORTD &= !0x08;			// Dim the LED
		}
		else {					// When the count has reached the duty cycle limit
			TCNT1 = 0;			// Reset the counter
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
