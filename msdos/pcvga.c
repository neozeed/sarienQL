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
#include <dos.h>

#ifdef __WATCOMC__
#include <i86.h>
#endif

#include "sarien.h"
#include "graphics.h"
#define __TURBOC__

#ifdef __WATCOMC__
#define __outp(a, b)	outp(a, b)
#endif
#ifdef __TURBOC__
//#define __outp(a, b)	outportb(a, b)
#define __outp(a, b)	outp(a, b)
#endif

#ifdef __WATCOMC__
void DebugBreak(void);
//#pragma aux DebugBreak = "int 3" parm[];
#endif


extern struct gfx_driver *gfx;
extern struct sarien_options opt;

UINT8	*exec_name;
static UINT8	*screen_buffer;

#if 0
void	(__interrupt __far *prev_08)	(void);
void	__interrupt __far tick_increment	(void);
#else
static void (interrupt far * prev_08)(void);
//static void interrupt far tick_increment(int es,int ds,int di,int si,int bp,
//		int sp,int bx,int dx,int cx,int ax,int ip,int cs,int flags);
void interrupt far tick_increment(void);
#endif
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

static void pc_timer ()
{
	static UINT32 cticks = 0;

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
	union REGS r;
	int i;

	clock_count = 0;
	clock_ticks = 0;

	screen_buffer = calloc (GFX_WIDTH, GFX_HEIGHT);

	prev_08 = _dos_getvect (0x08);
	_dos_setvect (0x08, tick_increment);

	memset (&r, 0x0, sizeof(union REGS));
#ifdef __MICROSOFT__
	r.w.ax = 0x13;
	int386 (0x10, &r, &r);
#endif

#ifdef __TURBOC__
	r.x.ax = 0x13;
	int86 (0x10, &r, &r);
#endif

	__outp (0x3c8, 0);
	for (i = 0; i < 32 * 3; i++)
		__outp (0x3c9, palette[i]);

	return err_OK;
}


static int pc_deinit_vidmode ()
{
	union REGS r;

	memset (&r, 0x0, sizeof(union REGS));

#ifdef __MICROSOFT__
	r.w.ax = 0x03;
	int386 (0x10, &r, &r);
#endif

#ifdef __TURBOC__
	r.x.ax = 0x03;
	int86 (0x10, &r, &r);
#endif

	free (screen_buffer);
	_dos_setvect (0x08, prev_08);

	return err_OK;
}


/* blit a block onto the screen */
static void pc_put_block (int x1, int y1, int x2, int y2)
{
	unsigned int h, w, p;

	if (x1 >= GFX_WIDTH)  x1 = GFX_WIDTH  - 1;
	if (y1 >= GFX_HEIGHT) y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)  x2 = GFX_WIDTH  - 1;
	if (y2 >= GFX_HEIGHT) y2 = GFX_HEIGHT - 1;

	h = y2 - y1 + 1;
	w = x2 - x1 + 1;
	p = GFX_WIDTH * y1 + x1;

	while (h--) {
#ifdef __MICROSOFT__
		/* Watcom uses linear 0xa0000 address */
		memcpy ((UINT8 *)0xa0000 + p, screen_buffer + p, w);
#else
		/* Turbo C wants 0xa0000000 in seg:ofs format */
		_fmemcpy ((UINT8 far *)0xa0000000 + p, screen_buffer + p, w);
#endif
		p += 320;
	}
}


static void pc_put_pixels(int x, int y, int w, UINT8 *p)
{
	UINT8 *s;
 	for (s = &screen_buffer[y * 320 + x]; w--; *s++ = *p++);
}


static int pc_keypress ()
{
	return !!kbhit();
}


static int pc_get_key ()
{
	union REGS r;
	UINT16 key;

	memset (&r, 0, sizeof(union REGS));
#ifdef __MICROSOFT__
	int386 (0x16, &r, &r);
#endif
#ifdef __TURBOC__
	int86 (0x16, &r, &r);
#endif

	key = r.h.al == 0 ? (r.h.ah << 8) : r.h.al;

	return key;
}

/* WATCOM HATES timer routines... */
/* lucky we call no other routines inside our timer */
/* coz SS!=DS and watcom wants SS==DS but it aint inside a timer! */

#if 0
void __interrupt __far tick_increment (void)
#else
void interrupt far tick_increment(void)
#endif
{
	clock_ticks++;
	_chain_intr(prev_08);
}

cls(){}

