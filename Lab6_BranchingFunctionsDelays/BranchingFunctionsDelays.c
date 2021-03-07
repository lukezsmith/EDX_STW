// BranchingFunctionsDelays.c Lab 6
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to 
// toggle an LED while a button is pressed and 
// turn the LED on when the button is released.  
// This lab will use the hardware already built into the LaunchPad.
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// built-in connection: PF0 connected to negative logic momentary switch, SW2
// built-in connection: PF1 connected to red LED
// built-in connection: PF2 connected to blue LED
// built-in connection: PF3 connected to green LED
// built-in connection: PF4 connected to negative logic momentary switch, SW1

#include "TExaS.h"

#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control

// Global Variables
unsigned long SW1; // input for PF4
unsigned long Out; // output for PF2

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortF_Init(void); // Initialise Ports PF2, PF4 
void Delay100ms(unsigned long time); // Delay 100ms

int main(void){ unsigned long volatile delay;
  TExaS_Init(SW_PIN_PF4, LED_PIN_PF2);  // activate grader and set system clock to 80 MHz
  PortF_Init(); 												// Initialise ports PF2, PF4
  EnableInterrupts();           				// enable interrupts for the grader
	
	Out = GPIO_PORTF_DATA_R;
  while(1){
    Delay100ms(1);														// delay each iteration by 100ms
		if(!(GPIO_PORTF_DATA_R & 0x10)){ 		// if P4=0, switch is pressed
			Out ^= 0x4;													// toggle LED output
			GPIO_PORTF_DATA_R &= ~0x4;					// clear PF2 
			GPIO_PORTF_DATA_R |= Out;						// set PF2 to new value
		}else{															// P4=1, switch is not pressed
			GPIO_PORTF_DATA_R |= 0x4;						// turn LED on
		}
  }
}

// Subroutine to initialize port F pins for input and output
// PF4 is input SW1
// PF2 is output to the LED
// Inputs: None
// Outputs: None
// Notes: These two pins are connected to hardware on the LaunchPad
void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // 2) delay   
  GPIO_PORTF_AMSEL_R &= ~0x14;      // 3) disable analog function for bits PF4 and PF2
  GPIO_PORTF_PCTL_R &= ~0x14;   		// 4) GPIO clear bit PCTL for PF4 and PF2
	GPIO_PORTF_AFSEL_R = ~0x14;       // 5) no alternate function for PF4 and PF2
  GPIO_PORTF_DIR_R |= 0x4;          // 6) PF4input, PF2 output   
  GPIO_PORTF_PUR_R |= 0x10;         // 7) enable pullup resistors on PF4    
  GPIO_PORTF_DEN_R |= 0x14;         // 8) enable digital pins PF4, PF2 
	GPIO_PORTF_DATA_R |= 0x4;			  	// 9) Set LED initially on
}

// Subroutine to delay by ~100ms
void Delay100ms(unsigned long time) {
  unsigned long i;
  while(time > 0){
    i = 1333333;  // this number means 100ms
    while(i > 0){
      i = i - 1;
    }
    time = time - 1; // decrements every 100 ms
  }
}


