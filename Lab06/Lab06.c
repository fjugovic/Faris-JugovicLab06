
/*
 *    File: Lab06.c
 * Author : Faris Jugovic
 * Created: 10/7/2022
 *
 * This code will cause the 8 PORTE LEDs to display in
 * 1 of 4 patterns. These patterns are Low to High, 
 * High to Low, Back and Forth, and BassSpectrum .
 * The patterns can be toggled in that order by pressing the joystick.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>               


// set variables to 0,1,2,3,respectively to mode variable
enum {LowToHigh, HighToLow, BackAndForth,BassSpectrum} mode;

// set variables to 0,1 respectively to Direction variable
enum {GoingHighSide, GoingLowSide} Direction; //Needed for the BackAndForth Pattern

static volatile unsigned int Tick; // Declares Tick counter
static bool DifPattern; // Tells main() that the pattern has changed

int main(void)
{
	
unsigned char Pattern = 0; // Controls the value of the LEDs

DDRE = 0xFF;  // Configure all PORTE pins as outputs
PORTE = 0xFF; // Turn off the active-low LEDs by setting them all to high

// set initial program values
Tick = 0;
Direction = GoingHighSide; //Makes the BackAndForth start low to high
DifPattern = 0;

// Must cause interrupt to trigger every 1ms by causing overflow in TIMER0
TCNT0  = -115;            // remember, counts up until overflow
TCCR0 = (1<<CS02);             // Default mode for this board
TIMSK = (1<<TOIE0);            // Sets TOIE0 to 1 which enables timer0 overflow interrupt

DDRB &= ~(1<<DDRB4);           // clear DDRB4 to make PB4 (joystick center button) an input
PORTB |= (1<<PINB4);           // enable PORTB.4 pull-up resistor
DDRD &= ~(1<<DDRD0);           // clear DDRD0 to make PD0 (external interrupt INT0) an input

EICRA = (1<<ISC00)|(1<<ISC01); // set interrupt to activate on RISING edge (i.e. on button release)
EIMSK |= (1<<INT0);            // Enable external interrupt 0

sei(); // Enable interrupts 

// "Game Loop"
while(1) { //Results in infinite while loop
if(Tick >= 50) { // 50 refers to 50ms delay
Tick = 0; // Reset tick count

switch(mode) { //Will execute current mode until it is changed in ISR(INT0_vect)
	
case LowToHigh: //Mode == 0 so LowToHigh

//Initialize Pattern for this mode
if(DifPattern == 1 ) { //Checks if pattern has changed
DifPattern = 0; //Resets the Pattern tracker
Pattern = 0x01;  //Gives first value for Pattern
}

PORTE = ~(Pattern); // Flips the bits in Pattern because PORTE is active low
Pattern <<= 0x01; //Shifts the on LED up
if(Pattern == 0x00){ //Resets the on LED when it passes the 8th LED
Pattern = 0x01; //Sets Pattern back to 1 before looping back to while(1)
}
break;

case HighToLow: //Mode == 1

//Initialize Pattern for this mode
if(DifPattern == 1) { //Checks if pattern has changed
DifPattern = 0; //Resets the Pattern tracker
Pattern = 0x80; //Gives first value for Pattern
}

PORTE = ~(Pattern); // Turn on Low signal LEDs
Pattern >>= 1; //Shifts the on LED down
if(Pattern == 0x00) //Resets the on LED when it passes the 8th LED
Pattern = 0x80; //Sets Pattern back to 1 before looping back to while(1)
break;

case BackAndForth: //Mode == 2

//Initialize Pattern for this mode
if(DifPattern == 1) { //Checks if pattern has changed
DifPattern = 0; //Resets the Pattern tracker
Direction = GoingHighSide; //Makes the BackAndForth start low to high
Pattern = 0x01; //Sets Pattern to 1 
}

PORTE = ~(Pattern); // Turn on Low signal LEDs
if(Direction == GoingHighSide) {
if(Pattern == 0x80) {
Direction = GoingLowSide; //Switch direction when at the 8th LED
Pattern = 0x40; //Sets 7th LED to on then move on
}
else {
Pattern <<= 1; //Shifts the on LED up
}
}
else {
if(Pattern == 0x01) {
Direction = GoingHighSide;
Pattern = 0x02;
}
else {
Pattern >>= 1; //Shifts the on LED down
}
}
break;

case BassSpectrum: //Mode == 3

//Initialize Pattern for this mode
if(DifPattern == 1) { //Checks if pattern has changed
DifPattern = 0; //Resets the Pattern tracker
Pattern = 0x80; //Gives first value for Pattern
}

PORTE = ~(Pattern);
Pattern >>= 1; //Shifts the on LED down
if(Pattern == 0x00) { //Resets the on LED when it passes the 8th LED
Pattern = 0xFF; //Sets Pattern back to 1 before looping back to while(1)
}

break;
}
} //Tick close
} // While(1) close
} //Main(void) close

ISR(TIMER0_OVF_vect) //Interrupt for the timer
{
Tick++; // Increment Tick
TCNT0 = -115; // reset timer0 counter
}

ISR(INT0_vect) // Interrupt is triggered on high edge of joystick clink PB4
{
switch(mode) { // Interrupt controls the current mode.
case LowToHigh:
mode = HighToLow; // Mode is now 1 for HighToLow
break;
case HighToLow:
mode = BackAndForth; // Mode is now 2 for BackAndForth
break;
case BackAndForth:
mode = BassSpectrum; // Mode is now 3 for BassSpectrum
break;
case BassSpectrum:
mode = LowToHigh; // Mode is now 0 for LowToHigh
}
DifPattern = 1; // Tells main() to move to next Pattern
}
