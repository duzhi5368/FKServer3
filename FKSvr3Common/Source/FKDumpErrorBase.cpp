/**
*	created:		2013-4-8   15:50
*	filename: 		FKDumpErrorBase
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#ifndef _WIN32_WINNT 
#define _WIN32_WINNT 0x500 
#endif
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/Dump/FKDumpErrorBase.h"
//------------------------------------------------------------------------
CThreadMonitor CThreadMonitor::m_thm;
//------------------------------------------------------------------------
int PrintThreadCallStack( DumpStackCallback Callback,int nPrintCount,void* param  ){
	return 0;
}
//------------------------------------------------------------------------
void CLD_OutputDebugString(const char* pszFmt,...){
	char szbuf[1024*8]={0};
	va_list ap;
	va_start( ap, pszFmt );
	_vsnprintf(&szbuf[0],sizeof(szbuf)-1,pszFmt,ap);
	va_end  ( ap );
	OutputDebugString(&szbuf[0]);
}
//------------------------------------------------------------------------