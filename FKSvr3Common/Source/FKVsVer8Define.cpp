/**
*	created:		2013-4-8   15:00
*	filename: 		FKVsVer8Define
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKVsVer8Define.h"
#include <time.h>
#include "../Include/FKStringEx.h"
//------------------------------------------------------------------------
size_t openbsd_strlcat(char *dst, const char *src, size_t siz){
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;


	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src)); 
}
//------------------------------------------------------------------------
size_t openbsd_strlcpy(char *dst, const char *src, size_t siz){
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	if (n == 0) {
		if (siz != 0)
			*d = '\0'; 
		while (*s++)
			;
	}

	return(s - src - 1);  
}
//------------------------------------------------------------------------
bool __cdecl _safe_vsnprintf (char *string,size_t count,const char *format,va_list ap)
{
	__try
	{
		_vsnprintf(string, count, format, ap);	
		string[count-1]='\0';
		return true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		sprintf_s(string, count,"[ERROR:_vsnprintf()“Ï≥£] %s", format);
	}
	return false;
}
//------------------------------------------------------------------------
char* __cdecl strncpy_clds (char * dest,
							size_t dsize,
							const char * source,
							size_t count)
{
	if (dest && source){
		char* pret=strncpy(dest,source,min(dsize,count));
		dest[dsize-1]='\0';
		return pret;
	}
	return 0;
}
//------------------------------------------------------------------------
size_t __cdecl strcpy_clds (char * dest,
							size_t dsize,
							const char * source)
{
	if (dest && source){
		size_t nret= openbsd_strlcpy(dest,source,dsize);
		dest[dsize-1]='\0';
		return nret;
	}
	return 0;
}
//------------------------------------------------------------------------
int __cdecl sprintf_clds(char * dest, size_t dsize,const char * lpformat, ...){
	va_list argList;
	va_start(argList, lpformat);
	int nRet=_vsnprintf(dest,dsize,lpformat, argList);
	va_end(argList);
	dest[dsize-1]='\0';
	return nRet;
}
//------------------------------------------------------------------------
void fopen_clds(FILE **fh,const char* filename,const char* open)
{
	(*fh)=fopen(filename,open);
}
//------------------------------------------------------------------------
void _strtime_clds(char* p,int nsize){
	char szBuffer[128]={0};
	_strtime(szBuffer);
	openbsd_strlcpy(p,szBuffer,nsize);
	p[nsize-1]='\0';
}
//------------------------------------------------------------------------
struct tm *  localtime_clds(const tm *ptm,const time_t *ptime)
{
	tm *  ptemptm=NULL;
	if (ptime){
		ptemptm=localtime(ptime);
		if (ptemptm) {
			memcpy((void*)ptm,(void*)ptemptm,sizeof(*ptm));
		}
	}
	return ptemptm;
};
//------------------------------------------------------------------------