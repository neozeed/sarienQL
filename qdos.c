/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2001 Stuart George and Claudio Matsuoka
 *  
 *  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sarien.h"
#include "graphics.h"
#include "keyboard.h"

#include <qdos.h>
#include <sys/qlib.h>

/////////////////////////////////////////////////////////
//	stolen from
//	https://github.com/SimonGreenaway/QL-sprites
#define SCREEN ((char *)0x20000)
#define SCREEN2 ((char *)0x28000)
typedef void * screen;
#define ADDRESS(screen,x,y) (((unsigned short *)screen)+y*64+x/4)

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

// Return colour at the given screen location

unsigned int unplot(screen screen,unsigned short x,unsigned short y)
{
	unsigned short d=(*ADDRESS(screen,x,y)&~masks[x&3])>>shifts[x&3];

	return (d&3)+(d>256?4:0);
}
/////////////////////////////////////////////////////////

WINDOWDEF_t       windef;

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

static struct gfx_driver gfx_pccga = {
	pc_init_vidmode,
	pc_deinit_vidmode,
	pc_put_block,
	pc_put_pixels,
	pc_timer,
	pc_keypress,
	pc_get_key
};


static UINT8 cga_map[16] = {
	0x00,		/*  0 - black */
	0x01,		/*  1 - blue */
	0x01,		/*  2 - green */
	0x01,		/*  3 - cyan */
	0x02,		/*  4 - red */
	0x02,		/*  5 - magenta */
	0x02,		/*  6 - brown */
	0x03,		/*  7 - gray */
	0x00,		/*  8 - dark gray */
	0x01,		/*  9 - light blue */
	0x01,		/* 10 - light green */
	0x01,		/* 11 - light cyan */
	0x02,		/* 12 - light red */
	0x02,		/* 13 - light magenta */
	0x02,		/* 14 - yellow */
	0x03		/* 15 - white */
};

static void pc_timer ()
{
	clock_ticks+=20;
}


int init_machine (int argc, char **argv)
{
	gfx = &gfx_pccga;
	opt.cgaemu = TRUE;
	return err_OK;
}


int deinit_machine ()
{
	return err_OK;
}

static int dump_count;
static int dump;

static int pc_init_vidmode ()
{
	int i;

	clock_count = 0;
	clock_ticks = 0;
	screen_buffer = calloc (GFX_WIDTH, GFX_HEIGHT);
	opt.cgaemu = TRUE;

    poll_init(&clock_ticks);

cls(SCREEN);
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
#if 0
        unsigned int i, h, w, p, p2;
        UINT8 *fbuffer;
        UINT8 *sbuffer;

        if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
        if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
        if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
        if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

        y1 &= ~1;                       /* Always start at an even line */

        h = y2 - y1 + 1;
        w = (x2 - x1 + 1) / 4 + 1;
        p = 40 * y1 + x1 / 4;           /* Note: (GFX_WIDTH / 4) * (y1 / 2) */
        p2 = p + 40 * y1;

        for (i = 0; i < h; i += 2) {
	//	plot(SCREEN,x1,y1,sbuffer);
        }
#else
UINT8 *s;
int xx,yy;
unsigned char pixel;
	
	s=&screen_buffer;
   for(yy=0;xx<GFX_HEIGHT-1;yy++)   {
      for(xx=0;xx<GFX_WIDTH-1;xx++)   {
         pixel = *s > 15 ? 0 : cga_map[*s];
         plot(SCREEN,xx,yy,pixel);
         s++;
      }
   }
#endif
	clock_ticks++;
}


static void pc_put_pixels(int x, int y, int w, UINT8 *p)
{
	UINT8 *s;
	int x1;
	x1=0;
#if 1
// 	for (s = &screen_buffer[y * 320 + x]; w--; *s++ = *p++);
	s=&screen_buffer[y * 320 + x];
	for (;w>0;w--){
		unsigned char pixel;
		//pixel=(UINT8)&p;

		pixel = *p > 15 ? 0 : cga_map[*p];

		plot(SCREEN,x+x1,y,pixel);
		x1++;
		p++;
		}
#else
printf("pc_put_pixels x%d y%d w%d %d\n",x,y,w,(UINT8)&p);
#endif
clock_ticks++;
}


static int pc_keypress ()
{
	clock_ticks++;
	pc_timer ();
	return 0;
}


static int pc_get_key ()
{
	clock_ticks++;
	return 0;
}

void tick_increment (void)
{
	clock_ticks++;
}

void notprintf(char *p, ...)
{}


