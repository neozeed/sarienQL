/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999-2003 Stuart George and Claudio Matsuoka
 *
 *  $Id: main.c,v 1.87 2003/09/02 01:10:55 cmatsuoka Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; see docs/COPYING for further details.
 */

#include <stdio.h>
#include <string.h>

#ifdef __QDOS__
#include <qdos.h>
#include <qptr.h>
#include <sms.h>
#include <fcntl.h>
#include <ansicondef.h>
extern int ql_kbhit;
extern int ql_charin;
int pc_get_key(void);


/* C68 Specifics */
char _prog_name[] = "Sarien 0.8";
int (*_cmdparams)()=NULL;
long (*_cmdchannels)()=NULL;
int (*_cmdwildcard)()=NULL;
long (*_stackchannels)()=NULL;
long _stack = 20L * 1024L;

/* set up definition of CONsole Window */
struct WINDOWDEF _condetails = {
     2,    /* Border Color */
     1,    /* Border Width */
     0,    /* Paper        */
     6,    /* Ink          */
     512,  /* Width        */
     256,  /* Height       */
     0,    /* X origin     */
     0     /* Y origin     */
};

// From SimonGreenaway's invaders
char *drive="WIN1_"; // Used to tell code from where to load image libs and screens.
char *rom;	// For future development (dual screen).
float qdos=-1;	//    "
int job=-1;	//    "
int convert=-1; //    "
#endif

#ifdef HAVE_ALLEGRO
#include <allegro.h>
#endif

#include "sarien.h"
#include "agi.h"
#include "text.h"
#include "graphics.h"
#include "sprite.h"

extern int optind;

struct sarien_options opt;
struct game_id_list game_info;
struct agi_game game;

#if (!defined(_TRACE) && !defined(__GNUC__))
INLINE void _D (char *s, ...) { s = s; }
#endif


#ifdef MACOSX
#  ifdef MACOSX_SDL
#    define main SDL_main     
#  else
     /* real main for OS X is in src/graphics/cocoa/cocoa.m */
#    define main gamemain
#  endif
#endif


