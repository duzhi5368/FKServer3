/**
*	created:		2013-3-22   20:05
*	filename: 		FKDumpErrorBase
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <Windows.h>
#include <assert.h>
#include "FKDumpError.h"
//------------------------------------------------------------------------
#if defined _DEBUG
	#include <crtdbg.h>
	#define AssertFatal(exp,str)	 __noop
	#define Assert(expr)			 __noop
	#ifndef TRACE
		#define TRACE CLD_OutputDebugString
	#endif
#else
	#define AssertFatal(exp,str)	 __noop
	#define Assert(expr)			 __noop
	#ifndef TRACE
		#define TRACE __noop
	#endif
#endif
//------------------------------------------------------------------------
#define  MAX_CALLSTACKINFO_COUNT		512
//------------------------------------------------------------------------
#define FUNCTION_BEGIN
#define FUNCTION_END 
//------------------------------------------------------------------------
struct stThreadDebugInfo{
	HANDLE  h;
	int		id;
	void*	addr;
	char	name[64];

	time_t				thisRunBeginTime;
	CLD_ThreadBase*						m_thobj;

	stThreadDebugInfo(){
	}
};
//------------------------------------------------------------------------
typedef void (WINAPI* funcInit_stSetCallStack)(void* psetcallstack,const char* fflinet_name);
typedef void (WINAPI* funcUnInit_stSetCallStack)(void* psetcallstack);
//------------------------------------------------------------------------
struct stProcessShareData{
	funcInit_stSetCallStack main_initfuncbegin;
	funcUnInit_stSetCallStack main_uninitfuncbegin;
	void* main_logger;

	static stProcessShareData* g_shareData;
	static bool s_ishost;
	static void share_init(bool ishost=false){};
	static void share_uninit(){};
};
//------------------------------------------------------------------------
extern LPTOP_LEVEL_EXCEPTION_FILTER SystemPre;
extern HINSTANCE g_hinstance;
extern HANDLE	g_mainwindowhandle;
//------------------------------------------------------------------------
#define EXCEPTION_CATCH_INIT(hinstance)	\
	do{ \
	g_hinstance=(HINSTANCE)hinstance;	\
	if (SystemPre==NULL){ \
	SetErrorMode(SEM_NOGPFAULTERRORBOX); \
	SystemPre = SetUnhandledExceptionFilter(&my_level_exception_filter); \
	} \
	} while(false); 

//------------------------------------------------------------------------
#define EXCEPTION_CATCH_UNINIT	\
	do{ \
	if (SystemPre!=NULL){ \
	SetErrorMode(0); \
	SetUnhandledExceptionFilter(SystemPre); \
	SystemPre=NULL; \
	} \
	} while(false); 

//------------------------------------------------------------------------
#define MAX_MONITOR_THREAD			512
//------------------------------------------------------------------------
class CThreadMonitor{
protected:
	static CThreadMonitor m_thm;
public:
	static CThreadMonitor& getme(){ return m_thm; }

	char m_szThreadFlag[MAX_MONITOR_THREAD];
	int DebugPrintAllThreadDebufInfo(){
		return 0;
	}
};
//------------------------------------------------------------------------
extern int PrintThreadCallStack( DumpStackCallback Callback = NULL,int nPrintCount=-1,void* param=NULL );
//------------------------------------------------------------------------
struct CStackThreadDebugInfo{
public:
	CStackThreadDebugInfo(void* addr,char* name,int locktick){
		m_info=new stThreadDebugInfo;
		if (m_info){ ZEROOBJ(m_info); }
	}
	~CStackThreadDebugInfo(){
		SAFE_DELETE(m_info);
	}
	stThreadDebugInfo* debuginfo(){return m_info;};
	void start(){ };
	void stop(){ };
private:
	stThreadDebugInfo* m_info;
};
//------------------------------------------------------------------------
#define THREAD_REGDEBUGINFO(name,addr,threadname,locktick)		CStackThreadDebugInfo name((void*)(addr),threadname,locktick)
//------------------------------------------------------------------------
void CLD_OutputDebugString(const char* pszFmt,...);
//------------------------------------------------------------------------