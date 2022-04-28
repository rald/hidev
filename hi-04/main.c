#include <stdlib.h>
#include <string.h>

#include "bea.h"


typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;

#define REG_DISPLAYCONTROL *((volatile uint32*)(0x04000000))
#define VIDEOMODE_3         0x0003
#define BGMODE_2            0x0400

#define SCREEN ((volatile uint16*)0x06000000)
#define SCREEN_W            240
#define SCREEN_H            160

#define SCREEN_S (SCREEN_W*SCREEN_H)

volatile uint16 dbuf[SCREEN_S]={0};


/* the button register holds the bits which indicate whether each button has
 * been pressed - this has got to be volatile as well
 */
volatile unsigned short* buttons = (volatile unsigned short*) 0x04000130;

/* the bit positions indicate each button - the first bit is for A, second for
 * B, and so on, each constant below can be ANDED into the register to get the
 * status of any one button */
#define BUTTON_A (1 << 0)
#define BUTTON_B (1 << 1)
#define BUTTON_SELECT (1 << 2)
#define BUTTON_START (1 << 3)
#define BUTTON_RIGHT (1 << 4)
#define BUTTON_LEFT (1 << 5)
#define BUTTON_UP (1 << 6)
#define BUTTON_DOWN (1 << 7)
#define BUTTON_R (1 << 8)
#define BUTTON_L (1 << 9)

/* this function checks whether a particular button has been pressed */
unsigned char button_pressed(unsigned short button) {
    /* and the button register with the button constant we want */
    unsigned short pressed = *buttons & button;

    /* if this value is zero, then it's not pressed */
    if (pressed == 0) {
        return 1;
    } else {
        return 0;
    }
}

#define REG_VCOUNT (* (volatile uint16*) 0x04000006)
void vsync(void) {
  while (REG_VCOUNT >= 160);
  while (REG_VCOUNT < 160);
}

uint16 rgb16(uint8 red, uint8 green, uint8 blue) {
  return (red & 0x1F) | (green & 0x1F) << 5 | (blue & 0x1F) << 10;
}

void pset(volatile uint16* srf,int x,int y,uint16 c) {
  srf[ y * SCREEN_W + x] = c;
}

void frect(volatile uint16* srf,int x,int y,int w, int h,uint16 c) {
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
          pset(srf,x+i,y+j,c);
        }
    }
}

void flip(volatile uint16* srf) {
  memcpy(SCREEN,srf,SCREEN_S);
}

void dbmp(volatile uint16* srf,const uint16* bmp,int w,int h,int x,int y) {
  for(int j=0;j<y+h;j++) {
    for(int i=0;i<x+w;i++) {
      srf[i+j*SCREEN_W]=bmp[i+j*w];      
    }    
  }
}


int main() {
  REG_DISPLAYCONTROL = VIDEOMODE_3 | BGMODE_2;

  for (int i = 0; i < SCREEN_W * SCREEN_H; ++i) {
  	SCREEN[i] = rgb16(0,0,0);
  }

  vsync();

  dbmp(SCREEN,bea_pixels,bea_width,bea_height,0,0);

  while(1);
  
  return 0;
}   
