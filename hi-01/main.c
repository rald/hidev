#include <stdlib.h>

#define RGB16(r,g,b)  ((r)+(g<<5)+(b<<10)) 

int main(void) {

	unsigned short* Screen = (unsigned short*)0x6000000; 
	*(unsigned long*)0x4000000 = 0x403; // mode3, bg2 on 
	unsigned long* Clock = (unsigned long*)0x4000100; 



  srand(*Clock);

  int x=0,y=0;
  int xi=0,yi=0;
  int c,r,g,b;
  int d=0;

  x=rand()%240;
  y=rand()%160;

  r=rand()%32;
  g=rand()%32;
  b=rand()%32;
      
  c=RGB16(r,g,b);

  xi=rand()%2?1:-1;
  yi=rand()%2?1:-1;

  for(int j=0;j<160;j++) {   
		for(int i=0;i<240;i++) {
			Screen[i+j*240] = RGB16(0,0,31);  
		}
	}


	while(1) {
  	
    Screen[x+y*240]=c;

    if(x+xi<0 || x+xi>=240) xi=-xi;
    if(y+yi<0 || y+yi>=160) yi=-yi;

    x+=xi;
    y+=yi;

    d++;

    if(d>=64) {
      d=0;

      r=rand()%32;
      g=rand()%32;
      b=rand()%32;
          
      c=RGB16(r,g,b);
    }


	}

}
