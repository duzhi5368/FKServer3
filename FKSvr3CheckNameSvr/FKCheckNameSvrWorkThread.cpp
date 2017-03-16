/**
*	created:		2013-4-8   15:07
*	filename: 		FKCheckNameSvrWorkThread
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKCheckNameSvr.h"
//------------------------------------------------------------------------
unsigned int __stdcall CCheckNameService::SimpleMsgProcessThread(CLD_ThreadBase* pthread,void* param)
{

	THREAD_TRY_EXECUTION2

		while(!pthread->IsTerminated())
		{ 
			DWORD startruntick=GetTickCount();
			Sleep(safe_min((DWORD)50,(DWORD)(50-(GetTickCount()-startruntick))));
		}

	THREAD_HANDLE_CRASH2
	return 0;	
}
//------------------------------------------------------------------------
unsigned int __stdcall CCheckNameService::LogicProcessThread(CLD_ThreadBase* pthread,void* param)
{
	GETCURREIP(pcurraddr);
	THREAD_REGDEBUGINFO(thdebuginfo,pcurraddr,"LogicProcessThread",60);

	while(!pthread->IsTerminated())
	{
		DWORD startruntick=GetTickCount();
		thdebuginfo.debuginfo()->thisRunBeginTime=time(NULL);

		do{
			CSubSvrSession* pUserSession = NULL;
			AILOCKT(m_gamesvrsession);
			CSyncSet< CSubSvrSession* >::iterator it,itnext;
			for (it=m_gamesvrsession.begin(),itnext=it;it!=m_gamesvrsession.end();it=itnext){
				itnext++;
				pUserSession=((CSubSvrSession*)*it);
				if (!pUserSession->isTerminate() && pUserSession->IsConnected()){
					pUserSession->run();
				}
			}	
		} while (false);
		thdebuginfo.debuginfo()->thisRunBeginTime=0;
		Sleep(safe_min((DWORD)50,(DWORD)(50-(GetTickCount()-startruntick))));
	}
	thdebuginfo.debuginfo()->thisRunBeginTime=0;
	return 0;
}
//------------------------------------------------------------------------