/**
*	created:		2013-3-22   19:30
*	filename: 		FKVsVer8Define
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <stdio.h>
#include <time.h>
#include "FKBaseDefine.h"
//------------------------------------------------------------------------
bool __cdecl _safe_vsnprintf (char *string,size_t count,const char *format,va_list ap);
//------------------------------------------------------------------------
#ifdef __STDC_WANT_SECURE_LIB__
#undef __STDC_WANT_SECURE_LIB__
#endif
//------------------------------------------------------------------------
#define		strncpy_s				strncpy_clds
#define		strcpy_s				strcpy_clds
#define		sprintf_s				sprintf_clds
#define		_strtime_s				_strtime_clds
#define		localtime_s				localtime_clds
#define		vsprintf_s				_vsnprintf
#define		sscanf_s				sscanf
#define		s_strncpy_s(d,s,nd)		strcpy_s(d,nd,s)
//------------------------------------------------------------------------
char* __cdecl	strncpy_clds (char * dest,size_t dsize,	const char * source,size_t count);
size_t __cdecl	strcpy_clds ( char * dest,  size_t dsize,  const char * source );
int __cdecl		sprintf_clds(char * dest, size_t dsize,const char * lpformat, ...);
void			fopen_clds(FILE **fh,const char* filename,const char* open);
void			_strtime_clds(char* p,int nsize);
struct tm *		localtime_clds(const tm *tm,const time_t *ptime);
//------------------------------------------------------------------------
#if (_MSC_VER >= 1400)
	#if (_MSC_VER >= 1500)
		#define stricmp		_stricmp
		#define strnicmp	_strnicmp
		#define strlwr		_strlwr
		#define tzset		_tzset
	#else
	#endif
#else
	#define		_itoa_s(n,s,slen,r)		itoa(n,s,r)
#endif
//------------------------------------------------------------------------