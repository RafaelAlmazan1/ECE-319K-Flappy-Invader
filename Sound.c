// Sound.c
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC.h"
#include "../inc/Timer.h"

uint32_t Counts, maxCounts;  // time in 10 ms
const uint8_t *currentSound; 

void SysTick_IntArm(uint32_t period, uint32_t priority){
  SysTick->CTRL = 0x00;      // disable SysTick during setup
  SysTick->LOAD = period-1;  // reload value
  SCB->SHP[1] = (SCB->SHP[1]&(~0xC0000000))|(priority<<30); // priority 2
  SysTick->VAL = 0;          // any write to VAL clears COUNT and sets VAL equal to LOAD
  SysTick->CTRL = 0x07;      // enable SysTick with 80 MHz bus clock and interrupts
}
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5-bit DAC
void Sound_Init(void){
// write this
DAC_Init();
SysTick_IntArm(7272, 2);
Counts = 0;
maxCounts = 0;
}
void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active
    // output one value to DAC if a sound is active

  DAC_Out(currentSound[Counts]);

  if (Counts == maxCounts - 1) 
    SysTick -> CTRL = 1;
  else 
    Counts++;
    
  //DAC_Out[array[counts]]
  //if end of array, SysTick ->CTRL = 1;, else counts++;
}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
// write this
  maxCounts = count;
  Counts = 0;
  currentSound = pt;
  SysTick->VAL = 0;          // any write to VAL clears COUNT and sets VAL equal to LOAD
  SysTick->CTRL = 0x07;      // enable SysTick with 80 MHz bus clock and interrupts 
}
void Sound_Shoot(void){
// write this
  Sound_Start( shoot, 4080);
}
void Sound_Killed(void){
// write this
  Sound_Start( invaderkilled, 3377);
}
void Sound_Explosion(void){
// write this
  Sound_Start( explosion, 2000);

}

