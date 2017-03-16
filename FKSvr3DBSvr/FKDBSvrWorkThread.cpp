/**
*	created:		2013-4-9   16:51
*	filename: 		FKDBSvrWorkThread
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKDBSvr.h"
//------------------------------------------------------------------------
unsigned int __stdcall DBService::SimpleMsgProcessThread(CLD_ThreadBase* pthread,void* param){
	GETCURREIP(pcurraddr);
	THREAD_REGDEBUGINFO(thdebuginfo,pcurraddr,"SimpleMsgProcessThread",60);

	while(!pthread->IsTerminated()){

		DWORD startruntick=GetTickCount();
		thdebuginfo.debuginfo()->thisRunBeginTime=time(NULL);

		do{
			CLoginSvrDBConnecter* pqpsocket = NULL;
			AILOCKT(m_loginsvrconnter);
			CSyncSet< CLoginSvrDBConnecter* >::iterator it,itnext;
			for (it=m_loginsvrconnter.begin(),itnext=it;it!=m_loginsvrconnter.end();it=itnext){
				itnext++;
				pqpsocket=(*it);
				if (!pqpsocket->isTerminate() && pqpsocket->IsConnected()){
					pqpsocket->run();
				}
			}	
		} while (false);
		do {
			if (m_checknamesvrconnecter){ m_checknamesvrconnecter->run(); }
			if (m_logsvrconnecter){	m_logsvrconnecter->run(); }
		}while(false);

		thdebuginfo.debuginfo()->thisRunBeginTime=0;

		Sleep(safe_min((DWORD)50,(DWORD)(50-(GetTickCount()-startruntick))));
	}
	thdebuginfo.debuginfo()->thisRunBeginTime=0;
	return 0;
}
//------------------------------------------------------------------------
unsigned int __stdcall DBService::LogicProcessThread(CLD_ThreadBase* pthread,void* param){
	GETCURREIP(pcurraddr);
	THREAD_REGDEBUGINFO(thdebuginfo,pcurraddr,"LogicProcessThread",60);

	while(!pthread->IsTerminated()){

		DWORD startruntick=GetTickCount();
		thdebuginfo.debuginfo()->thisRunBeginTime=time(NULL);
		do{

			CGameSvrSession* pUserSession = NULL;
			AILOCKT(m_gamesvrsession);
			CSyncSet< CGameSvrSession* >::iterator it,itnext;
			for (it=m_gamesvrsession.begin(),itnext=it;it!=m_gamesvrsession.end();it=itnext){
				itnext++;
				pUserSession=((CGameSvrSession*)*it);
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