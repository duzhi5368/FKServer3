/**
*	created:		2013-4-8   15:56
*	filename: 		FKDumpError
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <windows.h>
#include <wchar.h>
#include "../Include/Dump/FKDumpError.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include "../Include/FKWinFileIO.h"
#include "../Include/FKLogger.h"
//------------------------------------------------------------------------
unsigned int	minidump_type=MiniDumpNormal;
//------------------------------------------------------------------------
typedef BOOL
(WINAPI
 *tFMiniDumpWriteDump)(
 IN HANDLE hProcess,
 IN DWORD ProcessId,
 IN HANDLE hFile,
 IN MINIDUMP_TYPE DumpType,
 IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
 IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
 IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
 );
//------------------------------------------------------------------------
static tFMiniDumpWriteDump FMiniDumpWriteDump = NULL;
//------------------------------------------------------------------------
typedef BOOL
(WINAPI
 *tFSymInitialize)(
 IN HANDLE   hProcess,
 IN PSTR     UserSearchPath,
 IN BOOL     fInvadeProcess
 );
//------------------------------------------------------------------------
static tFSymInitialize FSymInitialize = NULL;
//------------------------------------------------------------------------
typedef BOOL
(WINAPI
 *tFStackWalk64)(
 DWORD                             MachineType,
 HANDLE                            hProcess,
 HANDLE                            hThread,
 LPSTACKFRAME64                    StackFrame,
 PVOID                             ContextRecord,
 PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
 PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
 PGET_MODULE_BASE_ROUTINE64        GetModuleBaseRoutine,
 PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
 );
//------------------------------------------------------------------------
static tFStackWalk64 FStackWalk64 = NULL;
//------------------------------------------------------------------------
typedef BOOL
(WINAPI
 *tFSymFromAddr)(
 IN  HANDLE              hProcess,
 IN  DWORD64             Address,
 OUT PDWORD64            Displacement,
 IN OUT PSYMBOL_INFO     Symbol
 );
//------------------------------------------------------------------------
tFSymFromAddr FSymFromAddr = NULL;
//------------------------------------------------------------------------
typedef BOOL
(WINAPI
 *tFSymGetLineFromAddr64)(
 IN  HANDLE                  hProcess,
 IN  DWORD64                 qwAddr,
 OUT PDWORD                  pdwDisplacement,
 OUT PIMAGEHLP_LINE64        Line64
 );
//------------------------------------------------------------------------
tFSymGetLineFromAddr64 FSymGetLineFromAddr64 = NULL;
//------------------------------------------------------------------------
typedef DWORD
(WINAPI
 *tFSymGetOptions)(
 VOID
 );
//------------------------------------------------------------------------
static tFSymGetOptions FSymGetOptions = NULL;
//------------------------------------------------------------------------
typedef DWORD
(WINAPI 
 *tFSymSetOptions)(
 IN DWORD   SymOptions
 );
//------------------------------------------------------------------------
static tFSymSetOptions FSymSetOptions = NULL;
//------------------------------------------------------------------------
typedef PVOID
(WINAPI 
 *tFSymFunctionTableAccess64)(
 HANDLE  hProcess,
 DWORD64 AddrBase
 );
//------------------------------------------------------------------------
static tFSymFunctionTableAccess64 FSymFunctionTableAccess64 = NULL;
//------------------------------------------------------------------------
typedef DWORD64
(WINAPI
 *tFSymGetModuleBase64)(
 IN  HANDLE              hProcess,
 IN  DWORD64             qwAddr
 );
//------------------------------------------------------------------------
static tFSymGetModuleBase64 FSymGetModuleBase64 = NULL;
//------------------------------------------------------------------------
static HMODULE sDbgHelpLib = NULL;
//------------------------------------------------------------------------
static bool InitDbgHelpLib()
{
	sDbgHelpLib = LoadLibrary("dbghelp.dll");
	if(NULL == sDbgHelpLib)
		return false;

	FMiniDumpWriteDump = (tFMiniDumpWriteDump)GetProcAddress(sDbgHelpLib,"MiniDumpWriteDump");
	FSymInitialize = (tFSymInitialize)GetProcAddress(sDbgHelpLib,"SymInitialize");
	FStackWalk64 = (tFStackWalk64)GetProcAddress(sDbgHelpLib,"StackWalk64");
	FSymFromAddr = (tFSymFromAddr)GetProcAddress(sDbgHelpLib,"SymFromAddr");
	FSymGetLineFromAddr64 = (tFSymGetLineFromAddr64)GetProcAddress(sDbgHelpLib,"SymGetLineFromAddr64");
	FSymGetOptions = (tFSymGetOptions)GetProcAddress(sDbgHelpLib,"SymGetOptions");
	FSymSetOptions = (tFSymSetOptions)GetProcAddress(sDbgHelpLib,"SymSetOptions");
	FSymFunctionTableAccess64 = (tFSymFunctionTableAccess64)GetProcAddress(sDbgHelpLib,"SymFunctionTableAccess64");
	FSymGetModuleBase64 = (tFSymGetModuleBase64)GetProcAddress(sDbgHelpLib,"SymGetModuleBase64");

	return (FMiniDumpWriteDump && 
		FSymInitialize && 
		FStackWalk64 && 
		FSymFromAddr &&
		FSymGetLineFromAddr64 && 
		FSymGetOptions && 
		FSymSetOptions && 
		FSymFunctionTableAccess64 &&
		FSymGetModuleBase64);
}
//------------------------------------------------------------------------
static void FreeDbgHelpLib()
{
	if(NULL != sDbgHelpLib){
		FreeLibrary(sDbgHelpLib);
		sDbgHelpLib = NULL;
	}

	FMiniDumpWriteDump = NULL;
	FSymInitialize = NULL;
	FStackWalk64 = NULL;
	FSymFromAddr = NULL;
	FSymGetLineFromAddr64 = NULL;
	FSymGetOptions = NULL; 
	FSymSetOptions = NULL;
	FSymFunctionTableAccess64 = NULL;
	FSymGetModuleBase64 = NULL;
}
//------------------------------------------------------------------------
INT CreateMiniDump(const char* szDumpFileName,LPEXCEPTION_POINTERS ExceptionInfo)
{
	if(!InitDbgHelpLib()){
		return 0;
	}
	if (szDumpFileName!=NULL){
		File f;
		if(File::Ok!=f.open(szDumpFileName,File::Write)){
			FreeDbgHelpLib();
			return 0;
		}
		HANDLE FileHandle	= f.getHandle();
		if( FileHandle ){
			MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;

			DumpExceptionInfo.ThreadId			= GetCurrentThreadId();
			DumpExceptionInfo.ExceptionPointers	= ExceptionInfo;
			DumpExceptionInfo.ClientPointers	= true;

			FMiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), FileHandle, (MINIDUMP_TYPE)minidump_type, (ExceptionInfo!=NULL)?&DumpExceptionInfo:NULL, NULL, NULL );
			f.close();
		}
	}
	FreeDbgHelpLib();
	return 0;
}
//------------------------------------------------------------------------
LONG WINAPI my_level_exception_filter(struct _EXCEPTION_POINTERS *ExceptionInfo){
	char szFileName[MAX_PATH];
	GetModuleFileName(g_hinstance,szFileName,MAX_PATH);
	char* p = strrchr(szFileName,'.');
	if(p) *p = 0;

	time_t ti = time(NULL);
	tm* t = localtime(&ti);
	char szTime[MAX_PATH];
	strftime(szTime,MAX_PATH,"%y%m%d%H%M%S",t);

	FileSystem::createPath(g_dumpbasepath);
	char dumpfilename[MAX_PATH];

	DWORD dwrandom=::GetCurrentThreadId();
	DWORD dwpid=::GetCurrentProcessId();
	sprintf_s(dumpfilename,sizeof(dumpfilename),"%s%s_%s_%d_%d.dmp",g_dumpbasepath,extractfiletitle(szFileName),szTime,dwpid,dwrandom);

	Sleep(2000);
	return CreateMiniDump(dumpfilename,ExceptionInfo);
}
//------------------------------------------------------------------------