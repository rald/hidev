#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "sweetie.h"

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

uint16 dbuf[SCREEN_S]={0};
int x=0,y=0;
int s=8;
int w=240/8,h=160/8;
uint8 r=0,g=0,b=0;
int c0=0,c1=0,c2=0;
int xi=0,yi=0;
int d=20000;
int p=12;
bool hold=false;

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

uint16 pget(volatile uint16* srf,int x,int y) {
  return srf[ y * SCREEN_W + x];
}

void frect(volatile uint16* srf,int x,int y,int w,int h,uint16 c) {
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
          pset(srf,x+i,y+j,c);
        }
    }
}

void drect(volatile uint16*srf,int x,int y,int w,int h,uint16 c) {
  for(int i=x;i<x+w;i++) {
    pset(srf,i,y,c);
    pset(srf,i,y+h-1,c);    
  }

  for(int j=y;j<y+h;j++) {
    pset(srf,x,j,c);
    pset(srf,x+h-1,j,c);    
  }
}

void flip(volatile uint16* srf) {
  memcpy((void*)SCREEN,(void*)srf,SCREEN_S);
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

  frect(SCREEN,0,0,239,159,rgb16(
    sweetie_palette[0*3+0],
    sweetie_palette[0*3+1],
    sweetie_palette[0*3+2]
  ));


  c1=pget(SCREEN,x*s,y*s);

  c2=rgb16(
    sweetie_palette[p*3+0],
    sweetie_palette[p*3+1],
    sweetie_palette[p*3+2]
  );
 
  vsync();
  frect(SCREEN,x*s,y*s,s,s,c2);

  while(1) {
  
    xi=0,yi=0;

    if(button_pressed(BUTTON_UP)) xi=0,yi=-1;
    if(button_pressed(BUTTON_DOWN)) xi=0,yi=1;
    if(button_pressed(BUTTON_LEFT)) xi=-1,yi=0;
    if(button_pressed(BUTTON_RIGHT)) xi=1,yi=0;

    if(button_pressed(BUTTON_B)) {
      if(!hold) {
        hold=true;
        p=(p+1)%16;

        c2=rgb16(
          sweetie_palette[p*3+0],
          sweetie_palette[p*3+1],
          sweetie_palette[p*3+2]
        );

        vsync();
        frect(SCREEN,x*s,y*s,s,s,c2);
      }
    } else if(hold) {
      hold=false;
    }

    if(xi || yi) {

      if(!button_pressed(BUTTON_A)) {
        vsync();
        frect(SCREEN,x*s,y*s,s,s,c1);
      }

      x+=xi;
      y+=yi;

      if(x<0) x=w-1;
      if(y<0) y=h-1;
      if(x>w-1) x=0;
      if(y>h-1) y=0;

      c1=pget(SCREEN,x*s,y*s);

      vsync();
      frect(SCREEN,x*s,y*s,s,s,c2);

    }

    for(int i=0;i<d;i++);
  
  } 
  
  return 0;
}

