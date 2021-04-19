// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); 								// Disable interrupts
void EnableInterrupts(void);  								// Enable interrupts
void initPortB(void);  				
void initPortE(void);  				
void initPortF(void);  				
void SysTick_Init(void);  										// Initialise SysTick
void SysTick_Wait(unsigned long delay);  			// SysTick Delay Function
void SysTick_Wait10ms(unsigned long delay);  	// SysTick Wrapper Function

// Linked data structure
struct State {
	unsigned long carLeds;
	unsigned long pedLeds;
  unsigned long Time;
  unsigned long nextState[8];
};
typedef const struct State STyp;

// State definitions
#define goWest		0
#define waitWest	1
#define goSouth		2
#define waitSouth	3
#define walk			4
#define allOff1		5
#define allOff2		6
#define dontWalk1	7
#define dontWalk2	8

STyp FSM[9] = {
	{0x0C, 0x02, 100, {goWest, goWest, waitWest, waitWest, waitWest, waitWest, waitWest, waitWest}},
	{0x14, 0x02, 60, {waitWest, goWest, goSouth, goSouth, walk, walk, goSouth, goSouth}},
	{0x21, 0x02, 100, {goSouth, waitSouth, goSouth, waitSouth, waitSouth, waitSouth, waitSouth, waitSouth}},
	{0x22, 0x02, 60, {waitSouth, goWest, goSouth, goWest, walk, walk, walk, walk}},		
	{0x24, 0x08, 100, {walk, dontWalk1, dontWalk1, dontWalk1, walk, dontWalk1, dontWalk1, dontWalk1}},
	{0x24, 0x02, 60, {dontWalk1, dontWalk2, dontWalk2, dontWalk2, dontWalk2, dontWalk2, dontWalk2, dontWalk2}},
	{0x24, 0x00, 60, {dontWalk2, allOff1, allOff1, allOff1, allOff1, allOff1, allOff1, allOff1}},
	{0x24, 0x02, 60, {allOff1, allOff2, allOff2, allOff2, allOff2, allOff2, allOff2, allOff2}},
	{0x24, 0x00, 60, {allOff2, goWest, goSouth, goWest, walk, goWest, goSouth, goWest}},
};

unsigned long currentState;	// index to current state
unsigned long Input;



// ***** 3. Subroutines Section *****

void PortBEF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x32;      				// 1) B E F
  delay = SYSCTL_RCGC2_R;      				// 2) no need to unlock
	
  GPIO_PORTE_AMSEL_R 	&= ~0x07; 			// 3) disable analog function on PE2-0
  GPIO_PORTE_PCTL_R 	&= ~0x00000FFF; 	// 4) enable regular GPIO
  GPIO_PORTE_DIR_R 		&= ~0x07; 			// 5) inputs on PE2-0
  GPIO_PORTE_AFSEL_R 	&= ~0x07; 			// 6) regular function on PE2-0
  GPIO_PORTE_DEN_R 		|=  0x07; 			// 7) enable digital on PE2-0

  GPIO_PORTB_AMSEL_R 	&= ~0x3F; 			// 3) disable analog function on PB5-0
  GPIO_PORTB_PCTL_R 	&= ~0x00FFFFFF; 	// 4) enable regular GPIO
  GPIO_PORTB_DIR_R 		|=  0x3F;    		// 5) outputs on PB5-0
  GPIO_PORTB_AFSEL_R 	&= ~0x3F; 			// 6) regular function on PB5-0
  GPIO_PORTB_DEN_R 		|=  0x3F;    		// 7) enable digital on PB5-0

  GPIO_PORTF_AMSEL_R 	&= ~0x0A;       	// 3) disable analog on PF1, PF3
  GPIO_PORTF_PCTL_R 	&= ~0x0000F0F0; 	// 4) PCTL GPIO on PF1, PF3
  GPIO_PORTF_DIR_R 		|=  0x0A;       	// 5) PF3, PF1 out
  GPIO_PORTF_AFSEL_R 	&= ~0x0A;       	// 6) disable alt funct on PF1, PF3
  GPIO_PORTF_DEN_R 		|=  0x0A;       	// 7) enable digital I/O on PF1, PF3
}

int main(void){ 
	TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	
	initPortB();	
	initPortE();
	initPortF(); 
	
	SysTick_Init();

	currentState = goWest;
  EnableInterrupts();
	
  while(1){
		GPIO_PORTB_DATA_R = FSM[currentState].carLeds;			// output car leds
		GPIO_PORTF_DATA_R = FSM[currentState].pedLeds; 			// output pedestrian leds

		SysTick_Wait10ms(FSM[currentState].Time);						// delay
		Input = GPIO_PORTE_DATA_R & 0x7;
		currentState = FSM[currentState].nextState[Input];	// advance to next state
  }
}

void initPortB(void){
	unsigned long delay;
	SYSCTL_RCGC2_R |= 0x2;								// 1 - activate clock for port
	delay = SYSCTL_RCGC2_R;								// 2 - allow time for clock to start
	GPIO_PORTB_AMSEL_R &= ~0x3F; 					// 3 - disable analog for pins
	GPIO_PORTB_PCTL_R &= ~0x00FFFFFF;			// 4 - PCTL GPIO on pins
	GPIO_PORTB_DIR_R |= 0x3F;							// 5 - pin directions
	GPIO_PORTB_AFSEL_R &= ~0x3F;	 				// 6 - disable alt functions for pins
	GPIO_PORTB_DEN_R |= 0x3F;							// 7 - enable digital I/O for pins
}

void initPortE(void){
	unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;								// 1 - activate clock for port
	delay = SYSCTL_RCGC2_R;								// 2 - allow time for clock to start
	GPIO_PORTE_AMSEL_R &= ~0x07; 					// 3 - disable analog for pins
	GPIO_PORTE_PCTL_R &= ~0x00000FFF;			// 4 - PCTL GPIO on pins
	GPIO_PORTE_DIR_R &= ~0x07;						// 5 - pin directions
	GPIO_PORTE_AFSEL_R &= ~0x07;	 				// 6 - disable alt functions for pins
	GPIO_PORTE_DEN_R |= 0x07;							// 7 - enable digital I/O for pins
}

void initPortF(void){
	unsigned long delay;
	SYSCTL_RCGC2_R |= 0x20;								// 1 - activate clock for port
	delay = SYSCTL_RCGC2_R;								// 2 - allow time for clock to start
	GPIO_PORTF_AMSEL_R &= ~0x0A; 					// 3 - disable analog for pins
	GPIO_PORTF_PCTL_R &= ~0x0000F0F0;			// 4 - PCTL GPIO on pins
	GPIO_PORTF_DIR_R |= 0x0A;							// 5 - pin directions
	GPIO_PORTF_AFSEL_R &= ~0x0A;	 				// 6 - disable alt functions for pins
	GPIO_PORTF_DEN_R |= 0x0A;							// 7 - enable digital I/O for pins
}

void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;						// disable SysTick during setup
	NVIC_ST_CTRL_R = 0x00000005;	// enable SysTick with core clock
}

void SysTick_Wait(unsigned long delay){
	NVIC_ST_RELOAD_R =  delay-1;						// number of counts to wait
	NVIC_ST_CURRENT_R = 0;									// any value written to CURRENT clears it
	while((NVIC_ST_CTRL_R&0x00010000)==0){}	// wait for count flag
}

void SysTick_Wait10ms(unsigned long delay){
	unsigned long i;
	for(i=0; i < delay; i++){
		SysTick_Wait(800000);	// wait 10ms
	}
}


