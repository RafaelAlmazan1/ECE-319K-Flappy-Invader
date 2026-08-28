/*
 * Switch.c
 *
 *  Created on: January 12, 2026
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // write this
  IOMUX->SECCFG.PINCM[PB16INDEX] = 0x00040081;
  IOMUX->SECCFG.PINCM[PB17INDEX] = 0x00040081;
 
}
// return current state of switches
uint32_t Switch_In(void){
    // write this
    uint32_t KeyInput = GPIOB->DIN31_0&((1<<16)|(1<<17));
    return (KeyInput >> 16);
}
