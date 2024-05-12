#include <qdos.h>
#include "keyboard.h"

//	208	up
//	216	down
//	192	left
//	200	right
//			{'\n',192,208,27,200,'\\',' ',216},
char keyMatrix[2][8][8]={
		{
			{KEY_F4,KEY_F1,'5',KEY_F2,KEY_F3,KEY_F5,'4','7'},
			{KEY_ENTER,KEY_LEFT,KEY_UP,KEY_ESCAPE,KEY_RIGHT,'\\',' ',KEY_DOWN},
			{']','z','.','c','b',0,'m','\''},
			{'[',0,'k','s','f','=','g',';'},
			{'l','3','h','1','a','p','d','j'},
			{'9','w','i',0,'r','-','y','o'},
			{'8','2','6','q','e','0','t','u'},
			{0,0,0,'x','v','/','n',','}
		},
		{
			{KEY_F4,KEY_F1,'%',KEY_F2,KEY_F3,KEY_F5,'$','&'},
			{KEY_ENTER,KEY_LEFT,KEY_UP,KEY_ESCAPE,KEY_RIGHT,'|',' ',KEY_DOWN},
			{'}','Z','>','C','B','£','M','@'},
			{'}',0,'K','S','F','+','G',':'},
			{'L','£','H','!','A','P','D','J'},
			{'(','W','I',0,'R','_','Y','O'},
			{'*','"','^','Q','E',')','T','U'},
			{0,0,0,'X','V','?','N','<'}
		}
	};

unsigned char scanKey()
{
	unsigned int i;
	int k=keyrow(7);
	unsigned int shift=k&1;
	unsigned int control=k&2;

	for(i=0;i<10;i++)
	{
		int key=keyrow(i);
		char k2;

		if(i==7) key&=0xFF; // Zap the shift and control key

		if(key)
		{
			for(k=7;k>=0;k--) if(key&(1<<k)) break;

			if(control&&(i==1)&&(k==1)) return KEY_BACKSPACE;

			k2=keyMatrix[shift][i][k];

			if(k2!=0) return k2;
			//else printf("%d %d\n",i,k);
		}
	}

	return 0;
}
