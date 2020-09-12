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

byte far *framebuf;

#define SETPIX(x,y,c) *(framebuf + (dword)SCREEN_WIDTH * (y) + (x)) = c
#define GETPIX(x,y) *(framebuf + (dword)SCREEN_WIDTH * (y) + (x))

#ifndef M_PI
#define M_PI 3.14159
#endif

unsigned char SIN4096[4096];

void init_sin()
{
  int i;
  for( i = 0; i < 4096; ++i ) {
    SIN4096[ i ] = 255 * ( (sin( 2.0 * M_PI * i / 4095.0 ) + 1.0 ) / 2.0 );
  }
}

void hline( word x0, word x1, word y, byte col )
{
  int i;
  if( y >= SCREEN_HEIGHT ) {
    return;
  }
  for( i = x0; i < x1 && i < SCREEN_WIDTH; ++i ) {
    SETPIX( i, y, col );
    col++;
  }
}

void draw_twist( word x0, word y0, word w, word h, word t )
{
  word a;
  word y, x1, x2, x3, x4, xm;

  for( y = y0; y < y0 + h; ++y ) {
    a  =       SIN4096[(t * 16 + y * 2) % 4096] * (w >> 2);
    xm = x0 + (SIN4096[(t * 16 + y * 8) % 4096] >> 3);
    x1 = xm + (SIN4096[(a +    0) % 4096] >> 2);
    x2 = xm + (SIN4096[(a + 1024) % 4096] >> 2);
    x3 = xm + (SIN4096[(a + 2048) % 4096] >> 2);
    x4 = xm + (SIN4096[(a + 3072) % 4096] >> 2);
    if(x1 < x2) {
      hline( x1, x2, y, 33);
    }
    if(x2 < x3) {
      hline( x2, x3, y, 49);
    }
    if(x3 < x4) {
      hline( x3, x4, y, 65);
    }
    if(x4 < x1) {
      hline( x4, x1, y, 81);
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

  set_graphics_mode();
  set_palette( fire_pal );
  framebuf = malloc( SCREEN_WIDTH*SCREEN_HEIGHT );
  if( framebuf == NULL ) {
    printf("not enough memory\n");
    return 1;
  }
  memset( framebuf, 0x00, SCREEN_WIDTH*SCREEN_HEIGHT );

  while(!kbhit())
  {
    memset( framebuf, 0x00, SCREEN_WIDTH*SCREEN_HEIGHT );
    draw_twist( 100, 0, 128, 200, frame );
    wait_for_retrace();
    memcpy( VGA, framebuf, SCREEN_WIDTH*SCREEN_HEIGHT );
    frame++;
  }
  getch();
  set_text_mode();
  return 0;
}
