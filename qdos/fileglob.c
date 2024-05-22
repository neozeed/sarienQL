/*  Sarien - A Sierra AGI resource interpreter engine
 *  Copyright (C) 1999,2001 Stuart George and Claudio Matsuoka
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
/*#include <dos.h>*/

#include "sarien.h"
#include "agi.h"

extern char *drive;


int file_isthere (char *fname)
{
struct stat st;
int filep,flen,rc;

//
   _D(_D_WARN "file_isthere is checking %s\n",fname);
   rc=stat(fname,&st);
   printf("stat rc'd me a %d\n",rc);
   if(rc==0)   {
      return 1;
   }
   return 0;
}



char* file_name (char *fname)
{
return strdup(fname);
}



char *fixpath (int flag, char *fname)
{
	static char path[MAX_PATH];
	char *p;
	int j;

_D("fileglob.c fixpath %s",fname);

sprintf(path,"%s%s",drive,fname);

	if (flag==1) {
		_D("flag was set adding %s\n",game.name);
		strcat (path, game.name);
	}

for (j=0;j<strlen(path);j++)
	{
	if(path[j]=='.')
		path[j]='_';
	path[j]=toupper(path[j]);
	}

_D("\t%s\n",path);
	return path;
}