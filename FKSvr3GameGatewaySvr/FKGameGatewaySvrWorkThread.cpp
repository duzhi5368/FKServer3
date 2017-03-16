/**
*	created:		2013-4-9   17:36
*	filename: 		FKGameGatewaySvrWorkThread
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKGameGatewaySvr.h"
//------------------------------------------------------------------------
#pragma warning(disable:4530)
//------------------------------------------------------------------------
unsigned int __stdcall GameGateService::SimpleMsgProcessThread(CLD_ThreadBase* pthread,void* param)
{
	GETCURREIP(pcurraddr);
	THREAD_REGDEBUGINFO(thdebuginfo,pcurraddr,"SimpleMsgProcessThread",60);

	while(!pthread->IsTerminated()){
		DWORD startruntick=GetTickCount();
		thdebuginfo.debuginfo()->thisRunBeginTime=time(NULL);
		thdebuginfo.debuginfo()->thisRunBeginTime=0;
		Sleep(safe_min((DWORD)20,(DWORD)(20-(GetTickCount()-startruntick)))/*,true*/);
	}
	thdebuginfo.debuginfo()->thisRunBeginTime=0;

	return 0;
}
//------------------------------------------------------------------------
unsigned int __stdcall GameGateService::LogicProcessThread_Try_S1()
{
	THREAD_TRY_EXECUTION2
		do{
			FUNCTION_WRAPPER(true,"CGameGateWayConnecter::run");
			CGameGateWayConnecter* pqpsocket = NULL;

			CSyncMap< DWORD,CGameGateWayConnecter* >::iterator it,itnext;
			for (it=m_gatewayconnter.begin(),itnext=it;it!=m_gatewayconnter.end();it=itnext){
				itnext++;
				pqpsocket=it->second;
				if (!pqpsocket->isTerminate() && pqpsocket->IsConnected()){
					pqpsocket->run();
				}
			}	
		} while (false);
	THREAD_HANDLE_CRASH2
	return 0;
}
//------------------------------------------------------------------------
unsigned int __stdcall GameGateService::LogicProcessThread_Try_S2()
{
	THREAD_TRY_EXECUTION2
		do {
			static DWORD m_lastruntick=GetTickCount();
			if (GetTickCount()-m_lastruntick>500) {
				FUNCTION_WRAPPER(true,"waitdelgateuser::run");
				CSyncSet< CGameGateWayUser* >::iterator it;

				if (m_waitdelgateuser.size()>0){
					for (it=m_waitdelgateuser.begin();it!=m_waitdelgateuser.end();it++){
						CGameGateWayUser* pgateuser=(*it);
						if (pgateuser){
							SAFE_DELETE(pgateuser);
						}
					}
					m_waitdelgateuser.clear();
				}
				m_lastruntick=GetTickCount();
			}
		} while (false);
	THREAD_HANDLE_CRASH2
	return 0;
}
//------------------------------------------------------------------------
unsigned int __stdcall GameGateService::LogicProcessThread_Try_S3()
{
	THREAD_TRY_EXECUTION2
		RunStep();
	THREAD_HANDLE_CRASH2
	return 0;
}
//------------------------------------------------------------------------
unsigned int __stdcall GameGateService::LogicProcessThread(CLD_ThreadBase* pthread,void* param)
{
	GETCURREIP(pcurraddr);
	THREAD_REGDEBUGINFO(thdebuginfo,pcurraddr,"LogicProcessThread",60);

	while(!pthread->IsTerminated())
	{
		DWORD startruntick=GetTickCount();
		thdebuginfo.debuginfo()->thisRunBeginTime=time(NULL);

		try
		{
			thdebuginfo.debuginfo()->thisRunBeginTime = time(NULL);

			do 
			{
				CAIntLock tAutoInfoIntLock1(m_gatewayconnter);
				LogicProcessThread_Try_S1();
			} while ( false );

			do
			{
				CAIntLock tAutoInfoIntLock2(m_waitdelgateuser);
				LogicProcessThread_Try_S2();

			}while( false );

			do 
			{
				LogicProcessThread_Try_S3();

			} while ( false );

			thdebuginfo.debuginfo()->thisRunBeginTime = 0;
		}

		catch(std::exception& e)		
		{
			g_logger.error("[ %s : PID=%d : TID=%d ] exception: %s \r\nCallStack:--------------------------------------------------\r\n", 
				__FUNC_LINE__, ::GetCurrentProcessId(), ::GetCurrentThreadId(), e.what());
			PrintThreadCallStack(NULL);
			g_logger.error("======================================\r\n\r\n");
		}
		catch(...)
		{
			g_logger.error(__FUNC_LINE__"Ö´ÐÐÒì³£======================================\r\n\r\n");
		}

		DWORD _TheSleepTime = 50;
		Sleep( _TheSleepTime );
	}
	thdebuginfo.debuginfo()->thisRunBeginTime=0;
	return 0;
}
//------------------------------------------------------------------------