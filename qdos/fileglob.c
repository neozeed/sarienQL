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
/*#include <dos.h>*/

#include "sarien.h"
#include "agi.h"


int file_isthere (char *fname)
{
FILE *f;
_D("fileglob.c: file_isthere %s\n",fname);
f=fopen(fname,"rb");
if(f>0)
	return 1;
else return 0;
}



char* file_name (char *fname)
{
char filename[25];
//	return ("fileglob.c: file_name fail none");
sprintf(filename,"%s",fname);
return(filename);
}



char *xfixpath (int flag, char *fname)
{
	static char path[MAX_PATH];
	char *p;

_D("fileglob.c fixpath %s",fname);

    	strcpy (path, game.dir);

	if (*path && (path[strlen(path)-1]!='\\' && path[strlen(path)-1] != '/'))
	{
		if(path[strlen(path)-1]==':')
			strcat(path, "./");
		else
			strcat(path, "/");
	}

	if (flag==1)
		strcat (path, game.name);

	strcat (path, fname);

	p = path;

	while(*p) {
		if (*p=='\\')
		    *p='/';
		p++;
	}
_D("\t%s\n",path);
	return path;
}


char *fixpath (int flag, char *fname)
{
	static char path[MAX_PATH];
	char *p;
	int j;

_D("fileglob.c fixpath %s",fname);

sprintf(path,"WIN3_%s",fname);

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