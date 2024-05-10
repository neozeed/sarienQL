/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id: pcvga.c,v 1.5 2002/11/16 01:20:21 cmatsuoka Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <qdos.h>
#include <sys/qlib.h>

/////////////////////////////////////////////////////////
//	stolen from
//	https://github.com/SimonGreenaway/QL-sprites
typedef void * screen;
screen SCREEN;
unsigned char *addresses[256];
unsigned short bits[8];

#define ADDRESS(screen,x,y) (((unsigned short *)screen)+y*64+x/4)
#define ADDRESS4(screen,x,y) (((unsigned short *)screen)+y*64+x/8)

void cls(screen screen)
{
        memset((unsigned char *)screen,0,32768);
}

// Return the 16 bit pixel data for the 4 pixels containing this pixel

unsigned short peek(screen screen,unsigned int y,unsigned int x)
{
        unsigned short *address=(unsigned short *)screen;
        unsigned short data;
        address+=y*64+(x>>2);

        data=*address;

        switch(x&3)
        {
                case 3: return data&0x0203;
                case 2: return data&0x080C;
                case 1: return data&0x2030;
                case 0: return data&0x80C0;
        }
}

const unsigned short masks[]={0x3F3F,0xCFCF,0xF3F3,0xFCFC};

const unsigned short colours[4][8]={
				{0,1<<6,2<<6,3<<6,512<<6,513<<6,514<<6,515<<6},
				{0,1<<4,2<<4,3<<4,512<<4,513<<4,514<<4,515<<4},
				{0,1<<2,2<<2,3<<2,512<<2,513<<2,514<<2,515<<2},
				{0,1,2,3,512,513,514,515}
				};

const unsigned short masks4[]={0x0101,0x202,0x404,0x808,0x1010,0x2020,0x4040,0x8080};
const unsigned short colours4[8][4]={
					{0,0x1<<7,0x100<<7,0x101<<7},
					{0,0x1<<6,0x100<<6,0x101<<6},
					{0,0x1<<5,0x100<<5,0x101<<5},
					{0,0x1<<4,0x100<<4,0x101<<4},
					{0,0x1<<3,0x100<<3,0x101<<3},
					{0,0x1<<2,0x100<<2,0x101<<2},
					{0,0x1<<1,0x100<<1,0x101<<1},
					{0,0x1,0x100,0x101}
				   };

const unsigned char shifts[]={6,4,2,0};

// Plot a point in the given colour
//
// (1) Find address of the 16bit area containing the require colour data
// (2) Mask out the bit with zeros
// (3) Set the colour via a pre-shifted 16bit value for each colour
//
// Can't see a way to speed this up. We need to read the screen, mask it with an AND,OR in the new colour and then write back to the screen.

inline void plot(screen screen,unsigned int x,unsigned int y,unsigned char c)
{
	register unsigned short *address=ADDRESS(screen,x,y);

	*address=(*address&masks[x&3])|colours[x&3][c];
}

inline void plot4(screen screen,unsigned int x,unsigned int y,unsigned char c)
{
	unsigned short *address=ADDRESS4(screen,x,y);

	if(x>511)	{	puts("X high"); exit(0); }
	if(y>511)	{	puts("Y high"); exit(0); }

	if(((unsigned int)address<0x20000)||((unsigned int)address>0x28000))
	{
		printf("Address OOR %X %d %d\n",(unsigned int)address,x,y);
		exit(2);
	}

	*address=(*address&masks4[x&7])|colours4[x&7][c];
}

// Return colour at the given screen location

unsigned int unplot(screen screen,unsigned short x,unsigned short y)
{
	unsigned short d=(*ADDRESS(screen,x,y)&~masks[x&3])>>shifts[x&3];

	return (d&3)+(d>256?4:0);
}

