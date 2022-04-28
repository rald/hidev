#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "sweetie.h"
#include "bitmap.h"
#include "font.h"

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

#define W 10
#define H 10

typedef struct Cell Cell;

struct Cell {
  uint8 clr;
  uint8 obj;
};

Cell cells[W*H];

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

void dbmp(volatile uint16* srf,const uint16* bmp,int w,int h,int f,int x,int y,int t) {
  for(int j=0;j<h;j++) {
    for(int i=0;i<w;i++) {
      int k=bmp[i+j*w+f*w*h];
      if(k!=t) {
        uint16 l=rgb16(
          sweetie_palette[k*3+0],
          sweetie_palette[k*3+1],
          sweetie_palette[k*3+2]
        );
        pset(srf,x+i,y+j,l);    
      }
    }    
  }
}

void dchr(volatile uint16* srf,const uint16* fnt,int w,int h,int f,int x,int y,int s,uint16 c) {
  for(int j=0;j<h;j++) {
    for(int i=0;i<w;i++) {
      int k=fnt[i+j*w+f*w*h];
      if(k!=0) {
        frect(srf,x+i*s,y+j*s,s,s,c);    
      }
    }    
  }
}

int indexOf(char *l,char c) {
  int j=-1;
  for(int i=0;l[i];i++) {
    if(l[i]==c) {
      j=i;
      break;
    }
  }
  return j;
}

void dtxt(volatile uint16* srf,const uint16* fnt,int w,int h,int x,int y,int s,uint16 c,char *txt) {
  char *l="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int xc=x,yc=y;
  for(int i=0;txt[i];i++) {
    int j=indexOf(l,txt[i]);
    if(j!=-1) {
      dchr(srf,fnt,w,h,j,xc,yc,s,c);
    }
    xc+=w*s;
    if(xc+w*s>=SCREEN_W) {
      yc+=h*s;
      if(yc+h*s>=SCREEN_H) break;
    }
  }
}

uint16 clr(int c) {
  return rgb16(
    sweetie_palette[c*3+0],
    sweetie_palette[c*3+1],
    sweetie_palette[c*3+2]
  );
}

int main() {

  REG_DISPLAYCONTROL = VIDEOMODE_3 | BGMODE_2;

  unsigned int cnt=0;

  while(1) {

    vsync();
    
    frect(SCREEN,0,0,SCREEN_W,SCREEN_H,clr(0));

    dtxt(SCREEN,font_pixels,font_width,font_height,0,0,1,clr(12),"PRESS START BUTTON");

    while(!button_pressed(BUTTON_START)) {
      cnt++;
    }
    srand(cnt);

    vsync();
    frect(SCREEN,0,0,SCREEN_W,SCREEN_H,clr(0));

    for(int j=0;j<160/16;j++) {
      for(int i=0;i<240/16;i++) {
        dbmp(SCREEN,bitmap_pixels,bitmap_width,bitmap_height,rand()%4,i*16,j*16,15);        
        dbmp(SCREEN,bitmap_pixels,bitmap_width,bitmap_height,rand()%24+4,i*16,j*16,15);        
      }
    }

    while(!button_pressed(BUTTON_START));

  } 
  
  return 0;
}
