/**
*	created:		2013-4-7   22:26
*	filename: 		FKIocp
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <malloc.h>
#include <wchar.h>
#include "../Include/Network/FKIocp.h"
#include "../Include/FKLogger.h"
#include "../Include/FKTimeMonitor.h"
#include "../Include/FKRandomPool.h"
//------------------------------------------------------------------------
safe_lookaside_allocator< ACCEPT_OLEX > ACCEPT_OLEX::m_allocator;
//------------------------------------------------------------------------
CLD_IocpObj::~CLD_IocpObj()
{
#ifdef _DEBUG
	if (::GetCurrentThreadId()!=m_IocpHandle.m_hRecycle->getcurid() && !m_IocpKey.pSocket->IsAccepter()){
		static bool boShowThisError=false;
		const char* szerror=vformat("发现在回收线程以外回收 IocpObj(%s,%d = %s,%d)",m_IocpKey.pSocket->GetLocalAddress(),
			m_IocpKey.pSocket->GetLocalPort(),
			m_IocpKey.pSocket->GetRemoteAddress(),
			m_IocpKey.pSocket->GetRemotePort());
		g_logger.fatal(szerror);
		if (!boShowThisError)
		{
			boShowThisError=true;
			::MessageBox(0,"fatal",szerror,0);
		}
	}
#endif
}
//------------------------------------------------------------------------
bool CLD_IocpObj::wsasend_safe(IN SOCKET s,
							   IN LPWSABUF lpBuffers,
							   IN DWORD dwBufferCount,
							   OUT LPDWORD lpNumberOfBytesSent,
							   IN DWORD dwFlags,
							   IN LPWSAOVERLAPPED lpOverlapped,
							   IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
							   int& nwsaErrorCode)
{
	FUNCTION_BEGIN;
	bool boRet=true;

	if (!m_boBind){	return false;}
	else{
		InterlockedIncrement((long *)&m_nRefCount);
		bool boaddref=(lpBuffers && lpBuffers->buf!=NULL && lpBuffers->len>0);
		if(boaddref){InterlockedIncrement((long *)&m_nSendRefCount);}

		if (::WSASend(s,lpBuffers,dwBufferCount,lpNumberOfBytesSent,dwFlags,lpOverlapped,lpCompletionRoutine)==SOCKET_ERROR)
		{
			nwsaErrorCode=WSAGetLastError();
			if ( nwsaErrorCode != ERROR_IO_PENDING )
			{
				if(boaddref){InterlockedDecrement((long *)&m_nSendRefCount);}
				InterlockedDecrement((long *)&m_nRefCount);
				boRet=false;
			}
		}
		else{

		}
	}
	return boRet;
}
//------------------------------------------------------------------------
bool CLD_IocpObj::wsarecv_safe(IN SOCKET s,
							   IN OUT LPWSABUF lpBuffers,
							   IN DWORD dwBufferCount,
							   OUT LPDWORD lpNumberOfBytesRecvd,
							   IN OUT LPDWORD lpFlags,
							   IN LPWSAOVERLAPPED lpOverlapped,
							   IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
							   int& nwsaErrorCode)
{
	FUNCTION_BEGIN;
	bool boRet=true;

	if (!m_boBind){return false;}
	else{
		InterlockedIncrement((long *)&m_nRefCount);
		bool boaddref=(lpBuffers && lpBuffers->buf!=NULL && lpBuffers->len>0);
		if(boaddref){InterlockedIncrement((long *)&m_nRecvRefCount);}
		if (::WSARecv(s,lpBuffers,dwBufferCount,lpNumberOfBytesRecvd,lpFlags,lpOverlapped,lpCompletionRoutine)==SOCKET_ERROR)
		{
			nwsaErrorCode=WSAGetLastError();
			if ( nwsaErrorCode != ERROR_IO_PENDING ){


				if(boaddref){InterlockedDecrement((long *)&m_nRecvRefCount);}
				InterlockedDecrement((long *)&m_nRefCount);
				boRet=false;
			}
		}
	}
	if (boRet){m_i64LastpostRecvTime=time(NULL);}
	return boRet;
}
//------------------------------------------------------------------------
bool CLD_IocpObj::RecycleMe(CLD_IocpObj* me)
{
	if(me){	return me->GetIocpHandle().AddRecycle(*me);}
	return false;
}
//------------------------------------------------------------------------
bool CLD_IocpObj::RecycleMe()
{
	if(this){ return this->GetIocpHandle().AddRecycle(*this);}
	return false;
}
//------------------------------------------------------------------------
bool CLD_IocpObj::IocpCanRecycle()
{
	if ( m_IocpKey.pSocket==NULL || !m_IocpKey.pSocket->IsConnected() ){

		time_t recycle=time(NULL)-m_dwAddRecycleTime;
		if (m_nRefCount==0 && recycle>(2)){
			return m_CanRecycle;
		}else if (recycle>(60)){

			return m_CanRecycle;
		}
	}
	return false;
}
//------------------------------------------------------------------------
LPFN_ACCEPTEX CLD_IocpBaseAccepter::m_lpAcceptExFun=NULL;
LPFN_GETACCEPTEXSOCKADDRS CLD_IocpBaseAccepter::m_lpGetAcceptExSockaddrsFun=NULL;
//------------------------------------------------------------------------
bool CLD_IocpBaseAccepter::Open(const char * sAddr,u_short nPort)
{
	FUNCTION_BEGIN;
	bool boRet=false;
	if (!IsConnected())
	{
		boRet=CLD_Accepter::Open(sAddr,nPort);
		if (boRet){		
			boRet=GetIocpObj().GetIocpHandle().BindAccepter(this);
			if (!boRet){Close();}
		}
	}
	return boRet;
}
//------------------------------------------------------------------------
void CLD_IocpBaseAccepter::DoDisconnect()
{
	FUNCTION_BEGIN;

	CLD_Accepter::DoDisconnect();
	GetIocpObj().GetIocpHandle().UnBindAccepter(this);
}
//------------------------------------------------------------------------
bool CLD_IocpBaseAccepter::SetAcceptExConfig(bool useAcceptex,DWORD dwPostSocket,DWORD dwConnectTimeOutSec,DWORD dwCheckConnectTick)
{
	if (useAcceptex || !m_bouseAcceptex){
		m_bouseAcceptex=useAcceptex;
		m_dwPostSocket=dwPostSocket;
		m_dwConnectTimeOutSec=dwConnectTimeOutSec;
		m_dwCheckConnectTick=dwCheckConnectTick;
		m_boStopAcceptex=false;
	}else{
		m_boStopAcceptex=true;
	}
	return m_bouseAcceptex;
}
//------------------------------------------------------------------------
bool CLD_IocpBaseAccepter::GetAcceptEx(SOCKET listensocket){
	if (m_lpAcceptExFun==NULL){
		DWORD dwResult;
		GUID guidAcceptEx=WSAID_ACCEPTEX;
		if (WSAIoctl(listensocket,SIO_GET_EXTENSION_FUNCTION_POINTER, 
			&guidAcceptEx, sizeof(guidAcceptEx), &m_lpAcceptExFun, 
			sizeof(LPFN_ACCEPTEX), &dwResult, NULL, NULL)==0){
				return true;
		}else{
			m_lpAcceptExFun=NULL;
			return false;
		}
	}else{
		return true;
	}
}
//------------------------------------------------------------------------
bool CLD_IocpBaseAccepter::GetAcceptExSockaddrs(SOCKET listensocket){
	if (m_lpGetAcceptExSockaddrsFun==NULL){
		DWORD dwResult;
		GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
		if (WSAIoctl(listensocket, SIO_GET_EXTENSION_FUNCTION_POINTER, 
			&guidGetAcceptExSockaddrs, sizeof(guidGetAcceptExSockaddrs), 
			&m_lpGetAcceptExSockaddrsFun, 
			sizeof(LPFN_GETACCEPTEXSOCKADDRS), 
			&dwResult, NULL, NULL)==0){
				return true;
		}else{
			m_lpGetAcceptExSockaddrsFun=NULL;
			return false;
		}
	}else{
		return true;
	}
}
//------------------------------------------------------------------------
int CLD_IocpBaseAccepter::PostAcceptEx(DWORD dwPostCount)
{
	if (dwPostCount==0){ dwPostCount=m_dwPostSocket; }
	int dwRetPostCount=0;
	if (m_bouseAcceptex && !m_boStopAcceptex){
		LINGER lingerStruct;
		lingerStruct.l_onoff = 1;
		lingerStruct.l_linger = 0;

		BOOL bNodelay = TRUE;
		DWORD dwBytes=0;

		for(DWORD i = 0; i < dwPostCount; i++){
			SOCKET s=WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (s==INVALID_SOCKET){ continue; }

			setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&bNodelay, sizeof(BOOL));
			setsockopt(s, SOL_SOCKET, SO_LINGER, (char*)&lingerStruct, sizeof(LINGER));

			ACCEPT_OLEX* pOlex=new ACCEPT_OLEX;
			if (!pOlex){closesocket_safe(s);	continue;}
			pOlex->nOpCode=IOCP_ACCEPT;
			pOlex->Accepter=this;
			pOlex->client=s;
			dwBytes=0;

			if(!m_lpAcceptExFun(SocketHandle(), s, 
				pOlex->wsaBuf.buf, pOlex->wsaBuf.len-_IOCP_ACCEPTEX_ADDRESSLENGTH_*2,
				_IOCP_ACCEPTEX_ADDRESSLENGTH_, _IOCP_ACCEPTEX_ADDRESSLENGTH_, 
				&dwBytes, pOlex)){
					if(WSAGetLastError() != ERROR_IO_PENDING){
						closesocket_safe(s);
						SAFE_DELETE(pOlex);
						continue;
					}
			}
			dwRetPostCount++;
			AILOCKT(m_acceptexsockets);
			m_acceptexsockets.insert(pOlex);
		}
	}
	return dwRetPostCount;
}
//------------------------------------------------------------------------
int CLD_IocpBaseAccepter::RunAcceptExCheck()
{
	int nret=0;
	if( m_bouseAcceptex && (GetTickCount()-m_dwtmp_lastCheckConnectTick)>m_dwCheckConnectTick ){
		ACCEPT_OLEX* pOlex=NULL;
		std::CSyncSet< ACCEPT_OLEX* >::iterator it,itnext;
		AILOCKT(m_acceptexsockets);
		for (it=m_acceptexsockets.begin(),itnext=it;it!=m_acceptexsockets.end();it=itnext){
			itnext++;
			pOlex=(*it);
			if (pOlex){
				int nconnecttime=0;
				int noutlen=sizeof(nconnecttime);
				if (pOlex->Accepter==NULL && pOlex->client==INVALID_SOCKET){
					m_acceptexsockets.erase(it);
					SAFE_DELETE(pOlex);
				}else if (pOlex->client!=INVALID_SOCKET){
					if(getsockopt(pOlex->client,SOL_SOCKET, SO_CONNECT_TIME, (char*)&nconnecttime,&noutlen )==0){
						if ( nconnecttime!=(-1) && ((DWORD)nconnecttime)>m_dwConnectTimeOutSec ){
							closesocket_safe(pOlex->client);
							pOlex->client=INVALID_SOCKET;
						}
					}else{
						closesocket_safe(pOlex->client);
						pOlex->client=INVALID_SOCKET;
					}
				}
			}
		}
		nret=m_acceptexsockets.size();
		if (m_boStopAcceptex && nret==0){
			m_bouseAcceptex=false;
			m_boStopAcceptex=false;
		}
		m_dwtmp_lastCheckConnectTick=GetTickCount();
	}
	return nret;
}
//------------------------------------------------------------------------
void CLD_IocpClientSocket::DeferFree()
{
	FUNCTION_BEGIN;
	GetIocpObj().GetIocpHandle().AddRecycle(GetIocpObj());
}
//------------------------------------------------------------------------
CLD_IocpConnecter::CLD_IocpConnecter()
:CLD_IocpConnecterBase()
{

}
//------------------------------------------------------------------------
bool CLD_IocpConnecter::Open(const char * sAddr,u_short nPort)
{
	FUNCTION_BEGIN;
	if (!IsConnected())
	{
		GetIocpObj().Init();
		if (CLD_Connecter::Open(sAddr,nPort))
		{
			if (GetIocpObj().GetIocpHandle().BindConnecter(this))
			{
				OnIocpConnect();
				return true;
			}
			Close();
		}
	}
	return false;
}
//------------------------------------------------------------------------
void CLD_IocpConnecter::DoDisconnect()
{
	FUNCTION_BEGIN;
	CLD_Connecter::DoDisconnect();
	GetIocpObj().SetRecycle();
	GetIocpObj().GetIocpHandle().UnBindConnecter(this);
}
//------------------------------------------------------------------------
CLD_IocpHandle::CLD_IocpHandle()
:m_CallBackVector(1024)
{
	FUNCTION_BEGIN;
	m_hIocp	= INVALID_HANDLE_VALUE;
	m_nWorkerCnt=0;
	m_hAddAccepter=INVALID_HANDLE_VALUE;	
	m_hCloseAcceptor=INVALID_HANDLE_VALUE;	
	m_hAcceptor=NULL;
	m_hRecycle=NULL;
	m_boTerminatedRecycle=false;
	m_AcceptorThreadDebuginfo=NULL;
	m_RecycleThreadDebuginfo=NULL;
	m_boIocpCallback=false;
	m_nIocpCallbackSleepTick=1000;
	m_nIocpHandleCallbackTick=1000;
	m_nIocpObjCallbackTick=1000;
	m_boUseAcceptex=false;
	m_dwCheckConnectcTick=200;
	m_AccepterCount=0;
	m_ClientCount=0;
	m_ConnecterCount=0;
	m_WaitRecycleCount=0;
	m_CallBackVector.clear();
	m_iocpcallbcak=NULL;
	m_iocpcallbcakparam=NULL;
	ZeroMemory(m_WorkerThreadinfos,sizeof(m_WorkerThreadinfos));
}
//------------------------------------------------------------------------
CLD_IocpHandle:: ~CLD_IocpHandle()
{
	FUNCTION_BEGIN;
	Uninit();
}
//------------------------------------------------------------------------
void CLD_IocpHandle::UnBindConnecter(CLD_IocpConnecterBase* Connecter)
{
	FUNCTION_BEGIN;
	INFOLOCK(m_ConnecterList);  
	if (find(m_ConnecterList.begin(),m_ConnecterList.end(),Connecter)!=m_ConnecterList.end()){
		m_ConnecterCount--;
		m_ConnecterList.remove(Connecter);
		Connecter->GetIocpObj().m_boBind=false;
	}
	UNINFOLOCK(m_ConnecterList);  
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::BindConnecter(CLD_IocpConnecterBase* Connecter)
{
	FUNCTION_BEGIN;
	bool boRet=false;
	if (Connecter){
		Connecter->GetIocpObj().m_boBind=false;
		if (IsReady()){
			Connecter->SetKeepAlive(true, KEEPALIVE_TIME, KEEPALIVE_INTERVAL);
			INFOLOCK(m_ConnecterList);  
			if (find(m_ConnecterList.begin(),m_ConnecterList.end(),Connecter)==m_ConnecterList.end()){
				HANDLE h=CreateIoCompletionPort( (HANDLE)Connecter->SocketHandle(), m_hIocp, (DWORD)Connecter->GetIocpObj().GetIocpKey() , 0  );
				if (h){
					boRet=true;
					Connecter->GetIocpObj().m_boBind=true;
					m_ConnecterList.push_back(Connecter);
					m_ConnecterCount++;
					static OLEX Overlapped(IOCP_RECV);
					InterlockedIncrement((long *)&Connecter->GetIocpObj().m_nRefCount);
					::PostQueuedCompletionStatus( m_hIocp,0 , (DWORD)Connecter->GetIocpObj().GetIocpKey(), &Overlapped );
				}
			}
			else{boRet=true;}
			UNINFOLOCK(m_ConnecterList);  
		}
	}
	return boRet;
}
//------------------------------------------------------------------------
void CLD_IocpHandle::UnBindAccepter(CLD_IocpBaseAccepter* Accepter)
{
	FUNCTION_BEGIN;
	if (Accepter){
		INFOLOCK(m_AccepterList);  
		bool boFind=(find(m_AccepterList.begin(),m_AccepterList.end(),Accepter)!=m_AccepterList.end());
		if (boFind){
			m_AccepterCount--;
			m_AccepterList.remove(Accepter);
			Accepter->GetIocpObj().m_boBind=false;
			SetEvent(m_hAddAccepter);
		}
		UNINFOLOCK(m_AccepterList);  
	}
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::BindAccepter(CLD_IocpBaseAccepter* Accepter)
{
	FUNCTION_BEGIN;
	bool boRet=false;
	if (Accepter 
		&& CLD_IocpBaseAccepter::GetAcceptEx(Accepter->SocketHandle()) 
		&& CLD_IocpBaseAccepter::GetAcceptExSockaddrs(Accepter->SocketHandle())){
			Accepter->GetIocpObj().m_boBind=false;
			if (IsReady()){
				INFOLOCK(m_AccepterList);  
				if (find(m_AccepterList.begin(),m_AccepterList.end(),Accepter)==m_AccepterList.end()){
					if(!(m_AccepterCount>=(MAXIMUM_WAIT_OBJECTS - IOCP_ACCEPTER_MINWAIT_OBJECTSID - 1))){
						m_AccepterCount++;
						m_AccepterList.push_back(Accepter);
						boRet=true;
						SetEvent(m_hAddAccepter);
					}
				}else{
					boRet=true;
				}
				UNINFOLOCK(m_AccepterList);  
			}
	}
	return boRet;
}
//------------------------------------------------------------------------
int CLD_IocpHandle::m_nProcessors=0;
//------------------------------------------------------------------------
int CLD_IocpHandle::GetNoOfProcessors()
{
	FUNCTION_BEGIN;
	if (0 == m_nProcessors){
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		m_nProcessors = si.dwNumberOfProcessors;
	}
	return m_nProcessors;
}
//------------------------------------------------------------------------
int CLD_IocpHandle::GetAccepterCount()
{
	FUNCTION_BEGIN;
	return m_AccepterCount;
}
//------------------------------------------------------------------------
int CLD_IocpHandle::GetClientCount()
{
	FUNCTION_BEGIN;
	return m_ClientCount;
}
//------------------------------------------------------------------------
int CLD_IocpHandle::GetConnecterCount()
{
	FUNCTION_BEGIN;
	return m_ConnecterCount;
}
//------------------------------------------------------------------------
int CLD_IocpHandle::GetWaitRecycleCount()
{
	FUNCTION_BEGIN;
	return m_WaitRecycleCount;
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::Init(int nWorkers)
{
	FUNCTION_BEGIN;
	Uninit();

	m_hIocp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0  );
	if ( !m_hIocp )
		return false;

	bool boret=	InitRecycle()
		&& InitWorkers(nWorkers)	
		&&	InitAcceptor();
	if (!boret){g_logger.error_out("CLD_IocpHandle InitThreads Failed...");}
	return boret;
}
//------------------------------------------------------------------------
void CLD_IocpHandle::Uninit()
{
	FUNCTION_BEGIN;
	UninitAcceptor();
	UninitWorkers();
	UninitRecycle();
	if ( m_hIocp != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hIocp );
		m_hIocp = INVALID_HANDLE_VALUE;
	}

	m_AccepterCount=0;
	m_ClientCount=0;
	m_ConnecterCount=0;
	m_WaitRecycleCount=0;
	m_WaitPrcessCount=0;

	INFOLOCK(m_ConnecterList);  
	m_ConnecterList.clear();
	UNINFOLOCK(m_ConnecterList);  

	INFOLOCK(m_AccepterList);  
	m_AccepterList.clear();
	UNINFOLOCK(m_AccepterList);  

	INFOLOCK(m_RecycleObjList);  
	m_RecycleObjList.clear();
	UNINFOLOCK(m_RecycleObjList);  
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::InitWorkers( int nWorkers )
{	
	FUNCTION_BEGIN;
	if ( nWorkers==0 ){
		nWorkers = (GetNoOfProcessors())*2;
		nWorkers = ((nWorkers>8)?8:nWorkers);
		if (nWorkers>4)	{

			nWorkers--;
		}
	}
	nWorkers = safe_min(nWorkers,MAX_WORKTHREADCOUNT-1);

	for ( int i = 0; i < MAX_WORKTHREADCOUNT; i++ ){
		m_WorkerThreadinfos[i].OwnerHandle=this;
		m_WorkerThreadinfos[i].pDebuginfo=NULL;
		m_WorkerThreadinfos[i].pWorkThreadObj=NULL;
	}
	m_nWorkerCnt=0;
	for ( int i = 0; i < nWorkers; i++ ){
		m_WorkerThreadinfos[i].pWorkThreadObj=CThreadFactory::CreateBind(static_callbackworker,&m_WorkerThreadinfos[i]);

		if ( m_WorkerThreadinfos[i].pWorkThreadObj==NULL )
			return false;
		m_nWorkerCnt++;
		m_WorkerThreadinfos[i].pWorkThreadObj->Start(false);
	}
	return true;
}
//------------------------------------------------------------------------
void CLD_IocpHandle::UninitWorkers()
{
	FUNCTION_BEGIN;

	if (m_nWorkerCnt>0)
	{
		int i=0;
		for ( i = 0; i < m_nWorkerCnt; i++ ){
			PostQueuedCompletionStatus( m_hIocp, 0, 0, NULL );
			Sleep(10);
		}
		Sleep(10);

		for ( i = 0; i < MAX_WORKTHREADCOUNT; i++ )
		{
			if ( m_WorkerThreadinfos[i].pWorkThreadObj != NULL )
			{
				m_WorkerThreadinfos[i].pWorkThreadObj->Waitfor();
				SAFE_DELETE(m_WorkerThreadinfos[i].pWorkThreadObj);				
			}
			m_WorkerThreadinfos[i].OwnerHandle=this;
			m_WorkerThreadinfos[i].pDebuginfo=NULL;
			m_WorkerThreadinfos[i].pWorkThreadObj=NULL;
		}	
	}
	m_nWorkerCnt = 0;
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::InitAcceptor()
{
	FUNCTION_BEGIN;

	m_hAddAccepter		= CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hCloseAcceptor	= CreateEvent( NULL, FALSE, FALSE, NULL );

	m_hAcceptor=CThreadFactory::CreateBind(static_callbackacceptor,this);
	if ( m_hAcceptor == NULL )
		return false;

	m_hAcceptor->Start(false);
	return true;
}
//------------------------------------------------------------------------
void CLD_IocpHandle::UninitAcceptor()
{
	FUNCTION_BEGIN;
	if ( m_hAcceptor != NULL )
	{
		SetEvent( m_hCloseAcceptor );		
		m_hAcceptor->Waitfor();
		SAFE_DELETE(m_hAcceptor);

		m_AcceptorThreadDebuginfo=NULL;

		CloseHandle( m_hAddAccepter );
		m_hAddAccepter = INVALID_HANDLE_VALUE;

		CloseHandle( m_hCloseAcceptor );
		m_hCloseAcceptor = INVALID_HANDLE_VALUE;
	}
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::InitRecycle()
{
	FUNCTION_BEGIN;	

	m_hRecycle=CThreadFactory::CreateBind(static_callbackrecycle,this);
	if ( m_hRecycle == NULL )
		return false;
	m_boTerminatedRecycle=false;
	m_hRecycle->Start(true);
	::SetThreadPriority(m_hRecycle->GetHandle(),THREAD_PRIORITY_IDLE);
	m_hRecycle->Resume();
	return true;
}
//------------------------------------------------------------------------
void CLD_IocpHandle::UninitRecycle()
{
	FUNCTION_BEGIN;
	if ( m_hRecycle != NULL )
	{
		m_boTerminatedRecycle=true;

		m_hRecycle->Waitfor();
		SAFE_DELETE(m_hRecycle);

		m_RecycleThreadDebuginfo=NULL;		
		m_boTerminatedRecycle=false;
	}
}
//------------------------------------------------------------------------
unsigned int CLD_IocpHandle::static_callbackworker(CLD_ThreadBase*, WorkThreadinfo* wth )
{
	if (wth && wth->OwnerHandle && wth->pWorkThreadObj)
	{
		THREAD_REGDEBUGINFO(thdebuginfo,&static_callbackworker,"Iocp Work Thread",60);
		stThreadDebugInfo* p =thdebuginfo.debuginfo();

		wth->pDebuginfo=p;
		wth->pDebuginfo->h=wth->pWorkThreadObj->GetHandle();

		wth->OwnerHandle->WorkThread(wth);

		wth->pDebuginfo=NULL;
	}
	return 0;
}
//------------------------------------------------------------------------
unsigned int CLD_IocpHandle::static_callbackacceptor(CLD_ThreadBase*, CLD_IocpHandle *thisobj )
{
	if (thisobj)
	{
		THREAD_REGDEBUGINFO(thdebuginfo,&static_callbackacceptor,"Iocp Accept Thread",60);
		stThreadDebugInfo* p =thdebuginfo.debuginfo();
		thisobj->m_AcceptorThreadDebuginfo=p;
		thisobj->m_AcceptorThreadDebuginfo->h=thisobj->m_hAcceptor->GetHandle();
		thisobj->AcceptThread();
		thisobj->m_AcceptorThreadDebuginfo=NULL;
	}
	return 0;
}
//------------------------------------------------------------------------
unsigned int CLD_IocpHandle::static_callbackrecycle(CLD_ThreadBase*, CLD_IocpHandle *thisobj )
{
	if (thisobj)
	{
		THREAD_REGDEBUGINFO(thdebuginfo,&static_callbackrecycle,"Iocp Recycle Thread",(60*10));
		stThreadDebugInfo* p =thdebuginfo.debuginfo();
		thisobj->m_RecycleThreadDebuginfo=p;
		thisobj->m_RecycleThreadDebuginfo->h=thisobj->m_hRecycle->GetHandle();
		thisobj->RecycleThread();
		thisobj->m_RecycleThreadDebuginfo=NULL;
	}
	return 0;
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::AddRecycle(CLD_IocpObj& pIocpObj)
{
	FUNCTION_BEGIN;

	CLD_Socket* pObj=pIocpObj.GetSocket();
	if (pObj){
		if (IsReady())
		{
			bool boAdd=false;
			pIocpObj.SetRecycle();
			INFOLOCK(m_RecycleObjList);  
			if (m_RecycleObjList.find(&pIocpObj)==m_RecycleObjList.end())
			{
				m_RecycleObjList.insert(&pIocpObj);
				boAdd=true;
			}
			UNINFOLOCK(m_RecycleObjList); 			
		}
		else
		{			
			pObj->Close();
			pIocpObj.SetRecycle();
			while (!(pObj->SocketCanRecycle() && pIocpObj.IocpCanRecycle())){Sleep(1);};
			SAFE_DELETE(pObj);
		}
	}
	return true;	
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::SetAcceptExConfig(bool useAcceptex,DWORD dwCheckConnectcTick)
{
	FUNCTION_BEGIN;
	m_boUseAcceptex=false;
	return false;
	bool boRet=m_boUseAcceptex;
	m_boUseAcceptex=useAcceptex;
	if (m_boUseAcceptex)
	{
		m_dwCheckConnectcTick=dwCheckConnectcTick;
	}
	return boRet;
}
//------------------------------------------------------------------------
bool CLD_IocpHandle::SetIocpCallBack(bool boCallBack,int nIocpCallbackSleepTick,DWORD nIocpObjCallbackTick,DWORD nIocpHandleCallbackTick)
{
	FUNCTION_BEGIN;
	bool boRet=m_boIocpCallback;
	m_boIocpCallback=boCallBack;
	if (m_boIocpCallback)
	{	
		m_nIocpCallbackSleepTick=safe_max<DWORD>(10,nIocpCallbackSleepTick);
		m_nIocpObjCallbackTick=safe_max<DWORD>(10,nIocpObjCallbackTick);
		m_nIocpHandleCallbackTick=safe_max<DWORD>(10,nIocpHandleCallbackTick);		
	}
	return boRet;
}
//------------------------------------------------------------------------
#define   CHECK_SLEEP(x,y,c)					if ( ((GetTickCount()-starttick)>x) || (dwcount>c) ){ Sleep(y);starttick=::GetTickCount();dwcount=0; }else{ dwcount++; };
//------------------------------------------------------------------------
void CLD_IocpHandle::AllIocpObjTimerCallBack()
{
	FUNCTION_BEGIN;

	CLD_IocpObj* pIocpObj=NULL;
	CLD_IocpConnecterBase* pConnter=NULL;
	DWORD starttick=::GetTickCount();
	DWORD dwcount=0;
	m_CallBackVector.clear();
	if (GetConnecterCount()>0)
	{
		INFOLOCK(m_ConnecterList);  
		for (CConnecterList::iterator item=m_ConnecterList.begin();item!=m_ConnecterList.end();item++)
		{
			pConnter=(*item);
			if (pConnter)
			{
				m_CallBackVector.push_back(pConnter);
			}
		}
		UNINFOLOCK(m_ConnecterList); 
		for (int i=0;i<(int)m_CallBackVector.size();i++)
		{
			CHECK_SLEEP(1,5,200);			
			pConnter=(CLD_IocpConnecterBase*)m_CallBackVector[i];
			if (pConnter)
			{

				AILOCKT(pConnter->GetIocpObj().m_callbacktoplock);
				pIocpObj=&pConnter->GetIocpObj();
				if (pConnter->IsConnected() && pIocpObj->m_boBind)
				{
					pIocpObj->RecycleThreadCallBack();
				}
			}
		}
		m_CallBackVector.clear();
	}

	CLD_IocpBaseAccepter* pAcceptor=NULL;
	CLD_IocpClientSocket* pClient=NULL;
	int nClientCount=0;
	if (GetAccepterCount()>0)
	{
		INFOLOCK(m_AccepterList);  
		for (CAccepterList::iterator item=m_AccepterList.begin();item!=m_AccepterList.end();item++)
		{
			pAcceptor=(*item);
			if (pAcceptor)
			{
				if (pAcceptor->GetClientsCount()>0)
				{
					INFOLOCK(*pAcceptor->GetClientList());
					for (CServerClientSocketList::iterator item=pAcceptor->GetClientList()->begin();item!=pAcceptor->GetClientList()->end();item++)
					{
						pClient=(CLD_IocpClientSocket*)(*item);
						//if ( pClient && ((GetTickCount()-pClient->m_lastcallbacktime)>300) )
						if ( pClient && ((GetTickCount()-pClient->m_lastcallbacktime)>1000) )
						{
							m_CallBackVector.push_back(pClient);

							pClient->m_lastcallbacktime=GetTickCount();

							AILOCKT(pClient->GetIocpObj().m_callbacktoplock);
							pIocpObj=&pClient->GetIocpObj();
							if (pClient->IsConnected() && pIocpObj->m_boBind)
							{
								pIocpObj->RecycleThreadCallBack();
							}
						}
						nClientCount++;
					}
					UNINFOLOCK(*pAcceptor->GetClientList());
				}
			}
		}
		UNINFOLOCK(m_AccepterList);  
	}
	m_ClientCount=nClientCount;
	m_CallBackVector.clear();
	m_WaitPrcessCount=m_ClientCount+m_ConnecterCount+m_WaitRecycleCount;
}
//------------------------------------------------------------------------
void CLD_IocpHandle::RecycleThread()
{
	FUNCTION_BEGIN;

	CLD_Socket* pWaitObj=NULL;
	CLD_IocpObj* pIocpObj=NULL;
	CWaitRecycleObjList::iterator item,itemNext;
	DWORD tmp_IocpObjCallbackTick=0;
	DWORD tmp_IocpHandleCallbackTick=0;
	DWORD starttick=::GetTickCount();
	DWORD dwcount=0;
	std::vector< CLD_IocpObj* > canDeleteobj(128);

	while ( !m_boTerminatedRecycle )
	{
		m_RecycleThreadDebuginfo->thisRunBeginTime=0;

		if (!m_boIocpCallback && !m_boUseAcceptex)
		{
			INFOLOCK(m_RecycleObjList);
			bool isempty=m_RecycleObjList.empty();
			UNINFOLOCK(m_RecycleObjList);
			if (isempty)
			{ 
				Sleep( 1000 );
			}
			else
			{ 
				Sleep( 400 );
			}
		}
		else
		{
			Sleep( safe_max(200,m_nIocpCallbackSleepTick-((m_WaitPrcessCount/200)*10)) );
			m_RecycleThreadDebuginfo->thisRunBeginTime=time(NULL);

			if (m_boIocpCallback){
				if ( (GetTickCount()-tmp_IocpObjCallbackTick) > m_nIocpObjCallbackTick )
				{
					FUNCTION_WRAPPER(true,"IocpHandle::AllIocpObjTimerCallBack");
					AllIocpObjTimerCallBack();
					Sleep(5);
					tmp_IocpObjCallbackTick=GetTickCount();
				}
				if ( (GetTickCount()-tmp_IocpHandleCallbackTick) > m_nIocpHandleCallbackTick )
				{
					FUNCTION_WRAPPER(true,"IocpHandle::RecycleThreadCallBack");
					this->RecycleThreadCallBack();
					Sleep(5);
					tmp_IocpHandleCallbackTick=GetTickCount();
				}
			}
		}
		FUNCTION_WRAPPER(true,"IocpHandle::RecycleThread::Run");
		starttick=::GetTickCount();
		m_RecycleThreadDebuginfo->thisRunBeginTime=time(NULL);

		canDeleteobj.clear();

		pIocpObj=NULL;
		pWaitObj=NULL;
		INFOLOCK(m_RecycleObjList); 
		int nWaitRecycleCount=0;
		try
		{
			nWaitRecycleCount=0;

			for (item=m_RecycleObjList.begin(),itemNext=item;item!=m_RecycleObjList.end();item=itemNext){
				itemNext++;
				pIocpObj=(*item);
				if (!pIocpObj){
					m_RecycleObjList.erase(item);
				}else{
					canDeleteobj.push_back(pIocpObj);
					nWaitRecycleCount++;
				}
			}
		}
		catch(...)
		{
		}
		UNINFOLOCK(m_RecycleObjList);  

		pIocpObj=NULL;
		pWaitObj=NULL;
		int nDelCount=canDeleteobj.size();
		if (nDelCount>0){
			for (int i=0;i<nDelCount;i++){
				CHECK_SLEEP(1,5,200);
				pIocpObj=canDeleteobj[i];
				if (pIocpObj){

					INFOLOCK(pIocpObj->m_callbacktoplock);
					pWaitObj=pIocpObj->GetSocket();
					if (pWaitObj && pWaitObj->SocketCanRecycle() && pIocpObj->IocpCanRecycle()){
						if (pWaitObj->IsConnected()){
							pWaitObj->Close();
							pIocpObj->SetRecycle();
							continue;
						}
						UNINFOLOCK(pIocpObj->m_callbacktoplock);
						SAFE_DELETE(pWaitObj);

						INFOLOCK(m_RecycleObjList);  
						m_RecycleObjList.erase(pIocpObj);
						nWaitRecycleCount--;
						UNINFOLOCK(m_RecycleObjList);  
						pWaitObj=NULL;
						pIocpObj=NULL;
					}else if (!pWaitObj){
						UNINFOLOCK(pIocpObj->m_callbacktoplock);
						g_logger.realtimeLog(zLogger::zFORCE,"异常的 iocp 对象( GetSocket==NULL )");
					}else if (pWaitObj->IsConnected()){

						pWaitObj->Close();
						UNINFOLOCK(pIocpObj->m_callbacktoplock);
					}else{
						UNINFOLOCK(pIocpObj->m_callbacktoplock);
					}
				}
			}
		}
		m_WaitRecycleCount=nWaitRecycleCount;
	}

	canDeleteobj.clear();
	m_RecycleThreadDebuginfo->thisRunBeginTime=0;
}
//------------------------------------------------------------------------
void CLD_IocpHandle::WorkThread(WorkThreadinfo* pInfo)
{
	FUNCTION_BEGIN;
	BOOL				bRet;
	DWORD				nTransferred;
	CLD_IocpObj::stIocpKey*	pKey;
	OLEX	*pOverlapped;
	CLD_Socket* pSocket=NULL;
	CLD_IocpObj* pObject=NULL;

	bool boTerminated=false;
	while ( !boTerminated )
	{
		if (pInfo)
		{
			pInfo->pDebuginfo->thisRunBeginTime=0;
		}

		bRet = GetQueuedCompletionStatus( m_hIocp, 
			&nTransferred,
			(DWORD *) &pKey,
			(OVERLAPPED **) &pOverlapped,
			INFINITE );

		FUNCTION_WRAPPER(true,NULL);
		if (pInfo){pInfo->pDebuginfo->thisRunBeginTime=time(NULL);}

		if (!pKey){	boTerminated=true;	break;}
		if( !bRet )
			continue;

		pSocket=pKey->pSocket;
		pObject=pKey->pIocpObj;

		if (!pOverlapped || !pSocket || !pObject 
			|| pObject->GetSocket()!=pSocket){

				if (pObject){INFOLOCK(pObject->m_callbacktoplock);}
				if (pSocket){pSocket->Close();}
				if (pObject && pObject->GetSocket() && pObject->GetSocket()!=pSocket){pObject->GetSocket()->Close();	}
				if (pObject){InterlockedDecrement((long *)&pObject->m_nRefCount);}
				if (pObject){UNINFOLOCK(pObject->m_callbacktoplock);}
				continue;
		}

		AILOCKT(pObject->m_callbacktoplock);
		if ( (nTransferred == 0 
			&& pOverlapped->wsaBuf.len!=0) 
			|| !pSocket->IsConnected() )
		{
			pSocket->Close();			
			InterlockedDecrement((long *)&pObject->m_nRefCount);
			continue;
		}
		switch(pOverlapped->nOpCode)
		{
		case IOCP_SEND:
			{
				if (nTransferred == 0 && pOverlapped->wsaBuf.len==0)
				{	
					pObject->PostIocpSend(NULL,0);
				}
				else
				{												
					pObject->PostIocpSend(pOverlapped,nTransferred);
					if (pSocket->IsConnected())
					{						
						pSocket->DoWrite();
					}

				}
				InterlockedDecrement((long *)&pObject->m_nRefCount);
			}
			break;
		case IOCP_RECV:
			{
				if (nTransferred == 0 && pOverlapped->wsaBuf.len==0)
				{					
					if (pSocket->IsConnected())
					{
						pSocket->DoRead();
					}

					pObject->PostIocpRecv(NULL,0);
				}
				else
				{
					pObject->PostIocpRecv(pOverlapped,nTransferred);
					if (pSocket->IsConnected())
					{
						pSocket->DoRead();
					}
				}
				InterlockedDecrement((long *)&pObject->m_nRefCount);
			}
			break;
		case IOCP_ACCEPT:
			{

			}
			break;
		default:
			{				
				pSocket->Close();				
				InterlockedDecrement((long *)&pObject->m_nRefCount);
			}
			break;
		}

	}
	if (pInfo){pInfo->pDebuginfo->thisRunBeginTime=0;}
}
//------------------------------------------------------------------------
void CLD_IocpHandle::AcceptThread()
{
	FUNCTION_BEGIN;
	HANDLE	hEvents[MAXIMUM_WAIT_OBJECTS] = { m_hAddAccepter, m_hCloseAcceptor,INVALID_HANDLE_VALUE  ,INVALID_HANDLE_VALUE };
	CLD_IocpBaseAccepter* pAccepters[MAXIMUM_WAIT_OBJECTS] ={NULL};

	CLD_IocpBaseAccepter*  pAcceptor=NULL;
	CLD_IocpClientSocket*  pObject=NULL;

	int		nEventCnt = IOCP_ACCEPTER_MINWAIT_OBJECTSID;
	int		nRet=0;

	WSANETWORKEVENTS	eventResult;

	for (int i = IOCP_ACCEPTER_MINWAIT_OBJECTSID; i < MAXIMUM_WAIT_OBJECTS; i++ )
	{
		hEvents[i] = INVALID_HANDLE_VALUE; 
	}

	bool boTerminated=false;
	while ( !boTerminated )
	{
		m_AcceptorThreadDebuginfo->thisRunBeginTime=0;
		nRet = WSAWaitForMultipleEvents( nEventCnt, hEvents, FALSE, INFINITE, FALSE );
		FUNCTION_WRAPPER(true,NULL);
		m_AcceptorThreadDebuginfo->thisRunBeginTime=time(NULL);		
		if(nRet>=0 && nRet<MAXIMUM_WAIT_OBJECTS)
		{
			if (nRet== WAIT_OBJECT_0)
			{				
				INFOLOCK(m_AccepterList); 
				nEventCnt=IOCP_ACCEPTER_MINWAIT_OBJECTSID;	
				ZeroMemory(pAccepters,sizeof(pAccepters));				
				for (int i=IOCP_ACCEPTER_MINWAIT_OBJECTSID;i<MAXIMUM_WAIT_OBJECTS;i++)
				{
					if (hEvents[i]!=INVALID_HANDLE_VALUE)
					{
						WSAResetEvent( hEvents[i] );
					}	
				}
				for (CAccepterList::iterator item=m_AccepterList.begin();item!=m_AccepterList.end();item++)
				{
					pAcceptor=(*item);
					if (pAcceptor)
					{
						if (hEvents[nEventCnt]==INVALID_HANDLE_VALUE)
						{
							hEvents[nEventCnt]=WSACreateEvent();
							WSAResetEvent( hEvents[nEventCnt] );
						}
						WSAEventSelect( pAcceptor->SocketHandle(), hEvents[nEventCnt], FD_ACCEPT );
						pAccepters[nEventCnt]=pAcceptor;
						nEventCnt++;
					}
				}
				UNINFOLOCK(m_AccepterList);  
				continue;
			}
			else if (nRet==(WAIT_OBJECT_0+1))
			{

				boTerminated=true;
				break;
			}
			else
			{
				for (int i=IOCP_ACCEPTER_MINWAIT_OBJECTSID;i<MAXIMUM_WAIT_OBJECTS;i++){
					pAcceptor=pAccepters[i];
					if (pAcceptor)
					{
						WSAEnumNetworkEvents( pAcceptor->SocketHandle(), hEvents[i], &eventResult );
						if ( !eventResult.lNetworkEvents )
							continue;
						WSAResetEvent( hEvents[i] );
						pObject=(CLD_IocpClientSocket *)pAcceptor->Accept();

						if (pObject)
						{							
							INFOLOCK(pObject->GetIocpObj().m_callbacktoplock);
							if (pObject->IsConnected())
							{											
								pObject->SetKeepAlive(true, KEEPALIVE_TIME, KEEPALIVE_INTERVAL);										
								HANDLE h=CreateIoCompletionPort( (HANDLE)pObject->SocketHandle(), m_hIocp, (DWORD) pObject->GetIocpObj().GetIocpKey(), 0  );
								if (!h)
								{
									pObject->GetIocpObj().m_boBind=false;
									pObject->Close();
								}
								else
								{
									pObject->GetIocpObj().m_boBind=true; 
									if (!pObject->GetIocpObj().PostIocpRecv(NULL,0))
									{
										pObject->Close();
									}
									pAcceptor->OnIocpClientConnect(pObject);

								}
								UNINFOLOCK(pObject->GetIocpObj().m_callbacktoplock);
							}
							else
							{
								pObject->Close();
								UNINFOLOCK(pObject->GetIocpObj().m_callbacktoplock);
								SAFE_DELETE(pObject);
							}
						}
						else
						{
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	for (int i = IOCP_ACCEPTER_MINWAIT_OBJECTSID; i < MAXIMUM_WAIT_OBJECTS; i++ ){
		if ( hEvents[i]!=INVALID_HANDLE_VALUE){
			WSACloseEvent( hEvents[i] );
			hEvents[i]=INVALID_HANDLE_VALUE;
		}
	}	
	m_AcceptorThreadDebuginfo->thisRunBeginTime=0;
}
//------------------------------------------------------------------------
void CLD_LoopbufIocpObj::Init()
{
	m_lastreducerecvmemtime=0;
	m_lastreducersendmemtime=0;
	CLD_IocpObj::Init();
	m_olRecv.nOpCode=IOCP_RECV;
	m_olSend.nOpCode=IOCP_SEND;
	m_recvbuf.Clear();

	INFOLOCK(m_bottomlocksendbuf);
	m_sendbuf->Clear();
	m_backsendbuf->Clear();
	UNINFOLOCK(m_bottomlocksendbuf);
}
//------------------------------------------------------------------------
CLD_LoopbufIocpObj::CLD_LoopbufIocpObj(CLD_IocpHandle* Owner,CLD_Socket* pSock,int nLoopbufSize)
:m_initloopbufsize(ROUNDNUM2(nLoopbufSize+1,1024)),
m_recvbuf(m_initloopbufsize),
m_sendbuffer(m_initloopbufsize),
m_backsendbuffer(m_initloopbufsize),
CLD_IocpObj(*Owner,pSock)
{
	m_olRecv.nOpCode=IOCP_RECV;
	m_olSend.nOpCode=IOCP_SEND;

	m_sendbuf=NULL;
	m_backsendbuf=NULL;	
	m_recvbuf.SetCanWrite(false);

	m_sendbuf=&m_sendbuffer;
	m_backsendbuf=&m_backsendbuffer;

	if (m_sendbuf && m_backsendbuf)
	{

		m_sendbuf->SetCanRead(false);
		m_backsendbuf->SetCanRead(false);
	}
}
//------------------------------------------------------------------------
CLD_LoopbufIocpObj::~CLD_LoopbufIocpObj()
{
	m_recvbuf.Clear();
	m_sendbuf->Clear();
	m_backsendbuf->Clear();
	m_sendbuf=NULL;
	m_backsendbuf=NULL;
}
//------------------------------------------------------------------------
int CLD_LoopbufIocpObj::AddSendBuf(const char* pbuf,int nlen)
{
	FUNCTION_BEGIN;

	CLD_Socket* pSocket=GetSocket();
	if (!pSocket){return 0;}

	int nRet=nlen;
	bool boHasBuf=false;

	INFOLOCK(m_bottomlocksendbuf);
	if (m_backsendbuf->InitIdleBuf(nlen,true))
	{
		CopyMemory(m_backsendbuf->pIdle,pbuf,nlen);
		m_backsendbuf->pIdle=m_backsendbuf->pIdle+nlen;
		boHasBuf=true;
	}
	UNINFOLOCK(m_bottomlocksendbuf);

	if (!boHasBuf){
		nRet=0;
		int ErrorCode=ERROR_CLOSESOCKET;
		pSocket->Error(eeNoEnoughSendbuf,ErrorCode,"sendbuf not has enough buf");
	}
	return nRet;
}
//------------------------------------------------------------------------
bool CLD_LoopbufIocpObj::PostSendBuf()
{
	static OLEX Overlapped(IOCP_SEND);	

	return PostIocpSend(NULL,0);
}
//------------------------------------------------------------------------
int CLD_LoopbufIocpObj::AddBuf_Send(const char* pbuf,int nlen)
{
	FUNCTION_BEGIN;
	int nRet=AddSendBuf(pbuf,nlen);
	if (nRet==nlen){ PostSendBuf(); }
	return nRet;
}
//------------------------------------------------------------------------
bool CLD_LoopbufIocpObj::PostIocpRecv(OLEX* pOverlapped,int nTransferred)
{
	FUNCTION_BEGIN;
	FUNCTION_WRAPPER(true,NULL);

	CLD_Socket* pSocket=GetSocket();
	if (!pSocket || !pSocket->IsConnected()){return false;}
	int nerrorcode=-1;
	int nwsaErrorCode=0;
	bool boRet=false;

	if (nTransferred<=m_recvbuf.nIdleLen)
	{
		nerrorcode=0;
		boRet=true;
		if (pOverlapped!=NULL)
		{
			if (nTransferred>0)
			{
				m_recvbuf.SetCanWrite(true);
				m_recvbuf.pIdle=m_recvbuf.pIdle+nTransferred;
				m_recvbuf.SetCanWrite(false);
			}
			InterlockedDecrement((long *)&m_nRecvRefCount);
		}
		if (m_nRecvRefCount==0)
		{
			if (m_recvbuf.nIdleLen<LOOPBUF_EXTEND_SIZE*2)
			{
				if (m_recvbuf.nDataLen>(m_recvbuf.nMaxSize/2))
				{
					m_recvbuf.InitIdleBuf(m_recvbuf.nMaxSize+IOCP_MAXSENDPACKETSIZE*2,true);
				}
				else
				{
					m_recvbuf.InitIdleBuf(m_recvbuf.nMaxSize-m_recvbuf.nDataLen-LOOPBUF_EXTEND_SIZE,true);
				}
			}
			else if(m_recvbuf.nMaxSize>(m_initloopbufsize*2) && m_recvbuf.nDataLen<(m_initloopbufsize/4))
			{
				if (time(NULL)>m_lastreducerecvmemtime)
				{
					m_recvbuf.ReSize(m_initloopbufsize);
					m_lastreducerecvmemtime=time(NULL)+30;
				}
			}
			nerrorcode=-3;
			if (m_recvbuf.nIdleLen>0 && m_recvbuf.pIdle)
			{
				nerrorcode=0;
				memset( &m_olRecv, 0, sizeof( OVERLAPPED ) );
				m_olRecv.wsaBuf.buf = m_recvbuf.pIdle;
				m_olRecv.wsaBuf.len = safe_min(IOCP_MAXSENDPACKETSIZE,m_recvbuf.nIdleLen); 

				DWORD nBytesReceived;
				DWORD nFlag = 0;

				boRet= wsarecv_safe( pSocket->SocketHandle(),
					&m_olRecv.wsaBuf,
					1,
					&nBytesReceived,
					&nFlag,
					(OVERLAPPED *) &m_olRecv,
					NULL,nwsaErrorCode );
			}
		}
	}

	if (nerrorcode<0 )
	{
		int ErrorCode=ERROR_CLOSESOCKET;
		pSocket->Error(eeNoEnoughRecvbuf,ErrorCode,(char *)vformat("recvbuf not has enough buf (%d,%d,%d)",nerrorcode,m_recvbuf.nIdleLen,(DWORD)m_recvbuf.pIdle));
	}
	else if (!boRet)
	{

		pSocket->Close();
		::WSASetLastError(nwsaErrorCode);
		pSocket->CheckSocketResult(SOCKET_ERROR ,eeReceive,(char *)vformat("PostIocpRecv:wsarecv() %d",nwsaErrorCode));
	}
	return boRet;
}
//------------------------------------------------------------------------
bool CLD_LoopbufIocpObj::PostIocpSend(OLEX* pOverlapped,int nTransferred)
{
	FUNCTION_BEGIN;
	FUNCTION_WRAPPER(true,NULL);

	CLD_Socket* pSocket=GetSocket();
	if (!pSocket || !pSocket->IsConnected()){return false;}
	bool boRet=false;
	int nerrorcode=-1;
	int nwsaErrorCode=0;

	INFOLOCK(m_bottomlocksendbuf);
	if (nTransferred<=m_sendbuf->nDataLen)	
	{
		nerrorcode=0;
		boRet=true;
		if (pOverlapped!=NULL)
		{
			if (nTransferred>0)
			{

				m_sendbuf->SetCanRead(true);
				m_sendbuf->pData=m_sendbuf->pData+nTransferred;
				m_sendbuf->SetCanRead(false);
			}
			InterlockedDecrement((long *)&m_nSendRefCount);
		}
		if (m_nSendRefCount==0)
		{
			if (m_backsendbuf->nDataLen<(m_initloopbufsize/4) && m_sendbuf->nDataLen<(m_initloopbufsize/4))
			{
				if (time(NULL)>m_lastreducersendmemtime)
				{
					if(m_sendbuf->nMaxSize>(m_initloopbufsize*2))
					{
						m_sendbuf->ReSize(m_initloopbufsize);
					}
					if(m_backsendbuf->nMaxSize>(m_initloopbufsize*2))
					{
						m_backsendbuf->ReSize(m_initloopbufsize);
					}
					m_lastreducersendmemtime=time(NULL)+30;
				}
			}
			if (m_backsendbuf->nDataLen>0)
			{
				if (m_sendbuf->nDataLen<=0)
				{
					m_sendbuf->Clear();

					CLD_LoopBuf* sendbuf=m_sendbuf;
					m_sendbuf=m_backsendbuf;
					m_backsendbuf=sendbuf;
				}
			}
			if (m_sendbuf->nDataLen>0)
			{
				memset( &m_olSend, 0, sizeof( OVERLAPPED ) );
				m_olSend.wsaBuf.buf = m_sendbuf->pData;				
				int nBytesSent=safe_min(IOCP_MAXSENDPACKETSIZE,m_sendbuf->nDataLen);
				m_olSend.wsaBuf.len = nBytesSent;

				boRet=wsasend_safe(  pSocket->SocketHandle(),
					&m_olSend.wsaBuf, 
					1,
					(LPDWORD)&nBytesSent,
					0,
					(OVERLAPPED *) &m_olSend,
					NULL,nwsaErrorCode );
			}
		}
	}

	UNINFOLOCK(m_bottomlocksendbuf);

	if (nerrorcode<0)
	{
		int ErrorCode=ERROR_CLOSESOCKET;
		pSocket->Error(eeNoEnoughSendbuf,ErrorCode,(char *)vformat("%d -> [ sendbytes(%d) > sendbufs(?) ]",nerrorcode,nTransferred));
	}
	else if (!boRet)
	{
		if (pOverlapped!=NULL && nTransferred>0)
		{			
			pSocket->Close();
		}
		::WSASetLastError(nwsaErrorCode);
		pSocket->CheckSocketResult(SOCKET_ERROR ,eeSend,(char *)vformat("PostIocpSend:wsasend() %d",nwsaErrorCode));
	}
	return boRet;
}
//------------------------------------------------------------------------