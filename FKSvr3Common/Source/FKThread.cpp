/**
*	created:		2013-4-8   15:40
*	filename: 		FKThread
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <process.h>
#include <wchar.h>
#include "../Include/FKThread.h"
#include "../Include/STLTemplate/FKFrameAllocator.h"
#include "../Include/FKError.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include "../Include/STLTemplate/FKSyncList.h"
#include <Ole2.h>
//------------------------------------------------------------------------
CLD_ThreadBase* CLD_ThreadBase::getThreadObj()
{
	FUNCTION_BEGIN;

	if (_TH_VAR_GET(tls_CurrThreadObj).pCurrThreadObj!=NULL 
		&& _TH_VAR_GET(tls_CurrThreadObj).PID==::GetCurrentProcessId()
		&& _TH_VAR_GET(tls_CurrThreadObj).ThreadId==CLD_ThreadBase::getcurid())
	{
		return _TH_VAR_GET(tls_CurrThreadObj).pCurrThreadObj;
	}
	return NULL;
}
//------------------------------------------------------------------------
bool CLD_ThreadBase::Start(bool boCreateSuspended)
{
	FUNCTION_BEGIN;
	if (m_hThread){
		if (m_boSuspended && !boCreateSuspended)	{ Resume();	}
	}else{
		m_ThreadId=0;
		m_hThread=NULL;
		m_Terminated=false;
		m_nDestroyTimeOut=INFINITE;
		m_dwExitCode=0;
		m_boSuspended=false;
		if(boCreateSuspended){
			m_hThread=smBeginThread(NULL,0,ThreadProxy,	this   ,CREATE_SUSPENDED,&m_ThreadId);
		}else{
			m_hThread=smBeginThread(NULL,0,ThreadProxy,	this   ,0,&m_ThreadId);
		}
		if (m_hThread){
			m_nPriority=GetThreadPriority(m_hThread);
			m_boSuspended=boCreateSuspended;
		}else{
			m_ThreadId=0;
			m_hThread=NULL;
			throw CLDError("CLD_ThreadBase Create Error!!",GetLastError(),true);
		}
	}
	return m_hThread?true:false;
}
//------------------------------------------------------------------------
CLD_ThreadBase::CLD_ThreadBase(bool boCreateSuspended,bool boDelThisOnTerminate)
{
	FUNCTION_BEGIN;
	m_ThreadId=0;
	m_hThread=NULL;
	m_Terminated=false;
	m_nDestroyTimeOut=INFINITE;
	m_boSuspended=false;
	m_dwExitCode=0;
	m_nPriority=0;
	m_boDelThisOnTerminate=boDelThisOnTerminate;
	if (!boCreateSuspended){Start(false);}
}
//------------------------------------------------------------------------
CLD_ThreadBase::~CLD_ThreadBase()
{
	FUNCTION_BEGIN;
	DWORD dwExitCode;
	if (m_hThread && Destroy(_dtWAIT,dwExitCode,m_nDestroyTimeOut)!=0) {
		Destroy(_dtTERMINATE,dwExitCode,INFINITE);
	}
}
//------------------------------------------------------------------------
unsigned int CLD_ThreadBase::ThreadProxy( void *pvParam )
{
	unsigned int nRet=0;
	try
	{
		_TH_VAR_GET(tls_CurrThreadObj).pCurrThreadObj=NULL;
		_TH_VAR_GET(tls_CurrThreadObj).ThreadId=0;
		_TH_VAR_GET(tls_CurrThreadObj).PID=0;
		CLD_ThreadBase *ThreadObj=(CLD_ThreadBase*)pvParam;

		if (ThreadObj){
			OleInitialize(NULL);
			_TH_VAR_GET(tls_CurrThreadObj).pCurrThreadObj=ThreadObj;
			_TH_VAR_GET(tls_CurrThreadObj).ThreadId=CLD_ThreadBase::getcurid();
			_TH_VAR_GET(tls_CurrThreadObj).PID=::GetCurrentProcessId();
			ThreadObj->m_dwExitCode=0;
			nRet= (unsigned int)ThreadObj->runthread();
			ThreadObj->m_dwExitCode=nRet;
			ThreadObj->OnTerminate();
			ThreadObj->m_ThreadId=0;
			ThreadObj->m_hThread=NULL;
			ThreadObj->m_Terminated=false;
			ThreadObj->m_boSuspended=false;

			_TH_VAR_GET(tls_CurrThreadObj).pCurrThreadObj=NULL;
			_TH_VAR_GET(tls_CurrThreadObj).ThreadId=0;
			_TH_VAR_GET(tls_CurrThreadObj).PID=0;
			if (ThreadObj->m_boDelThisOnTerminate){SAFE_DELETE(ThreadObj);	}
			OleUninitialize();
		}
	}catch (std::exception& e){
		g_logger.error("[ %s : PID=%d : TID=%d ] exception: %s \r\nCallStack:--------------------------------------------------\r\n", __FUNC_LINE__,::GetCurrentProcessId(),CLD_ThreadBase::getcurid(),e.what());
		PrintThreadCallStack(NULL);
		g_logger.error("======================================\r\n\r\n");
	}
	_TH_VAR_FREE;
	return nRet;
}
//------------------------------------------------------------------------
bool CLD_ThreadBase::Suspend()
{
	FUNCTION_BEGIN;
	if (!m_boSuspended && m_hThread){
		(SuspendThread(m_hThread)==1);
		m_boSuspended=true;
	}
	return m_boSuspended;
}
//------------------------------------------------------------------------
bool CLD_ThreadBase::Resume()
{
	FUNCTION_BEGIN;
	if (m_boSuspended && m_hThread){
		(ResumeThread(m_hThread)!=1);
		m_boSuspended=false;
	}
	return (!m_boSuspended);
}
//------------------------------------------------------------------------
void CLD_ThreadBase::Terminate()
{
	m_Terminated=true;
}
//-----------------------------------------------------------------------
bool CLD_ThreadBase::IsTerminated()
{
	return m_Terminated;
}
//------------------------------------------------------------------------
HANDLE CLD_ThreadBase::GetHandle()
{
	return m_hThread;
}
//------------------------------------------------------------------------
DWORD CLD_ThreadBase::GetId()
{
	return m_ThreadId;
}
//------------------------------------------------------------------------
bool CLD_ThreadBase::getruntime()
{
	FILETIME lpCreationTime;
	FILETIME lpExitTime;
	FILETIME lpKernelTime;
	FILETIME lpUserTime;
	::GetThreadTimes(GetHandle(),&lpCreationTime,&lpExitTime,&lpKernelTime,&lpUserTime);
	m_i64Kerneltime=( QWORD) lpKernelTime.dwHighDateTime<<32 | lpKernelTime.dwLowDateTime;
	m_i64Usertime=( QWORD) lpUserTime.dwHighDateTime<<32 | lpUserTime.dwLowDateTime;
	return true;
}
//------------------------------------------------------------------------
DWORD CLD_ThreadBase::GetExitCode()
{
	return m_dwExitCode;
}
//------------------------------------------------------------------------
bool CLD_ThreadBase::SetPriority(int pri)
{
	FUNCTION_BEGIN;
	if (!m_hThread){
		return false;
	}
	bool boRet=false;
	if (m_nPriority!=pri){
		boRet=(SetThreadPriority(m_hThread,pri)!=0);
		if (boRet){
			m_nPriority=pri;
		}
	}else{
		boRet=true;
	}
	return boRet;
}
//------------------------------------------------------------------------
bool CLD_ThreadBase::IsRunning(void)
{
	FUNCTION_BEGIN;
	DWORD status;
	if(m_boSuspended || !m_hThread){
		return false;
	}
	if (GetExitCodeThread(m_hThread,&status)==0){
		return false;
	}
	if (status == STILL_ACTIVE){
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
int CLD_ThreadBase::Waitfor(DWORD const dwWaitTimes)
{
	FUNCTION_BEGIN;
	DWORD  dwExitCode=0;
	return Destroy(_dtWAIT,dwExitCode,dwWaitTimes);
}
//------------------------------------------------------------------------
int CLD_ThreadBase::TerminateForce(DWORD dwExitCode)
{
	FUNCTION_BEGIN;
	DWORD  tempdwExitCode=dwExitCode;
	return Destroy(_dtTERMINATE,tempdwExitCode);
}
//------------------------------------------------------------------------
int CLD_ThreadBase::Destroy(_DESTROY_TYPE dt,DWORD & dwExitCode,
							DWORD const dwWaitTimes )
{
	FUNCTION_BEGIN;
	int iRet;
	if (!m_hThread){
		m_ThreadId=0;
		return -1;
	}
	switch(dt) 
	{
	case _dtTRY:
		if (GetExitCodeThread(m_hThread,&dwExitCode)==0){
			return -2;
		}
		if (dwExitCode==STILL_ACTIVE){  
			return 1;  
		}

		if (m_hThread){
			m_dwExitCode=dwExitCode;
			m_boSuspended=false;
			CloseHandle(m_hThread);
			m_ThreadId=0;
			m_hThread=NULL;
			m_Terminated=false;
			m_boSuspended=false;
		}
		break;  
	case _dtWAIT:

		Terminate();
		Resume();
		Sleep(1);
		iRet=WaitForSingleObject(m_hThread,dwWaitTimes);  
		if (iRet==WAIT_TIMEOUT){
			dwExitCode=STILL_ACTIVE;
			return 1;  
		}
		if (iRet==WAIT_OBJECT_0){
			GetExitCodeThread(m_hThread,&dwExitCode); 

			if (m_hThread){
				m_dwExitCode=dwExitCode;
				m_boSuspended=false;
				CloseHandle(m_hThread);
				m_ThreadId=0;
				m_hThread=NULL;
				m_Terminated=false;
				m_boSuspended=false;
			}
		}
		break;  
	case _dtTERMINATE:
		if (::GetExitCodeThread(m_hThread,&dwExitCode)==0){
			return -2;
		}
		if (dwExitCode==STILL_ACTIVE){
			Terminate();
			Resume();
			Sleep(1);
			TerminateThread(m_hThread,dwExitCode);
			WaitForSingleObject(m_hThread, INFINITE);

			if (m_hThread){
				m_dwExitCode=dwExitCode;
				m_boSuspended=false;
				CloseHandle(m_hThread);
				m_ThreadId=0;
				m_hThread=NULL;
				m_Terminated=false;
				m_boSuspended=false;
			}
		}
		break;  
	}
	return 0;
}
//------------------------------------------------------------------------