int main (int argc, char *argv[])
{
	int ec;

	/* we must do this before _ANYTHING_ else if using allegro!! */
#ifdef HAVE_ALLEGRO
	allegro_init ();
	install_keyboard ();
#endif

#ifdef __MSDOS__
	exec_name = strdup(argv[0]);
#endif
#ifdef __QDOS__
void *sv;
long ver;
int s;
char fbuf[20];
char tbuf[20];
 __ANSICONF__.emulation = VT100; 
 __ANSICONF__.csi = 0; 
 _conwrite = ANSI_conwrite;
	#if 0
sms_info(&sv,&ver);
setSysBase(sv);
	for(s=1;s<argc;s++)
	{
		if(strcmp(argv[s],"-c")==0) convert=1;
		else if(strcmp(argv[s],"-d")==0) drive=argv[++s];
		else if(strcmp(argv[s],"-rom")==0) rom=argv[++s];
		else if(strcmp(argv[s],"-qdos")==0) qdos=atof(argv[++s]);
		else if(strcmp(argv[s],"-job")==0) job=atoi(argv[++s]);
		else
		{
			printf("Unknown command line argument: %s\n",argv[s]);
			exit(4);	
		}
	}
	#endif

/*this is terrible. sorry.*/
   s=-1;
   while(s!=1)   {
   cls();
   printf("compiled drive was [%s]\n",drive);
   printf("enter drive!\n");
   gets(fbuf);
   if(strlen(fbuf)>0)
      drive=fbuf;
   printf("\n\n");
   printf("main.c: drive is \n[%s]\n",drive);
   sprintf(tbuf,"%s%s",drive,"AGI");
   s=file_isthere(tbuf);
   if(s!=1) {
      printf("\nfile %s not found!\npress Enter to try again\n\n>",tbuf);
      getch();
      printf("\033[2J\033[1;1H");
      }
   }
while(ql_kbhit){
     pc_get_key();
}
sleep(1);
ql_kbhit=0;
#endif

	game.clock_enabled = FALSE;
	game.state = STATE_INIT;

	opt.scale = 1;

#ifdef USE_COMMAND_LINE
	if ((ec = parse_cli (argc, argv)) != err_OK)
		goto bail_out;

	if (opt.gamerun == GAMERUN_CRC) {
		char name[80];
		agi_detect_game (argc > 1 ? argv[optind] :
			get_current_directory ());
		printf ("CRC: 0x%x (Ver 0x%x)\n", game.crc, game.ver);
		if (match_crc (game.crc, get_config_file(), name, 80))
			printf(" AGI game detected: %s\n", name);
		else
			printf(" Unknown game (config file: %s)\n",
				get_config_file());
		exit (0);
	}

	if (opt.gamerun == GAMERUN_GAMES) {
		list_games ();
		exit (0);
	}
#ifdef OPT_LIST_OBJECTS
	if (opt.gamerun == GAMERUN_OBJECTS) {
		agi_detect_game(argc > 1 ? argv[optind] : get_current_directory());
		agi_init();
		show_objects();

		/* errgh!! ugly goto */
		goto bail_out;
	}
#endif
#ifdef OPT_LIST_DICT
	if (opt.gamerun == GAMERUN_WORDS){
	    agi_detect_game(argc > 1 ? argv[optind] : get_current_directory());
	    agi_init();
	    show_words();

	    goto bail_out;
	}
#endif
#endif

	init_machine (argc, argv);

	game.color_fg = 15;
	game.color_bg = 0;

	if ((game.sbuf = calloc (_WIDTH, _HEIGHT)) == NULL) {
		ec = err_NotEnoughMemory;
		goto bail_out;
	}
#ifdef USE_HIRES
	if ((game.hires = calloc (_WIDTH * 2, _HEIGHT)) == NULL) {
		ec = err_NotEnoughMemory;
		goto bail_out_2;
	};
#endif

	if (init_sprites () != err_OK) {
		ec = err_NotEnoughMemory;
		goto bail_out_3;
	}

	if (init_video () != err_OK) {
		ec = err_Unk;
		goto bail_out_4;
	}


#ifdef OPT_PICTURE_VIEWER
	if (opt.gamerun == GAMERUN_PICVIEW) {
		console.y = 0;
		if (agi_detect_game (argc > 1 ? argv[optind] :
			get_current_directory ()) == err_OK)
		{
			agi_init ();
			view_pictures ();
		}

		goto bail_out;
	}
#endif

#if !defined (__MSDOS__)
	/* printf() breaks GCC 3.0 build */
	fprintf (stdout,
"TITLE VERSION - A Sierra AGI resource interpreter engine.\n"
"Copyright (C) 1999-2003 Stuart George\n"
"Portions Copyright (C) 1998 Lance Ewing, (C) 1999 Felipe Rosinha,\n"
" (C) 1999-2003 Claudio Matsuoka, (C) 1999-2001 Igor Nesterov,\n"
" (C) 2001,2002 Vasyl Tsvirkunov, (C) 2001,2002 Thomas Akesson\n"
"QL-DOS 2-D code by Simon Greenaway\n" /* https://github.com/SimonGreenaway/QL-sprites */
"QL-DOS timer code by mk79 (Marcel Kilgus)\n"	/* https://www.qlforum.co.uk/viewtopic.php?p=45918&sid=ab2363b94730ea40d1006626996fc886#p45918 */		
#ifdef SCALE2X
"Scale2x Copyright (C) 2001-2002 Andrea Mazzoleni\n"
#endif
#ifndef HAVE_GETOPT_LONG
"Portions Copyright (C) 1989-1997 Free Software Foundation, Inc.\n"
#endif
"\n"
"This program is free software; you can redistribute it and/or modify it\n"
"under the terms of the GNU General Public License, version 2 or later,\n"
"as published by the the Free Software Foundation.\n"
"\n");
#endif

	report ("Enabling interpreter console\n");
	console_init ();
	report ("--- Starting console ---\n\n");
	if (!opt.gfxhacks)
		report ("Graphics driver hacks disabled (if any)\n");

	game.ver = -1;		/* Don't display the conf file warning */

	_D ("Detect game");
	if (agi_detect_game (argc > 1 ? argv[optind] :
		get_current_directory ()) == err_OK)
	{
		game.state = STATE_LOADED;
		_D (_D_WARN "game loaded");
	} else {
		if (argc > optind) {
			report ("Could not open AGI game \"%s\".\n\n",
				argv[optind]);
		}
	}

	_D ("Init sound");
	init_sound ();

	report (" \nSarien VERSION is ready.\n");
	if (game.state < STATE_LOADED) {
       		console_prompt ();
		do { main_cycle (); } while (game.state < STATE_RUNNING);
		if (game.ver < 0) game.ver = 0;	/* Enable conf file warning */
	}

	ec = run_game ();

	deinit_sound ();
	deinit_video ();

bail_out_4:
	deinit_sprites ();
bail_out_3:
#ifdef USE_HIRES
	free (game.hires);
#endif
bail_out_2:
	free (game.sbuf);
bail_out:
	if (ec == err_OK || ec == err_DoNothing) {
		deinit_machine ();
		exit (ec);
	}

	printf ("Error %04i: ", ec);

	switch (ec) {
	case err_BadCLISwitch:
		printf("Bad CLI switch.\n");
		break;
	case err_InvalidAGIFile:
		printf("Invalid or inexistent AGI file.\n");
		break;
	case err_BadFileOpen:
		printf("Unable to open file.\n");
		break;
	case err_NotEnoughMemory:
		printf("Not enough memory.\n");
		break;
	case err_BadResource:
		printf("Error in resource.\n");
		break;
	case err_UnknownAGIVersion:
		printf("Unknown AGI version.\n");
		break;
	case err_NoGameList:
		printf("No game ID List was found!\n");
		break;
	}
	printf("\nUse parameter -h to list the command line options\n");

	deinit_machine ();

#ifdef HAVE_ALLEGRO
	remove_keyboard();
#endif
#ifdef __MSDOS__
	free(exec_name);
#endif

	return ec;
}
#ifdef HAVE_ALLEGRO
END_OF_MAIN()
#endif

