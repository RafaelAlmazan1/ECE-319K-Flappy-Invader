// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Your name
// Last Modified: January 12, 2026

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC.h"
#include "../inc/Arabic.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#define SCREEN_W 128
#define SCREEN_H 160
#define PIPE_W   16
#define GAP_H    40
#define PIPE_SPEED 2
#define BLACK  0x0000
#define BIRD_W 18
#define BIRD_H 18
#define FOX_W 16
#define FOX_H 16
#define PELLET_W  3
#define PELLET_H  3
#define PELLET_SPEED 4
// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz



uint32_t Flag, score, gameOver, startup, pause, oldKeys;

typedef enum {English, Spanish} Language_t;
Language_t myLanguage=English;
typedef enum {SCORE, LOSE} phrase_t;
const char Score_English[] ="Score: ";
const char Score_Spanish[] ="Puntaje: ";
const char Lose_English[] ="YOU LOST";
const char Lose_Spanish[] ="PERDISTE";
const char *Phrases[2][2]={
  {Score_English, Score_Spanish},
  {Lose_English, Lose_Spanish}
};

typedef struct{
  int32_t x;
  int32_t gapY;
  uint8_t foxAlive;
  uint8_t passed;
} Pipe_t;

Pipe_t pipes[3];

typedef struct{
  int32_t x;
  int32_t y;
} Bird_t;

Bird_t bird;

typedef struct{
  int32_t x;
  int32_t y;
  uint8_t active;
} Pellet_t;

Pellet_t pellet;

void DrawFox(int32_t pipeX, int32_t gapY){
  int32_t foxX = pipeX + (PIPE_W - FOX_W)/2;
  int32_t foxY = gapY + FOX_H/2;

  if(foxX > -FOX_W && foxX < SCREEN_W){
    ST7735_DrawBitmap(foxX, foxY, Fox, FOX_W, FOX_H);
  }
}

void InitPellet(void){
  pellet.x = 0;
  pellet.y = 0;
  pellet.active = 0;
}

void FirePellet(void){
  if(pellet.active == 0){
    pellet.active = 1;
    pellet.x = bird.x + BIRD_W;
    pellet.y = bird.y - BIRD_H/2 + PELLET_H/2;
    Sound_Shoot();
  }
}

void CheckShootButton(void){
  uint32_t keys = Switch_In();

  if(keys & 0x02){   // PB17 
    FirePellet();
  }

    if(keys & 0x01 && (oldKeys & 0x01) == 0){   // PB16 
      pause = !pause;
  }
  oldKeys = keys;
}

void DrawPellet(void){
  if(pellet.active){
    ST7735_DrawBitmap(pellet.x, pellet.y, pellet0, PELLET_W, PELLET_H);
  }
}

void UpdatePellet(void){
  if(pellet.active){
    pellet.x += PELLET_SPEED;

    if(pellet.x >= SCREEN_W){
      pellet.active = 0;
    }
  }
}

void InitBird(void){
  bird.x = 10;
  bird.y = 80;
}

void UpdateBird(void){
  uint32_t adc = ADCin();
  bird.y = SCREEN_H - ((adc * (SCREEN_H - BIRD_H)) / 4095);
}

void DrawBird(void){
  ST7735_DrawBitmap(bird.x, bird.y, Bird0, BIRD_W, BIRD_H);
}

void InitPipes(void){
  pipes[0].x = 120; pipes[0].gapY = 50;  pipes[0].foxAlive = 1; pipes[0].passed = 0;
  pipes[1].x = 200; pipes[1].gapY = 90;  pipes[1].foxAlive = 1; pipes[1].passed = 0;
  pipes[2].x = 280; pipes[2].gapY = 70;  pipes[2].foxAlive = 1; pipes[2].passed = 0;
}

void DrawOnePipe(int32_t x, int32_t gapY){
  int32_t drawX = x;
  int32_t drawW = PIPE_W;

  int32_t topH = gapY - GAP_H/2;
  int32_t bottomY = gapY + GAP_H/2;
  int32_t bottomH = SCREEN_H - bottomY;


  if(drawX < 0){
    drawW = PIPE_W + drawX;   
    drawX = 0;
  }
  if((drawX + drawW) > SCREEN_W){
    drawW = SCREEN_W - drawX;
  }

  if(drawW <= 0) return;

  if(topH > 0){
    ST7735_FillRect(drawX, 0, drawW, topH, ST7735_YELLOW);
  }
  if(bottomH > 0){
    ST7735_FillRect(drawX, bottomY, drawW, bottomH, ST7735_YELLOW);
  }
}

