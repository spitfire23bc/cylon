# cylon
ATTiny4313 3LED Larson scanner code for use in a model Cylon Raider.

Initial fade-in and scan brightness are performed with software PWM using 
timer0 interrupts:
	overflow - all three LEDs lit
	compareA - dim outer LEDs
	compareB - dim mid LED (this interrupt is disabled afer the initial fade-in)

                       ------
                  1 --|      |-- 20  +5v
      scan LED1   2 --|      |-- 19
      scan LED2   3 --|      |-- 18  wep LEDs
                  4 --|      |-- 17  thr LED2
                  5 --|      |-- 16  thr LED1
      scan LED3   6 --|      |-- 15  scan LED11
      scan LED4   7 --|      |-- 14  scan LED10
      scan LED5   8 --|      |-- 13  scan LED9
      scan LED6   9 --|      |-- 12  scan LED8
             0V  10 --|      |-- 11  scan LED7
                       ------