void init(unsigned int c)
{
	int i;
	short colours=c,mode=0;

	//background=(unsigned char *)createBuffer(256);
	//scratch=createBuffer(256);
	//secondAddress=(int)(background)-0x20000;

	mt_dmode(&colours,&mode);

	for(i=0;i<256;i++) addresses[i]=(unsigned char *)(i<<7);

	for(i=0;i<8;i++) bits[i]=(i&3)+(i&4)<<7;

	SCREEN=(screen)0x20000;
}
/////////////////////////////////////////////////////////



#include "sarien.h"
#include "graphics.h"

extern struct gfx_driver *gfx;
extern struct sarien_options opt;

UINT8	*exec_name;
static UINT8	*screen_buffer;

static int	pc_init_vidmode	(void);
static int	pc_deinit_vidmode	(void);
static void	pc_put_block		(int, int, int, int);
static void	pc_put_pixels		(int, int, int, UINT8 *);
static void	pc_timer		(void);
static int	pc_get_key		(void);
static int	pc_keypress		(void);


#define TICK_SECONDS 18

static struct gfx_driver gfx_pcvga = {
	pc_init_vidmode,
	pc_deinit_vidmode,
	pc_put_block,
	pc_put_pixels,
	pc_timer,
	pc_keypress,
	pc_get_key
};

// 0x6 white
// 0x4 green
// 0x2 red
// 0x0 black
#define AA 0x00
#define BB 0x02
#define CC 0x01
#define DD 0x03
static UINT8 cga_map[16] = {
	AA,		/*  0 - black */
	BB,		/*  1 - blue */
	BB,		/*  2 - green */
	BB,		/*  3 - cyan */
	CC,		/*  4 - red */
	CC,		/*  5 - magenta */
	CC,		/*  6 - brown */
	DD,		/*  7 - gray */
	AA,		/*  8 - dark gray */
	BB,		/*  9 - light blue */
	BB,		/* 10 - light green */
	BB,		/* 11 - light cyan */
	CC,		/* 12 - light red */
	CC,		/* 13 - light magenta */
	CC,		/* 14 - yellow */
	DD		/* 15 - white */
};


static void pc_timer ()
{
	static UINT32 cticks = 0;
//clock_ticks++;
	while (cticks == clock_ticks);
	cticks = clock_ticks;
}


int init_machine (int argc, char **argv)
{
	gfx = &gfx_pcvga;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}


static int pc_init_vidmode ()
{
	int i;

	clock_count = 0;
	clock_ticks = 0;

	init(4);
	screen_buffer = calloc (GFX_WIDTH, GFX_HEIGHT);
	opt.cgaemu = TRUE;

	poll_init(&clock_ticks);
	return err_OK;
}


static int pc_deinit_vidmode ()
{
	free (screen_buffer);

	return err_OK;
}


/* blit a block onto the screen */
static void pc_put_block (int x1, int y1, int x2, int y2)
{
UINT8 *s;
int xx,yy;
unsigned char pixel;
	
   s=&screen_buffer;
   for(yy=0;xx<GFX_HEIGHT-1;yy++)   {
      for(xx=0;xx<GFX_WIDTH-1;xx++)   {
         pixel = *s > 15 ? 0 : cga_map[*s];
         plot4(SCREEN,xx,yy,pixel);
         s++;
      }
   }
//clock_ticks++;
}


static void pc_put_pixels(int x, int y, int w, UINT8 *p)
{
	UINT8 *s;
	int x1;
	x1=0;
	s=&screen_buffer[y * 320 + x];
	for (;w>0;w--){
		unsigned char pixel;
		//pixel=(UINT8)&p;

		pixel = *p > 15 ? 0 : cga_map[*p];

		plot4(SCREEN,x+x1,y,pixel);
		x1++;
		p++;
		}
//clock_ticks++;
}


static int pc_keypress ()
{
//clock_ticks++;
}


static int pc_get_key ()
{
	UINT16 key;
//clock_ticks++;
	key=0;
	return key;
}

	