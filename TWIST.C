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

#undef USE_MODEY

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

byte far *framebuf[4];

#ifdef USE_MODEY
#define SETPIX(x,y,c) *(framebuf[(x) % 4]+((dword)SCREEN_WIDTH * (y) >> 2) + ((x) >> 2)) = c
#define GETPIX(x,y) *(framebuf[(x) % 4]+((dword)SCREEN_WIDTH * (y) >> 2) + ((x) >> 2))
#else
#define SETPIX(x,y,c) *(framebuf[0] + (dword)SCREEN_WIDTH * (y) + (x)) = c
#define GETPIX(x,y) *(framebuf[0] + (dword)SCREEN_WIDTH * (y) + (x))
#endif

#ifndef M_PI
#define M_PI 3.14159
#endif

unsigned char SIN256[256];
unsigned char SIN4096[4096];

void init_sin()
{
  int i;
  for( i = 0; i < 256; ++i ) {
    SIN256[ i ] = 255 * ( (sin( 2.0 * M_PI * i / 255.0 ) + 1.0 ) / 2.0 );
  }
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
    a  = SIN4096[(t * 16 + y * 2 + 1024) % 4096] * w >> 2;
    x1 = SIN4096[(a +    0) % 4096] >> 2;
    x2 = SIN4096[(a + 1024) % 4096] >> 2;
    x3 = SIN4096[(a + 2048) % 4096] >> 2;
    x4 = SIN4096[(a + 3072) % 4096] >> 2;
    xm = x0 + (SIN4096[(t * 16 + y * 8 + 1024) % 4096] >> 3);
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

#ifdef USE_MODEY
  set_mode_y();
  front = vga_page[0];
  back  = vga_page[1];
#else
  set_graphics_mode();
#endif

  set_palette( fire_pal );
#ifdef USE_MODEY
  for( i = 0; i < 4; ++i ) {
    framebuf[i] = malloc( SCREEN_WIDTH*SCREEN_HEIGHT );
    if( framebuf[i] == NULL ) {
      printf("not enough memory\n");
      return 1;
    }
    memset( framebuf[i], 0x00, SCREEN_WIDTH*SCREEN_HEIGHT );
  }
#else
  framebuf[0] = malloc( SCREEN_WIDTH*SCREEN_HEIGHT );
  if( framebuf[0] == NULL ) {
    printf("not enough memory\n");
    return 1;
  }
  memset( framebuf[0], 0x00, SCREEN_WIDTH*SCREEN_HEIGHT );
#endif

  while(!kbhit())
  {
#ifdef USE_MODEY
    for(i = 0; i < 4; ++i) {
      memset( framebuf[i], 0x00, SCREEN_WIDTH*SCREEN_HEIGHT );
    }
    draw_twist( 100, 0, 128, 200, frame );
    blit2page( framebuf, back, 100, 0, 128, 200);
    wait_for_retrace();
    page_flip( &front, &back );
#else
    memset( framebuf[0], 0x00, SCREEN_WIDTH*SCREEN_HEIGHT );
    draw_twist( 100, 0, 128, 200, frame );
    wait_for_retrace();
    memcpy( VGA, framebuf[0], SCREEN_WIDTH*SCREEN_HEIGHT );
#endif
    frame++;
  }
  getch();
  set_text_mode();
  return 0;
}
