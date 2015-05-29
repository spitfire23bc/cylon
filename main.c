/*
 * main.c
 *
 *	ATTiny4313 3LED Larson scanner code for use in a model Cylon Raider.
 *	
 *	Initial fade-in and scan brightness are performed with software PWM using 
 *	timer0 interrupts:
 *		overflow - all three LEDs lit
 *		compareA - dim outer LEDs
 *		compareB - dim mid LED (this interrupt is disabled afer the initial fade-in)
 *
 *                       ------
 *                  1 --|      |-- 20  +5v
 *      scan LED1   2 --|      |-- 19  wep LEDs
 *      scan LED2   3 --|      |-- 18  
 *                  4 --|      |-- 17  thr LED2
 *                  5 --|      |-- 16  thr LED1
 *      scan LED3   6 --|      |-- 15  scan LED11
 *      scan LED4   7 --|      |-- 14  scan LED10
 *      scan LED5   8 --|      |-- 13  scan LED9
 *      scan LED6   9 --|      |-- 12  scan LED8
 *             0V  10 --|      |-- 11  scan LED7
 *                       ------
 *
 *	TODO - add flickering thrusters
 *	TODO - tidy up interrupt code and fade-in
 *
 */
/*===========================================================================*/
// Libararies and defs
#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

/*===========================================================================*/
// Type defs
typedef enum { false, true } bool_t;

/*===========================================================================*/
// Prototypes
int main(void);
int larson(void);

/*===========================================================================*/
// Constants

// Number of LEDs = 11
const uint16_t con_scan_LED_first = (1 << 0);
const uint16_t con_scan_LED_mid = (1 << 5);
const uint16_t con_scan_LED_last = (1 << 10);

const uint8_t con_thr_1 = 0x01;
const uint8_t con_thr_2 = 0x02;

// Constants to define which port bits to set
const uint8_t con_scan_bitsD = 0x7F;
const uint8_t con_scan_bitsB = 0x0F;
const uint8_t con_thr_bitsB = 0x30;
const uint8_t con_wep_bitsB = 0x80;

const uint16_t thr_array1[16] = {13000, 10000, 7000, 15000, 12000, 9000, 11000, 12000, 13000, 12000, 15000, 11000, 7000, 14000, 8000, 11000};
const uint16_t thr_array2[16] = {13001, 10000, 7000, 15000, 12000, 9000, 11000, 12000, 13000, 12000, 15000, 11000, 7000, 14000, 8000, 11000};

/*===========================================================================*/
// Global signals

// Begin left-to-right
static bool_t scan_ltr = true;			

// Current lit LED - intialised to the middle LED
static uint16_t scan_led = 0x00;

// Strings to contain scanner and thruster LED on/off commands
volatile uint16_t scan_string = 0x0000;
volatile uint8_t thr_string = 0x00;
uint8_t portb_string = 0x00;

// Counters
volatile uint8_t i = 0;
volatile uint8_t j = 0;
volatile uint8_t k = 0;
volatile uint8_t l = 0;
volatile uint8_t m = 0;


/*===========================================================================*/


int main(void) {

	// Enable global interrupts
	sei();
	
	// Enable timer interrupts
	//        t0 compA    |   t0 compB    |  t0 overflow |   t1 compA    |   t1 compB
	TIMSK = (1 << OCIE0A) | (1 << OCIE0B) | (1 << TOIE0) | (1 << OCIE1A) | (1 << OCIE1B);
	
	OCR1A = 500;
	OCR1B = 500;
	
	/*-----------------------------------------------------------------------*/

	// Initialise PORTB and PORTD as outputs
	DDRB = 0xFF;
	DDRD = 0xFF;
	scan_led = con_scan_LED_mid;
	/*-----------------------------------------------------------------------*/

	// Setup timer0 to overflow at approx 60Hz (8bit timer / 64 = 16320 ticks)
	//              /64 prescale
	TCCR0B |= (1 << CS01) | (1 << CS00);
	
	// Setup timer1 with no prescaling
	TCCR1B |= (1 << CS10);
	
	/*-----------------------------------------------------------------------*/

	// Main loop
	while(1) {
		
		// Update the output port(s) to light the LEDs as demanded
		PORTD = scan_string & con_scan_bitsD;			// 7 eye LEDs (scan_string bits 0..6)
		
		portb_string = ((scan_string >> 7) & con_scan_bitsB);	// 4 eye LEDs (scan_string bits 7..10)
		portb_string |= ((thr_string << 4) & con_thr_bitsB);	// 2 thruster LEDs (thr_string bits 0..1)
		portb_string |= con_wep_bitsB;							// Weapons bay LEDs at constant brightness
	
		PORTB = portb_string;

	}	
	
	return 1;
}

/*===========================================================================*/

// timer0 overflow interrupt - occurs at ~60Hz
ISR(TIMER0_OVF_vect) {

	// Reset timer1 (16 bit timer)
	TCNT1 = 0;
	// Switch on thruster LEDs
	thr_string = con_thr_1 | con_thr_2;


	// Scanner fade-in
	if (OCR0B<245) {
		if (j >= 3) {
			OCR0A++;
			OCR0B += 10;
			j=0;
		}
		else {
			j++;
		}
	}
	else {
		// Disable timer0 compare B interrupt once at full brightness
		TIMSK &= ~(1 << OCIE0B);
		
		if (k >= 63) {
			i++;
		}
		else {
			k++;
		}
		
	}


	if (i>=4) {
		// Call larson() to shift the lit LED demand
		larson();
		i=0;
	}
	
	scan_string |= (scan_led<<1) | scan_led | (scan_led>>1);
}

/*===========================================================================*/

// timer0 compare A interrupt
// PWM off for outer LEDs
ISR(TIMER0_COMPA_vect) {
	scan_string &= ~((scan_led<<1) | (scan_led>>1));
}

/*===========================================================================*/

// timer0 compare B interrupt
// PWM off for inner LED (disabled after startup)
ISR(TIMER0_COMPB_vect) {
	scan_string &= ~scan_led;
}

/*===========================================================================*/

// timer1 compare A interrupt
// 
ISR(TIMER1_COMPA_vect) {
	thr_string &= ~con_thr_1;

	if (l<15) {
		l++;
	}
	else {
		l=0;
	}		
	OCR1B += thr_array1[l];
}

/*===========================================================================*/

// timer1 compare B interrupt
// 
ISR(TIMER1_COMPB_vect) {
	thr_string &= ~con_thr_2;
	
	if (m>0) {
		m--;
	}
	else {
		m=15;
	}
	OCR1A += thr_array2[m];
}

/*===========================================================================*/

// Shifts the current lit LED left/right between the defined limits 
// con_scan_LED_first and con_scan_LED_last
int larson(void) {

	if (scan_ltr==true) {						// Scan left to right
		if (scan_led < con_scan_LED_last) {
			scan_led = (scan_led << 1);			// Left-shift to light next LED
		}
		else {				 					// Right-most position		
			scan_ltr = false;					// Set the direction to right-to-left
		}
	}
	else {										// Scan right to left 
		if (scan_led > con_scan_LED_first) {
			scan_led = (scan_led >> 1);			// Right-shift to light next LED
		}
		else {									// Left-most position
			scan_ltr = true;					// Set the direction to left-to-right
		}
	}

	return 1;

}

