/**
*	created:		2013-3-22   20:03
*	filename: 		FKDumpError
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <Dbghelp.h>
#include "../FKThread.h"
//------------------------------------------------------------------------
LONG WINAPI my_level_exception_filter(struct _EXCEPTION_POINTERS *ExceptionInfo);
//------------------------------------------------------------------------
typedef void (*DumpStackCallback)(void* param,const char* szFormat,...);
typedef DWORD( WINAPI *pOnExceptionEndCallBack)(int dumpret,const char* dumpfilename,const char* logfilename,const char* codefiledir);
typedef DWORD( WINAPI *pGetModuleFileNameFunc)(IN HMODULE hModule, OUT LPSTR lpFilename,IN DWORD nSize);
typedef DWORD( WINAPI *pOnExceptionBeginCallBack)(LPEXCEPTION_POINTERS ExceptionInfo);
//------------------------------------------------------------------------
extern unsigned int	minidump_type;
extern pGetModuleFileNameFunc g_geterrormodulefilename;
extern  char * g_dumpbasepath;
extern pOnExceptionBeginCallBack g_onexceptionbegincallback;
extern pOnExceptionEndCallBack g_onexceptionendcallback;
//------------------------------------------------------------------------