void DrawPipes(void){
  for(int i = 0; i < 3; i++){
    DrawOnePipe(pipes[i].x, pipes[i].gapY);
    if(pipes[i].foxAlive){
      DrawFox(pipes[i].x, pipes[i].gapY);
    }
  }
}

void UpdatePipes(void){
  for(int i = 0; i < 3; i++){
    pipes[i].x -= PIPE_SPEED;

    if((pipes[i].x + PIPE_W) < 0){
      pipes[i].x = SCREEN_W + 60;
      pipes[i].gapY += 20;
      if(pipes[i].gapY > 110){
        pipes[i].gapY = 50;
      }
      pipes[i].foxAlive = 1;
      pipes[i].passed = 0;
    }
  }
}

void UpdateScore(void){
  for(int i = 0; i < 3; i++){
    if((pipes[i].passed == 0) && ((pipes[i].x + PIPE_W) < bird.x)){
      pipes[i].passed = 1;
      score++;
    }
  }
}

uint8_t CheckCollision(int32_t x1, int32_t y1, int32_t w1, int32_t h1, int32_t x2, int32_t y2, int32_t w2, int32_t h2){
  int32_t left1   = x1;
  int32_t right1  = x1 + w1 - 1;
  int32_t top1    = y1 - h1 + 1;
  int32_t bottom1 = y1;

  int32_t left2   = x2;
  int32_t right2  = x2 + w2 - 1;
  int32_t top2    = y2 - h2 + 1;
  int32_t bottom2 = y2;

  if(right1 < left2)  return 0;
  if(right2 < left1)  return 0;
  if(bottom1 < top2)  return 0;
  if(bottom2 < top1)  return 0;

  return 1;
}

void CheckPelletFoxCollision(void){
  if(pellet.active == 0) return;

  for(int i = 0; i < 3; i++){
    if(pipes[i].foxAlive){
      int32_t foxX = pipes[i].x + (PIPE_W - FOX_W)/2;
      int32_t foxY = pipes[i].gapY + FOX_H/2;

      if(CheckCollision(pellet.x, pellet.y, PELLET_W, PELLET_H, foxX, foxY, FOX_W, FOX_H)){
        pipes[i].foxAlive = 0;
        pellet.active = 0;
        Sound_Explosion();
      }
    }
  }
}

void CheckBirdPipeCollision(void){
  for(int i = 0; i < 3; i++){
    int32_t topH = pipes[i].gapY - GAP_H/2;
    int32_t bottomY = pipes[i].gapY + GAP_H/2;
    int32_t bottomH = SCREEN_H - bottomY;

    if(topH > 0){
      if(CheckCollision(bird.x, bird.y, BIRD_W, BIRD_H, pipes[i].x, topH-1, PIPE_W, topH)){
        gameOver = 1;
        Sound_Killed();
      }
    }

    if(bottomH > 0){
      if(CheckCollision(bird.x, bird.y, BIRD_W, BIRD_H, pipes[i].x, SCREEN_H-1, PIPE_W, bottomH)){
        gameOver = 1;
        Sound_Killed();
      }
    }
  }
}

void CheckBirdFoxCollision(void){
  for(int i = 0; i < 3; i++){
    if(pipes[i].foxAlive){
      int32_t foxX = pipes[i].x + (PIPE_W - FOX_W)/2;
      int32_t foxY = pipes[i].gapY + FOX_H/2;

      if(CheckCollision(bird.x, bird.y, BIRD_W, BIRD_H, foxX, foxY, FOX_W, FOX_H)){
        gameOver = 1;
        Sound_Killed();
      }
    }
  }
}


////////////////////////////// ERASE FUNCTIONS
void EraseFox(int32_t pipeX, int32_t gapY){
  int32_t foxX = pipeX + (PIPE_W - FOX_W)/2;
  int32_t foxY = gapY + FOX_H/2;
  if(foxX > -FOX_W && foxX < SCREEN_W){
    ST7735_FillRect(foxX, foxY-FOX_H+1, FOX_W, FOX_H, BLACK);
  }
}

void EraseBird(void){
  ST7735_FillRect(bird.x, bird.y-BIRD_H+1, BIRD_W, BIRD_H, BLACK);
}

