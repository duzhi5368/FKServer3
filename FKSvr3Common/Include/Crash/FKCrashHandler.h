/**
*	created:		2013-4-8   12:59
*	filename: 		FKCrashHandler
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
bool HookCrashReporter(bool logon);
//------------------------------------------------------------------------
#ifdef WIN32
//------------------------------------------------------------------------
#include <Windows.h>
#include <DbgHelp.h>
#include "FKStackWalker.h"
//------------------------------------------------------------------------
class CStackWalker : public StackWalker
{
public:
	void OnOutput(LPCSTR szText);
	void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName);
	void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion);
	void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry);
	void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr);
};
//------------------------------------------------------------------------
void StartCrashHandler();

typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
int __cdecl HandleCrash(PEXCEPTION_POINTERS pExceptPtrs);

void SetUnhandledCrashFilter();
//------------------------------------------------------------------------
#define THREAD_TRY_EXECUTION __try 
#define THREAD_HANDLE_CRASH  __except(HandleCrash(GetExceptionInformation())) {}
//------------------------------------------------------------------------
#ifdef _DEBUG
	#define THREAD_TRY_EXECUTION2 __try {
	#define THREAD_HANDLE_CRASH2  } __except(HandleCrash(GetExceptionInformation())) {}
#else
	#define THREAD_TRY_EXECUTION2 __try {
	#define THREAD_HANDLE_CRASH2  } __except(HandleCrash(GetExceptionInformation())) {}
#endif
//------------------------------------------------------------------------
#else

#define THREAD_TRY_EXECUTION 
#define THREAD_HANDLE_CRASH 

#define THREAD_TRY_EXECUTION2 ;
#define THREAD_HANDLE_CRASH2 ;

#endif
//------------------------------------------------------------------------