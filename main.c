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
int thr_control(void);
int scan_control(void);
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

// Delays
const uint8_t con_scan_delay = 31;

const uint8_t scan_array[40] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 5, 7, 9, 11, 13, 16, 18, 21, 24, 28, 31, 35, 39, 43, 48, 53, 60, 67, 75, 85, 98, 117, 149, 255};

const uint16_t thr_array[64] = {106, 220, 343, 475, 619, 776, 950, 1145, 1366, 1621, 1922, 2291, 2766, 3436, 4582, 6317, 8388, 10378, 13038, 15000, 16000, 15000, 14200, 13500, 14500, 15500, 16000, 14500, 12000, 13500, 15500, 15000, 13500, 11500, 13000, 14000, 13000, 12500, 11000, 14000, 14000, 15000, 14000, 14500, 15000, 15500, 16000, 15000, 16000, 15000, 14000, 12000, 13000, 14000, 14000, 15000, 14000, 15000, 13000, 12000, 13000, 11500, 12500, 14000};

/*===========================================================================*/
// Global signals

// Begin left-to-right
volatile bool_t scan_ltr = true;			

// Current lit LED
volatile uint16_t scan_led = 0x00;

// Strings to contain scanner and thruster LED on/off commands
volatile uint16_t scan_string = 0x0000;
volatile uint8_t thr_string = 0x00;
uint8_t portb_string = 0x00;

// Counters
volatile uint8_t scan_out_pos = 0;
volatile uint8_t scan_mid_pos = 15;
volatile uint8_t scan_delay = 0;
volatile uint8_t thr_pos_A = 0;
volatile uint8_t thr_pos_B = 0;
volatile uint8_t wait_to_scan= 0;


/*===========================================================================*/


int main(void) {

	// Enable global interrupts
	sei();
	
	// Enable timer interrupts
	//        t0 compA    |   t0 compB    |  t0 overflow |   t1 compA    |   t1 compB
	TIMSK = (1 << OCIE0A) | (1 << OCIE0B) | (1 << TOIE0) | (1 << OCIE1A) | (1 << OCIE1B);
	
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
	thr_control();
	scan_control();
}

/*===========================================================================*/

// timer0 compare A interrupt
// PWM off for outer LEDs
ISR(TIMER0_COMPA_vect) {
	scan_string &= ~((scan_led<<1) | (scan_led>>1));
}

/*===========================================================================*/

// timer0 compare B interrupt
// PWM off for mid LED (disabled after startup)
ISR(TIMER0_COMPB_vect) {
	scan_string &= ~scan_led;
}

/*===========================================================================*/

// timer1 compare A interrupt
// 
ISR(TIMER1_COMPA_vect) {
	thr_string &= ~con_thr_1;
}

/*===========================================================================*/

// timer1 compare B interrupt
// 
ISR(TIMER1_COMPB_vect) {
	thr_string &= ~con_thr_2;
}

/*===========================================================================*/

int thr_control(void) {

	// Reset timer1 (16 bit timer)
	TCNT1 = 0;
	
	// Thruster LEDs PWM on
	thr_string = con_thr_1 | con_thr_2;
	
	
	// Select timer1 compA value
	if (thr_pos_A<23) {
		thr_pos_A++;
	}
	else if (thr_pos_A<63) {
		thr_pos_A++;
	}
	else {
		thr_pos_A=23;
	}		
	OCR1A = thr_array[thr_pos_A];
	
	
	// Select timer1 compB value
	if (thr_pos_B<23) {
		thr_pos_B++;
	}
	else if (thr_pos_B>24) {
		thr_pos_B--;
	}
	else {
		thr_pos_B=63;
	}		
	OCR1B = thr_array[thr_pos_B];

	return 1;
}

/*===========================================================================*/

int scan_control(void) {

	// Scanner fade-in
	if (scan_mid_pos<38) {
			scan_mid_pos++;		
			scan_out_pos++;	
			OCR0A = scan_array[scan_out_pos];
			OCR0B = scan_array[scan_mid_pos];
	}
	else {
		// Disable timer0 compare B interrupt once at full brightness
		TIMSK &= ~(1 << OCIE0B);
		
		/*-------------------------------------------------------------------*/
		// Call larson() after an initial delay to shift the lit LED demand
		
		if (wait_to_scan >= con_scan_delay) {
			scan_delay++;
		}
		else {
			wait_to_scan++;
		}
	}

	if (scan_delay>=6) {

		larson();
		scan_delay=0;
	}
	
	// Scanner mid and outer LEDs PWM on
	scan_string |= (scan_led<<1) | scan_led | (scan_led>>1);

	return 1;
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