void ErasePellet(void){
  if(pellet.active){
    ST7735_FillRect(pellet.x, pellet.y-PELLET_H+1, PELLET_W, PELLET_H, BLACK);
  }
}

void EraseOnePipe(int32_t x, int32_t gapY){
  int32_t drawX = x;
  int32_t drawW = PIPE_W;
  int32_t topH = gapY - GAP_H/2;
  int32_t bottomY = gapY + GAP_H/2;
  int32_t bottomH = SCREEN_H - bottomY;

  if(drawX < 0){
    drawW = PIPE_W + drawX;
    drawX = 0;
  }
  if((drawX + drawW) > SCREEN_W){
    drawW = SCREEN_W - drawX;
  }
  if(drawW <= 0) return;

  if(topH > 0){
    ST7735_FillRect(drawX, 0, drawW, topH, BLACK);
  }
  if(bottomH > 0){
    ST7735_FillRect(drawX, bottomY, drawW, bottomH, BLACK);
  }
}

void ErasePipes(void){
  for(int i=0; i<3; i++){
    EraseOnePipe(pipes[i].x, pipes[i].gapY);
    if(pipes[i].foxAlive){
      EraseFox(pipes[i].x, pipes[i].gapY);
    }
  }
}


void EraseGame(void){
  ErasePipes();
  EraseBird();
  ErasePellet();
}

//END OF ERASE FUNCTIONS

void UpdateGame(void){
  CheckShootButton();
    if (!pause) {
      EraseGame();
      UpdatePipes();
      UpdateBird();
      UpdatePellet();
      CheckPelletFoxCollision();
      CheckBirdPipeCollision();
      CheckBirdFoxCollision();
      UpdateScore();
  }
}

void DrawScore(void){
  ST7735_FillRect(0,0,80,10,BLACK);
  ST7735_SetCursor(0,0);
  ST7735_OutString((char *)Phrases[SCORE][myLanguage]);
  ST7735_OutUDec(score);
}

void DrawGame(void){
  //ST7735_FillScreen(BLACK);
  DrawPipes();
  DrawBird();
  DrawPellet();
  DrawScore();
}

void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

Arabic_t ArabicAlphabet[]={
alif,ayh,baa,daad,daal,dhaa,dhaal,faa,ghayh,haa,ha,jeem,kaaf,khaa,laam,meem,noon,qaaf,raa,saad,seen,sheen,ta,thaa,twe,waaw,yaa,zaa,space,dot,null
};
Arabic_t Hello[]={alif,baa,ha,raa,meem,null}; // hello
Arabic_t WeAreHonoredByYourPresence[]={alif,noon,waaw,ta,faa,raa,sheen,null}; // we are honored by your presence
int main0(void){ // main 0, demonstrate Arabic output
  Clock_Init80MHz(0);
  LaunchPad_Init();
  ST7735_InitR(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_WHITE);
  Arabic_SetCursor(0,15);
  Arabic_OutString(Hello);
  Arabic_SetCursor(0,31);
  Arabic_OutString(WeAreHonoredByYourPresence);
  Arabic_SetCursor(0,63);
  Arabic_OutString(ArabicAlphabet);
  while(1){
  }
}
uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}


// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    if (!startup)
      UpdateGame();
    Flag = 1;
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}
// use main1 to observe special characters
// int main1(void){ // main1
//     char l;
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
//   ST7735_FillScreen(0x0000);            // set screen to black
//   for(phrase_t myPhrase=HELLO; myPhrase<= GOODBYE; myPhrase++){
//     for(Language_t myL=English; myL<= French; myL++){
//          ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
//       ST7735_OutChar(' ');
//          ST7735_OutString((char *)Phrases[myPhrase][myL]);
//       ST7735_OutChar(13);
//     }
//   }
//   Clock_Delay1ms(3000);
//   ST7735_FillScreen(0x0000);       // set screen to black
//   l = 128;
//   while(1){
//     Clock_Delay1ms(2000);
//     for(int j=0; j < 3; j++){
//       for(int i=0;i<16;i++){
//         ST7735_SetCursor(7*j+0,i);
//         ST7735_OutUDec(l);
//         ST7735_OutChar(' ');
//         ST7735_OutChar(' ');
//         ST7735_SetCursor(7*j+4,i);
//         ST7735_OutChar(l);
//         l++;
//       }
//     }
//   }
// }

