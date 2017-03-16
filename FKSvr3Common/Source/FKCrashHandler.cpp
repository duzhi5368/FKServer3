/**
*	created:		2013-4-8   13:01
*	filename: 		FKCrashHandler
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#ifdef WIN32
//------------------------------------------------------------------------
#pragma warning( disable : 4311 )
//------------------------------------------------------------------------
#include "../Include/Crash/FKCrashHandler.h"
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <cassert>
//------------------------------------------------------------------------
bool ON_CRASH_BREAK_DEBUGGER;
//------------------------------------------------------------------------
void StartCrashHandler()
{
	// 设置一个系统异常回调函数
	//  以尝试捕获的不在__try和__except结构之间的异常
	SetUnhandledCrashFilter();	



	// Firstly, check if there is a debugger present. There isn't any point in
	// handling crashes internally if we have a debugger attached, that would 
	// just piss us off. :P

	DWORD code;

	// Check for a debugger.
	__asm
	{
		MOV EAX, FS:[0x18]
		MOV EAX, DWORD PTR [EAX + 0x30]
		MOV ECX, DWORD PTR [EAX]
		MOV [DWORD PTR code], ECX
	}

	if(code & 0x00010000)
	{
		// We got a debugger. We'll tell it to not exit on a crash but instead break into debugger.
		ON_CRASH_BREAK_DEBUGGER = true;
	}
	else
	{
		// No debugger. On crash, we'll call OnCrash to save etc.
		ON_CRASH_BREAK_DEBUGGER = false;
	}
	//ON_CRASH_BREAK_DEBUGGER = IsDebuggerPresent();
}
//------------------------------------------------------------------------
// GetExceptionDescription
// Translate the exception code into something human readable
static const TCHAR *GetExceptionDescription(DWORD ExceptionCode)
{
	struct ExceptionNames
	{
		DWORD	ExceptionCode;
		TCHAR *	ExceptionName;
	};

#if 0  // from winnt.h
#define STATUS_WAIT_0					((DWORD   )0x00000000L)	
#define STATUS_ABANDONED_WAIT_0		  ((DWORD   )0x00000080L)	
#define STATUS_USER_APC				  ((DWORD   )0x000000C0L)	
#define STATUS_TIMEOUT				   ((DWORD   )0x00000102L)	
#define STATUS_PENDING				   ((DWORD   )0x00000103L)	
#define STATUS_SEGMENT_NOTIFICATION	  ((DWORD   )0x40000005L)	
#define STATUS_GUARD_PAGE_VIOLATION	  ((DWORD   )0x80000001L)	
#define STATUS_DATATYPE_MISALIGNMENT	 ((DWORD   )0x80000002L)	
#define STATUS_BREAKPOINT				((DWORD   )0x80000003L)	
#define STATUS_SINGLE_STEP			   ((DWORD   )0x80000004L)	
#define STATUS_ACCESS_VIOLATION		  ((DWORD   )0xC0000005L)	
#define STATUS_IN_PAGE_ERROR			 ((DWORD   )0xC0000006L)	
#define STATUS_INVALID_HANDLE			((DWORD   )0xC0000008L)	
#define STATUS_NO_MEMORY				 ((DWORD   )0xC0000017L)	
#define STATUS_ILLEGAL_INSTRUCTION	   ((DWORD   )0xC000001DL)	
#define STATUS_NONCONTINUABLE_EXCEPTION  ((DWORD   )0xC0000025L)	
#define STATUS_INVALID_DISPOSITION	   ((DWORD   )0xC0000026L)	
#define STATUS_ARRAY_BOUNDS_EXCEEDED	 ((DWORD   )0xC000008CL)	
#define STATUS_FLOAT_DENORMAL_OPERAND	((DWORD   )0xC000008DL)	
#define STATUS_FLOAT_DIVIDE_BY_ZERO	  ((DWORD   )0xC000008EL)	
#define STATUS_FLOAT_INEXACT_RESULT	  ((DWORD   )0xC000008FL)	
#define STATUS_FLOAT_INVALID_OPERATION   ((DWORD   )0xC0000090L)	
#define STATUS_FLOAT_OVERFLOW			((DWORD   )0xC0000091L)	
#define STATUS_FLOAT_STACK_CHECK		 ((DWORD   )0xC0000092L)	
#define STATUS_FLOAT_UNDERFLOW		   ((DWORD   )0xC0000093L)	
#define STATUS_INTEGER_DIVIDE_BY_ZERO	((DWORD   )0xC0000094L)	
#define STATUS_INTEGER_OVERFLOW		  ((DWORD   )0xC0000095L)	
#define STATUS_PRIVILEGED_INSTRUCTION	((DWORD   )0xC0000096L)	
#define STATUS_STACK_OVERFLOW			((DWORD   )0xC00000FDL)	
#define STATUS_CONTROL_C_EXIT			((DWORD   )0xC000013AL)	
#define STATUS_FLOAT_MULTIPLE_FAULTS	 ((DWORD   )0xC00002B4L)	
#define STATUS_FLOAT_MULTIPLE_TRAPS	  ((DWORD   )0xC00002B5L)	
#define STATUS_ILLEGAL_VLM_REFERENCE	 ((DWORD   )0xC00002C0L)	 
#endif

	ExceptionNames ExceptionMap[] =
	{
		{0x40010005, _T("a Control-C")},
		{0x40010008, _T("a Control-Break")},
		{0x80000002, _T("a Datatype Misalignment")},
		{0x80000003, _T("a Breakpoint")},
		{0xc0000005, _T("an Access Violation")},
		{0xc0000006, _T("an In Page Error")},
		{0xc0000017, _T("a No Memory")},
		{0xc000001d, _T("an Illegal Instruction")},
		{0xc0000025, _T("a Noncontinuable Exception")},
		{0xc0000026, _T("an Invalid Disposition")},
		{0xc000008c, _T("a Array Bounds Exceeded")},
		{0xc000008d, _T("a Float Denormal Operand")},
		{0xc000008e, _T("a Float Divide by Zero")},
		{0xc000008f, _T("a Float Inexact Result")},
		{0xc0000090, _T("a Float Invalid Operation")},
		{0xc0000091, _T("a Float Overflow")},
		{0xc0000092, _T("a Float Stack Check")},
		{0xc0000093, _T("a Float Underflow")},
		{0xc0000094, _T("an Integer Divide by Zero")},
		{0xc0000095, _T("an Integer Overflow")},
		{0xc0000096, _T("a Privileged Instruction")},
		{0xc00000fD, _T("a Stack Overflow")},
		{0xc0000142, _T("a DLL Initialization Failed")},
		{0xe06d7363, _T("a Microsoft C++ Exception")},
	};

	for (int i = 0; i < sizeof(ExceptionMap) / sizeof(ExceptionMap[0]); i++)
		if (ExceptionCode == ExceptionMap[i].ExceptionCode)
			return ExceptionMap[i].ExceptionName;

	return _T("an Unknown exception type");
}
//------------------------------------------------------------------------
void echo(const char * format, ...)
{
    va_list vl;
    CHAR szBuf[4096] = "";
    va_start(vl, format);
    vsnprintf_s(szBuf, sizeof(szBuf),_TRUNCATE, format, vl);
    va_end(vl);
}
//------------------------------------------------------------------------
void PrintCrashInformation(PEXCEPTION_POINTERS except)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	TCHAR username[200];
	DWORD usize = 198;
	if(!GetUserName(username, &usize))
		strcpy(username, "Unknown");
	TCHAR winver[200];
	OSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(ver);
	if(GetVersionEx(&ver) == 0)
	{
		ver.dwBuildNumber = 0;
		ver.dwMajorVersion = 5;
		ver.dwMinorVersion = 1;
	}
	MEMORYSTATUS mi;
	mi.dwLength = sizeof(mi);
	GlobalMemoryStatus(&mi);

	if(ver.dwMajorVersion == 5 && ver.dwMinorVersion == 0)
		strcpy(winver, "Windows 2000");
	else if(ver.dwMajorVersion == 5 && ver.dwMinorVersion == 1)
		strcpy(winver, "Windows XP");
	else if(ver.dwMajorVersion == 5 && ver.dwMajorVersion >= 2)
		strcpy(winver, "Windows 2003");
	else if(ver.dwMajorVersion >= 5)
		strcpy(winver, "Windows Vista");
	else
		strcpy(winver, "Unknown Windows");
 
	echo("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	echo("Server has crashed. Reason was:\n");
	echo("   %s at 0x%08X\n", GetExceptionDescription(except->ExceptionRecord->ExceptionCode),
		(unsigned long)except->ExceptionRecord->ExceptionAddress);
	echo("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	echo("System Information:\n");
	echo("   Running as: %s on %s Build %u\n", username, winver, ver.dwBuildNumber);
	echo("   Running on %u processors (type %u)\n", si.dwNumberOfProcessors, si.dwProcessorType);
	echo("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	echo("Call Stack: \n");
 
	CStackWalker sw;
	sw.ShowCallstack();
	echo("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
 
}
//------------------------------------------------------------------------
void CStackWalker::OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName)
{

}
//------------------------------------------------------------------------
void CStackWalker::OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion)
{

}
//------------------------------------------------------------------------
void CStackWalker::OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
{

}
//------------------------------------------------------------------------
void CStackWalker::OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
{
	CHAR buffer[STACKWALK_MAX_NAMELEN];
	if ( (eType != lastEntry) && (entry.offset != 0) )
	{
		if (entry.name[0] == 0)
			strcpy(entry.name, "(function-name not available)");
		if (entry.undName[0] != 0)
			strcpy(entry.name, entry.undName);
		if (entry.undFullName[0] != 0)
			strcpy(entry.name, entry.undFullName);

		char * p = strrchr(entry.loadedImageName, '\\');
		if(!p)
			p = entry.loadedImageName;
		else
			++p;

		if (entry.lineFileName[0] == 0)
		{
			if(entry.name[0] == 0)
				sprintf(entry.name, "%p", entry.offset);
			
			sprintf(buffer, "%s!%s Line %u\n", p, entry.name, entry.lineNumber );
		}
		else
			sprintf(buffer, "%s!%s Line %u\n", p, entry.name, entry.lineNumber);

		OnOutput(buffer);
	}
}
//------------------------------------------------------------------------
void CStackWalker::OnOutput(LPCSTR szText)
{

}
//------------------------------------------------------------------------
class Protect
{
private:
	CRITICAL_SECTION m_cs;
public:
	Protect()
	{
		InitializeCriticalSection( &m_cs );
	}
	~Protect()
	{
		DeleteCriticalSection( &m_cs );
	}
	void Lock()
	{
		EnterCriticalSection( &m_cs );
	}
	void Unlock()
	{
		LeaveCriticalSection( &m_cs );
	}
	bool CheckReturn()
	{
		if( m_cs.RecursionCount != 1 )
			return true;
		return false;
	}
};
//------------------------------------------------------------------------
// 获取当前进程的名称
void GetModuleFilenameWithoutPath(char* szOutFilename, int size)
{
	if (!szOutFilename)
		return;
	ZeroMemory(szOutFilename, size);

	TCHAR modname[MAX_PATH];
	ZeroMemory(modname, sizeof(modname));
	if(GetModuleFileNameA(0, modname, MAX_PATH*2-2) <= 0)
		strcpy(modname, "UNKNOWN");

	char * mname = strrchr(modname, '\\');
	if (mname)
		(void*)mname++;	// Remove the last \	//

	char * szExpand = strstr(mname, ".exe");
	if (szExpand)		// Remove the .exe		//
		mname[szExpand-mname] = 0;

	int len = strlen(mname);
	if (len < size)
		strcpy_s(szOutFilename, size, mname);	
}
//------------------------------------------------------------------------
// 创建一个新的外部进程来生成当前崩溃进程无法在内部生成的DUMP文件
bool CreateDumpProcess(PEXCEPTION_POINTERS pExceptPtrs)
{
	const char* CRASH_DUMP_PROCESS_NAME = "CrashDumpProcess.exe";
	HANDLE hCrashHandledEvent = CreateEventA(0, true, false, 0);

	char szProcessName[128];
	GetModuleFilenameWithoutPath(szProcessName, 128);

	// 用命令行传递参数： 崩溃进程id、崩溃的线程id、异常指针、等待外部进程生成DUMP文件的事件句柄、崩溃进程名
	char szCommandLine[MAX_PATH];
	sprintf_s(szCommandLine, MAX_PATH, "%s %d %d %d %d %s", CRASH_DUMP_PROCESS_NAME, GetCurrentProcessId(), GetCurrentThreadId(), pExceptPtrs, hCrashHandledEvent, szProcessName);

	STARTUPINFOA startup_info;
	memset(&startup_info, 0, sizeof(STARTUPINFO));
	startup_info.dwFlags = STARTF_USESTDHANDLES;
	startup_info.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
	startup_info.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
	startup_info.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
	startup_info.dwFlags = STARTF_USESHOWWINDOW;
	startup_info.wShowWindow = SW_HIDE;
	PROCESS_INFORMATION process_info;										
	if (CreateProcessA(0, szCommandLine, NULL, NULL, true, 0, NULL, NULL, &startup_info, &process_info))
	{
		HANDLE aryHandle[2] = {hCrashHandledEvent, process_info.hProcess};
		if (WAIT_OBJECT_0 == WaitForMultipleObjects(2, aryHandle, false, 10 * 60 * 1000))
		{
			return true;
		}
	}
	
	return false;
}
//------------------------------------------------------------------------
Protect g_p;
HANDLE hDump = INVALID_HANDLE_VALUE;
//------------------------------------------------------------------------
// 异常处理
int __cdecl HandleCrash(PEXCEPTION_POINTERS pExceptPtrs)
{    
#ifdef _FINAL_
    return EXCEPTION_EXECUTE_HANDLER;
#endif

	g_p.Lock(); 

	if(pExceptPtrs == 0)
	{
		// Raise an exception :P
		__try
		{
			RaiseException(EXCEPTION_INVALID_HANDLE, 0, 0, 0);			
		}
		__except(HandleCrash(GetExceptionInformation()), EXCEPTION_CONTINUE_EXECUTION)
		{

		}		
	}

	// Create the date/time string
	time_t curtime = time(NULL);
	tm * pTime = localtime(&curtime);

	char filename[MAX_PATH];
	TCHAR modname[128];	
	GetModuleFilenameWithoutPath(modname, sizeof(modname));

	sprintf(filename, "CrashDumps\\dump-%s-%u-%u-%u-%u-%u-%u-%u.dmp",
		modname, pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday,
		pTime->tm_hour, pTime->tm_min, pTime->tm_sec, GetCurrentThreadId());

	if (hDump == INVALID_HANDLE_VALUE)
		hDump = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,0,0);

	if(hDump == INVALID_HANDLE_VALUE)
	{
		// Create the directory first
		CreateDirectory("CrashDumps", 0);
		hDump = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,0 , 0);
	}

	if(hDump == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, "Could not open crash dump file.", "Crash dump error.", MB_OK);
	}
	else if (pExceptPtrs)
	{  
		// 写出Dump文件
		MINIDUMP_EXCEPTION_INFORMATION info;
		info.ClientPointers = FALSE;
		info.ExceptionPointers = pExceptPtrs;
		info.ThreadId = GetCurrentThreadId();

#ifdef CRASH_MINIDUMP
		if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			hDump, MiniDumpWithIndirectlyReferencedMemory, &info, 0, 0))
#else
		if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			hDump, MiniDumpWithFullMemory, &info, 0, 0))
#endif
		{	
			DWORD dwErrorCode = GetLastError();

			// 删除未生成的DUMP文件
			CloseHandle(hDump);
			DeleteFile(filename);

			// 打开一个外部进程来生成DUMP文件
			if (!CreateDumpProcess(pExceptPtrs))
			{				
				char szTmp[MAX_PATH];
				sprintf_s(szTmp, MAX_PATH, "dump failure! error code is %u", dwErrorCode);
				MessageBoxA(NULL, szTmp, 0, 0);
			}
		}	

		// 手动导出堆栈信息到log文件
		PrintCrashInformation(pExceptPtrs); 
	}

	char szFileName[MAX_PATH];
	::GetModuleFileName(NULL,szFileName,_countof(szFileName));
	char *pcszFileName = _tcsrchr(szFileName,'\\')+1;
	char szInfo[MAX_PATH*2];
	wsprintf(szInfo,"程序%s遇到异常即将关闭，系统生成了保存错误信息的相关文件，十分抱歉。。。",pcszFileName);
	MessageBox(NULL,szInfo,"提示",MB_ICONINFORMATION);


	// 中止当前程序
	HANDLE pH = OpenProcess( PROCESS_TERMINATE, TRUE, GetCurrentProcessId() );
	TerminateProcess( pH, 1 );
	CloseHandle( pH );

	g_p.Unlock();

	return EXCEPTION_CONTINUE_SEARCH;
}
//------------------------------------------------------------------------
// 设置一个系统异常回调函数SetUnhandledExceptionFilter以捕获的不在__try和__except结构之间的异常
typedef LONG (WINAPI * CrashFilterType)(PEXCEPTION_POINTERS pExceptPtrs);
CrashFilterType g_PrevCrashFilter = NULL;

_invalid_parameter_handler g_PrevInvalidParameterHandler = NULL;
_purecall_handler g_PrevPurecallHandler = NULL;
//------------------------------------------------------------------------
void __cdecl My_Invalid_Parameter_Handler(const wchar_t * pszExpression, const wchar_t * pszFunction, const wchar_t * pszFile, unsigned int nLine, uintptr_t pReserved)
{
	// 强制生成一个DUMP
	HandleCrash(NULL);

	if(g_PrevInvalidParameterHandler)
		g_PrevInvalidParameterHandler(pszExpression, pszFunction, pszFile, nLine, pReserved);
}
//------------------------------------------------------------------------
void __cdecl My_Purecall_Handler(void)
{
	// 强制生成一个DUMP
	HandleCrash(NULL);

	if (g_PrevPurecallHandler)	
		g_PrevPurecallHandler();	
}
//------------------------------------------------------------------------
LONG WINAPI CrashFilter(PEXCEPTION_POINTERS pExceptPtrs)
{	
	LONG lnRet = HandleCrash(pExceptPtrs);
	if (g_PrevCrashFilter && EXCEPTION_CONTINUE_SEARCH == lnRet)
	{
		return g_PrevCrashFilter(pExceptPtrs);
	}
	return lnRet;
}
//------------------------------------------------------------------------
void SetUnhandledCrashFilter()
{		
	g_PrevCrashFilter = SetUnhandledExceptionFilter(&CrashFilter);
	assert(g_PrevCrashFilter && "设置系统异常处理函数失败！");

	g_PrevInvalidParameterHandler = _set_invalid_parameter_handler(My_Invalid_Parameter_Handler);
	g_PrevPurecallHandler = _set_purecall_handler(My_Purecall_Handler);
}
//------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------