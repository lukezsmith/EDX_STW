// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****
#define PE0_SW								(*((volatile unsigned long *)0x40024004))
#define PE1_LED								(*((volatile unsigned long *)0x40024008))

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); 				// Disable interrupts
void EnableInterrupts(void);  				// Enable interrupts
void PortE_Init(void);								// Initialise Port E pins for I/O
void Delay1ms(unsigned long msec); 		// delay program for a given amount of ms

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz
  PortE_Init(); 																 // initialise port E pins
  EnableInterrupts();           								 // enable interrupts for the grader
  while(1){
    Delay1ms(100);								// delay for 100ms
		if(PE0_SW == 0x01){
			PE1_LED ^= 0x02;
		}else{
			PE1_LED = 0x02;
		}
  }
  
}

// Subroutine to initialise Port E pins for I/O
void PortE_Init(void){
	volatile unsigned long delay;
	
	SYSCTL_RCGC2_R |= 0x00000010;					// enable clock for Port E
	delay = SYSCTL_RCGC2_R;								// delay
	GPIO_PORTE_AFSEL_R &= 0x00;						// disable analog function
	GPIO_PORTE_PCTL_R	&= 0x00;						// GPIO clear bit PCTL
	GPIO_PORTE_DIR_R &= ~0x01;						// PE0 input
	GPIO_PORTE_DIR_R |= 0x02; 						// PE1 output
	GPIO_PORTE_AMSEL_R &= 0x00; 					// no alternative function
	//GPIO_PORTE_PDR_R &= 0x01;							// enable pull down resistor PE0
	//GPIO_PORTE_PDR_R &= 0x02;							// enable pull down resister PE1
	GPIO_PORTE_DEN_R |= 0x03;							// enable pins PE4-PE0
	
	PE1_LED = 0x02;												// set LED to high
}

// Subroutine to delay for a given amount of ms
void Delay1ms(unsigned long msec){
  unsigned long i;
  while(msec > 0){
    i = 13333;  // this number means 1ms
    while(i > 0){
      i = i - 1;
    }
    msec = msec - 1; // decrements every 1ms
  }
}

