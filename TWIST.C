#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <conio.h>
#include <dos.h>
#include <math.h>
#include <mem.h>
#include <time.h>

#include "types.h"
#include "vga.h"
#include "pal.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

byte far *framebuf[4];

#define SETPIX(x,y,c) *(framebuf[(x) % 4]+((dword)SCREEN_WIDTH * (y) >> 2) + ((x) >> 2)) = c
#define GETPIX(x,y) *(framebuf[(x) % 4]+((dword)SCREEN_WIDTH * (y) >> 2) + ((x) >> 2))

#ifndef M_PI
#define M_PI 3.14159
#endif

unsigned char SIN256[256];
unsigned char SIN4096[4096];
unsigned char COS4096[4096];

void init_sin()
{
  int i;
  for( i = 0; i < 256; ++i ) {
    SIN256[ i ] = 256 * ( (sin( 2.0 * M_PI * i / 255.0 ) + 1.0 ) / 2.0 );
  }
  for( i = 0; i < 4096; ++i ) {
    SIN4096[ i ] = 256 * ( (sin( 2.0 * M_PI * i / 4095.0 ) + 1.0 ) / 2.0 );
    COS4096[ i ] = 256 * ( (cos( 2.0 * M_PI * i / 4095.0 ) + 1.0 ) / 2.0 );
  }
}

void hline( word x0, word x1, word y, byte col )
{
  int i;
  if( y >= 200 )
    return;
  for( i = x0; i < x1 && i < 320; ++i ) {
    SETPIX( i, y, col );
    if(i % 2) col++;
  }
}

void draw_twist( word x0, word y0, word w, word h, word t )
{
  word a;
  word s1, s2, s3, s4;
  word y, x1, x2, x3, x4, xm;

  for( y = y0; y < y0 + h; ++y ) {
    a  = COS4096[(t * 64 + y * 2) % 4096] * 8;
    s1 = SIN4096[(a +    0) % 4096] / 4;
    s2 = SIN4096[(a + 1024) % 4096] / 4;
    s3 = SIN4096[(a + 2048) % 4096] / 4;
    s4 = SIN4096[(a + 3072) % 4096] / 4;
    x1 = w/2-32 + s1;
    x2 = w/2-32 + s2;
    x3 = w/2-32 + s3;
    x4 = w/2-32 + s4;
    xm = x0 + COS4096[(t * 64 + y * 8) % 4096] / 8;
    if(x1 < x2) {
      hline( (xm + x1), (xm + x2), y, 33);
    }
    if(x2 < x3) {
      hline( (xm + x2), (xm + x3), y, 49);
    }
    if(x3 < x4) {
      hline( (xm + x3), (xm + x4), y, 65);
    }
    if(x4 < x1) {
      hline( (xm + x4), (xm + x1), y, 81);
    }
  }
}

int main()
{
  int i;
  word frame = 0;
  word front, back;

  randomize();
  frame = random(1024);
  init_sin();

  set_mode_y();
  front = vga_page[0];
  back  = vga_page[1];

  /* set_palette( fire_pal ); */
  for( i = 0; i < 4; ++i ) {
    framebuf[i] = malloc( SCREEN_WIDTH*SCREEN_HEIGHT );
    if( framebuf[i] == NULL ) {
      printf("not enough memory\n");
      return 1;
    }
    memset( framebuf[i], 0x00, SCREEN_WIDTH*SCREEN_HEIGHT );
  }

  while(!kbhit())
  {
    for(i = 0; i < 4; ++i) {
      memset( framebuf[i], 0x00, SCREEN_WIDTH*SCREEN_HEIGHT );
    }
    draw_twist( 100, 0, 120, 200, frame );
    blit2page( framebuf, back, 100, 0, 120, 200);
    wait_for_retrace();
    page_flip( &front, &back );
    frame++;
  }

  set_text_mode();
  return 0;
}