// // use main2 to observe graphics
// int main2(void){ // main2
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
//     //note: if you colors are weird, see different options for
//     // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()
//   uint16_t sky = ST7735_Color565(120, 200, 255);
//   //ST7735_FillScreen(sky);
//   ST7735_FillScreen(ST7735_BLACK);
//   // ST7735_DrawBitmap(birdX, birdY, Bird0, 16, 20);
//   //ST7735_DrawBitmap(22, 159, Bird0, 18,18); // player ship bottom
//   //ST7735_DrawBitmap(53, 151, Bird0, 18,18);
//   //ST7735_DrawBitmap(42, 159, Bird0, 18,18); // player ship bottom
//   //ST7735_DrawBitmap(62, 159, Bird0, 18,18); // player ship bottom
//   //ST7735_DrawBitmap(82, 159, Bird0, 18,18); // player ship bottom
//   //ST7735_DrawBitmap(0, 90, Bird0, 18,18);
//   //ST7735_DrawBitmap(90,90, Fox, 16,17 );
//   //ST7735_DrawBitmap(30,30, pellet, 3, 3);
//   //pipes[0].x = 100;
//   //pipes[0].gapY = 70;
//   //pipes[0].foxAlive = 1;


//   //DrawOnePipe(pipes[0]);
//   //ST7735_DrawBitmap(40, 9, Bird0, 18,18);
//   //ST7735_DrawBitmap(60, 9, Bird0, 18,18);
//   //ST7735_DrawBitmap(80, 9, Bird0, 18,18);

//   for(uint32_t t=5000;t>0;t=t-5){
//     SmallFont_OutVertical(t,104,6); // top left
//     Clock_Delay1ms(50);              // delay 50 msec
//   }
//   ST7735_FillScreen(0x0000);   // set screen to black
//   ST7735_SetCursor(1, 1);
//   ST7735_OutString("GAME OVER");
//   ST7735_SetCursor(1, 2);
//   ST7735_OutString("Nice try,");
//   ST7735_SetCursor(1, 3);
//   ST7735_OutString("Earthling!");
//   ST7735_SetCursor(2, 4);
//   ST7735_OutUDec(1234);
//   while(1){
//   }
// }

// use main3 to test switches and LEDs
// int main3(void){ // main3
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   Switch_Init(); // initialize switches
//   LED_Init(); // initialize LED
//   while(1){
//     // write code to test switches and LEDs
    
//   }
// }
// use main4 to test sound outputs
// int main4(void){ uint32_t last=0,now;
//   __disable_irq();
//   PLL_Init(); // set bus speed
//   LaunchPad_Init();
//   Switch_Init(); // initialize switches
//   LED_Init(); // initialize LED
//   Sound_Init();  // initialize sound
//   TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
//   __enable_irq();
//   while(1){
//     now = Switch_In(); // one of your buttons
//     if((last == 0)&&(now == 1)){
//       Sound_Shoot(); // call one of your sounds
//     }
//     if((last == 0)&&(now == 2)){
//       Sound_Killed(); // call one of your sounds
//     }
//     if((last == 0)&&(now == 4)){
//       Sound_Explosion(); // call one of your sounds
//     }
//     if((last == 0)&&(now == 8)){
//       Sound_Fastinvader1(); // call one of your sounds
//     }
//     // modify this to test all your sounds
//   }
// }

// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  startup = 1;
  uint32_t keys = Switch_In();
  Flag = 0;
  score = 0;
  gameOver = 0;
  pause = 0;
  InitPipes();
  InitBird();
  InitPellet();
  __enable_irq();

  ST7735_SetRotation(2);
  ST7735_SetCursor(0,0);
  ST7735_OutString("English PB16 (right)");
  ST7735_SetCursor(0,2);
  ST7735_OutString("Espanol PB17 (left)");


  

  while(startup){
    uint32_t keys = Switch_In();

    if(keys & 0x01){
      myLanguage = English;
      startup = 0;
    }
    if(keys & 0x02){
      myLanguage = Spanish;
      startup = 0;
    }
  }

  ST7735_FillScreen(ST7735_BLACK);

  while(1){
    if (Flag) {
        Flag = 0;
    if (!startup) {
      if(!gameOver)
        DrawGame();
      else 
        break;
      }
    }
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  }
      startup = 1;
      ST7735_FillScreen(ST7735_BLACK);
      ST7735_SetCursor(6,8);
      ST7735_OutString((char *)Phrases[LOSE][myLanguage]);

      while(1){
      }
}
