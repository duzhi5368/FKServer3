/**
*	created:		2013-3-22   19:24
*	filename: 		FKMisc
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKBaseDefine.h"
#include "zlib/zlib.h"
//------------------------------------------------------------------------
bool	SetClipboardText(const char* lpszBuffer);
#define Copy2Clipboard(x)  SetClipboardText(x)
char *	getpasswd(const char *prompt,char * szInputbuf,int nbuflen,char chpass='*');
//------------------------------------------------------------------------
int		compresszlib(unsigned char* pIn,unsigned long nInLen,unsigned char* pOut,unsigned long& pOutLen,int level=Z_DEFAULT_COMPRESSION );
int		uncompresszlib(unsigned char* pIn,unsigned long nInLen,unsigned char* pOut,unsigned long& pOutLen);
//------------------------------------------------------------------------
unsigned int _random(unsigned int nMax,unsigned int nMin=0);
unsigned int _random_d(unsigned int nMax,unsigned int nMin=0);
__inline void random_full(void* pbuf,int nsize){
	for (int i = 0; i < (nsize/4); i++) {
		*((DWORD*)pbuf) = (DWORD)_random(0xffffffff, 0);
		pbuf=((char*)pbuf)+4;
	}
	int nmod=nsize%4;
	if (nmod>0){
		DWORD nrand=(DWORD)_random(0xffffffff, 0);
		memcpy(pbuf,&nrand,nmod);
	}
}
//------------------------------------------------------------------------