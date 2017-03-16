/**
*	created:		2013-4-7   22:05
*	filename: 		FKIocp
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKAsynSocket.h"
#include "../STLTemplate/FKLookasideAlloc.h"
//------------------------------------------------------------------------
#define MAX_WORKTHREADCOUNT					48
#define IOCP_ACCEPTER_MINWAIT_OBJECTSID		2
#define IOCP_MAXSENDPACKETSIZE				1024*4
#define _IOCP_ACCEPTEX_ADDRESSLENGTH_		sizeof(sockaddr_in) + 16
//------------------------------------------------------------------------
class CLD_IocpBaseAccepter;
class CLD_IocpAccepter;
class CLD_IocpObj;
//------------------------------------------------------------------------
enum CLD_IocpOpCode
{
	IOCP_NONE,
	IOCP_ACCEPT,
	IOCP_SEND,
	IOCP_RECV,	
};
//------------------------------------------------------------------------
struct OLEX : public OVERLAPPED
{
	CLD_IocpOpCode	nOpCode;
	WSABUF	wsaBuf;
	OLEX(CLD_IocpOpCode op=IOCP_NONE){	wsaBuf.buf=NULL;wsaBuf.len=0;nOpCode=op; }
};
//------------------------------------------------------------------------
struct ACCEPT_OLEX:public OLEX
{
	CLD_IocpBaseAccepter* Accepter;
	SOCKET client;
	char buffer[512];
	ACCEPT_OLEX():OLEX(IOCP_ACCEPT)
	{ 
		Accepter=NULL;
		client=INVALID_SOCKET;
		wsaBuf.buf=&buffer[0];
		wsaBuf.len=sizeof(buffer); 
	};

	void* operator  new(size_t n)
	{
		return LOOKASIDE_GETMEM(m_allocator);
	}
	void operator  delete(void* p)
	{
		m_allocator.freemem((ACCEPT_OLEX *) p);
	}   
	static safe_lookaside_allocator< ACCEPT_OLEX > m_allocator;	
};
//------------------------------------------------------------------------
typedef void(*pIocpHandleCallBacklFunc)(void*);
//------------------------------------------------------------------------
class CLD_IocpObj
{
protected:
	friend class CLD_IocpConnecter;
	friend class CLD_IocpClientSocket;
	friend class CLD_IocpAccepter;
public:
	CLD_IocpHandle& GetIocpHandle(){return m_IocpHandle;}
	CLD_Socket* GetSocket(){return m_IocpKey.pSocket;}

	bool RecycleMe();
	static bool RecycleMe(CLD_IocpObj* me);

	virtual int AddBuf_Send(const char* pbuf,int nlen){return 0;};
	virtual int AddSendBuf(const char* pbuf,int nlen){return 0;};
	virtual bool PostSendBuf(){return false;};

	//设置线程回调函数和参数
	void SetRecycleThreadCallBack(pIocpHandleCallBacklFunc func,void* param)
	{
		m_iocpcallbcak=func;
		m_iocpcallbcakparam=param;
	}

	virtual void SetRecycle(){m_dwAddRecycleTime=time(NULL);}	//设置循环时间

	CIntLock& GetTopLock(){ return m_callbacktoplock;}
protected:
	int m_nSendRefCount;
	int m_nRecvRefCount;
	int m_nRefCount;

protected:
	virtual bool IocpCanRecycle();

	CLD_IocpObj(CLD_IocpHandle& Owner,CLD_Socket* pSock):m_IocpKey(pSock,this),m_IocpHandle(Owner)
	{
		m_nSendRefCount=0;
		m_nRecvRefCount=0;
		m_nRefCount=0;

		m_dwAddRecycleTime=0;
		m_boBind=false;
		m_CanRecycle=true;
		m_iocpcallbcak=NULL;
		m_iocpcallbcakparam=NULL;
	}
	virtual ~CLD_IocpObj();

	virtual void Init()
	{
		m_nSendRefCount=0;
		m_nRecvRefCount=0;
		m_nRefCount=0;
		m_dwAddRecycleTime=0;
		m_boBind=false;
		m_CanRecycle=true;
	}

	friend class CLD_IocpHandle;


	bool wsasend_safe(IN SOCKET s,
		IN LPWSABUF lpBuffers,
		IN DWORD dwBufferCount,
		OUT LPDWORD lpNumberOfBytesSent,
		IN DWORD dwFlags,
		IN LPWSAOVERLAPPED lpOverlapped,
		IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
		int& nwsaErrorCode);

	bool wsarecv_safe(IN SOCKET s,
		IN OUT LPWSABUF lpBuffers,
		IN DWORD dwBufferCount,
		OUT LPDWORD lpNumberOfBytesRecvd,
		IN OUT LPDWORD lpFlags,
		IN LPWSAOVERLAPPED lpOverlapped,
		IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
		int& nwsaErrorCode);

	struct stIocpKey {
		CLD_Socket* pSocket;
		CLD_IocpObj* pIocpObj;
		stIocpKey(CLD_Socket* p1,CLD_IocpObj* p2):pSocket(p1),pIocpObj(p2){}
	};
	const stIocpKey* GetIocpKey(){return &m_IocpKey;};

	virtual bool PostIocpRecv(OLEX* pOverlapped=NULL,int nTransferred=0){return false;};	//发送Iocp接收
	virtual bool PostIocpSend(OLEX* pOverlapped=NULL,int nTransferred=0){return false;};	//发送Iocp发送
	virtual void RecycleThreadCallBack(){
		if (m_iocpcallbcak){m_iocpcallbcak(m_iocpcallbcakparam);}
	}

protected:
	time_t m_i64LastpostRecvTime;		//上次的收发时间
	time_t m_i64LastpostSendTime;

	bool m_CanRecycle;					//能够循环	
	bool m_boBind;						//能够被绑定
	time_t m_dwAddRecycleTime;

	const stIocpKey m_IocpKey;			//Iocp的key
	CLD_IocpHandle& m_IocpHandle;		//IocpHandle的引用

	pIocpHandleCallBacklFunc m_iocpcallbcak;	
	void* m_iocpcallbcakparam;

	OLEX m_olRecv;						//OVERLAPPED
	OLEX m_olSend;
private:
	CIntLock m_callbacktoplock;
};
//------------------------------------------------------------------------
class CLD_IocpBaseAccepter:public CLD_Accepter
{
public:
	CLD_IocpBaseAccepter()
	{
		m_bouseAcceptex=false;
		m_boStopAcceptex=false;
		m_dwPostSocket=50;
		m_dwConnectTimeOutSec=2;
		m_dwCheckConnectTick=400;
		m_dwtmp_lastCheckConnectTick=0;
	}

	virtual ~CLD_IocpBaseAccepter(){}
	virtual bool Open(const char * sAddr,u_short nPort);

	virtual CLD_IocpObj& GetIocpObj()=0;

	bool SetAcceptExConfig(bool useAcceptex,DWORD dwPostSocket=50,DWORD dwConnectTimeOutSec=2,DWORD dwCheckConnectcTick=400);
protected:
	friend class CLD_IocpHandle;
	virtual void DoDisconnect();
	virtual void OnIocpClientConnect(CLD_Socket* Socket)=0;

	virtual CLD_IocpClientSocket * CreateIocpClient(SOCKET s)=0;

	virtual int RunAcceptExCheck();

	static bool GetAcceptEx(SOCKET listensocket);
	static bool GetAcceptExSockaddrs(SOCKET listensocket);

	virtual int PostAcceptEx(DWORD dwPostCount=0);


	bool m_bouseAcceptex;				//是否接受
	DWORD m_dwPostSocket;				
	DWORD m_dwConnectTimeOutSec;		//超出连接时间
	DWORD m_dwCheckConnectTick;			//心跳包用	
	DWORD m_dwtmp_lastCheckConnectTick;	//上次确定连接的时间
	bool m_boStopAcceptex;					
	std::CSyncSet< ACCEPT_OLEX* > m_acceptexsockets;

	static LPFN_ACCEPTEX m_lpAcceptExFun;
	static LPFN_GETACCEPTEXSOCKADDRS m_lpGetAcceptExSockaddrsFun;
private:
	virtual CLD_ClientSocket * CreateClient(SOCKET s)
	{
		return (CLD_ClientSocket*)CreateIocpClient(s);
	}

	virtual void OnClientConnect(CLD_Socket* Socket){};
};
//------------------------------------------------------------------------
class CLD_IocpAccepter:public CLD_IocpBaseAccepter
{
public:
	CLD_IocpAccepter(CLD_IocpHandle& Owner)
		:m_IocpObj(Owner,this),CLD_IocpBaseAccepter(){}
	virtual ~CLD_IocpAccepter(){}
	virtual CLD_IocpObj& GetIocpObj(){return m_IocpObj;};
protected:
	CLD_IocpObj m_IocpObj;	
};
//------------------------------------------------------------------------
class CLD_IocpClientSocket:public CLD_ClientSocket
{
protected:
	DWORD m_lastcallbacktime;
	friend class CLD_IocpBaseAccepter;
	friend class CLD_IocpHandle;

	CLD_IocpClientSocket(CLD_IocpBaseAccepter* Owner,SOCKET s)
		:CLD_ClientSocket((CLD_Accepter *)Owner,s),m_lastcallbacktime(0){}

	virtual ~CLD_IocpClientSocket(){}
	virtual void OnIocpConnect(){};
public:
	virtual CLD_IocpObj& GetIocpObj()=0;
private:
	virtual void DeferFree();
	//禁止从 cld_socket 继承该函数 防止还没有和iocp bind 就执行了 send 或则 recv 操做
	virtual void OnConnect(){};
};
//------------------------------------------------------------------------
class CLD_IocpConnecterBase:public CLD_Connecter
{
protected:
	DWORD m_lastcallbacktime;
	CLD_IocpConnecterBase()
		:CLD_Connecter(),m_lastcallbacktime(0){};
	virtual ~CLD_IocpConnecterBase(){}
	virtual void OnIocpConnect()=0;
public:
	friend class CLD_IocpHandle;
	virtual CLD_IocpObj& GetIocpObj()=0;
private:
	virtual void OnConnect(){};
};
//------------------------------------------------------------------------
class CLD_IocpConnecter:public CLD_IocpConnecterBase
{
public:
	CLD_IocpConnecter();
	virtual ~CLD_IocpConnecter(){}

	virtual bool Open(const char * sAddr,u_short nPort);
protected:
	friend class CLD_IocpHandle;
	virtual void DoDisconnect();
};
//------------------------------------------------------------------------
//IOCP管理类
class CLD_IocpHandle
{
public:
	CLD_IocpHandle();
	virtual ~CLD_IocpHandle();

	bool IsReady(){return m_hIocp != INVALID_HANDLE_VALUE;};
	HANDLE GetIocp(){return m_hIocp;};

	bool Init(int nWorkers=0);
	void Uninit();

	static int GetNoOfProcessors();

	int GetAccepterCount();
	int GetClientCount();
	int GetConnecterCount();
	int GetWaitRecycleCount();

	bool SetAcceptExConfig(bool useAcceptex,DWORD dwCheckConnectcTick=100);
	bool SetIocpCallBack(bool boCallBack,int nIocpCallbackSleepTick=100,DWORD nIocpObjCallbackTick=100,DWORD nIocpHandleCallbackTick=100);

	void SetRecycleThreadCallBack(pIocpHandleCallBacklFunc func,void* param)
	{
		m_iocpcallbcak=func;
		m_iocpcallbcakparam=param;
	}
protected:
	friend class CLD_IocpConnecter;
	friend class CLD_IocpClientSocket;
	friend class CLD_IocpBaseAccepter;
	friend class CLD_IocpObj;

	struct WorkThreadinfo
	{		
		CLD_IocpHandle*		OwnerHandle;
		stThreadDebugInfo*	pDebuginfo;
		CLD_ThreadBase*		pWorkThreadObj;
	};

	bool BindAccepter(CLD_IocpBaseAccepter* Accepter);
	void UnBindAccepter(CLD_IocpBaseAccepter* Accepter);

	//这里只绑定已经和服务器建立连接的 Connecter
	//可以在异步的 OnConnect 事件里面进行绑定
	bool BindConnecter(CLD_IocpConnecterBase* Connecter);
	void UnBindConnecter(CLD_IocpConnecterBase* Connecter);

	bool InitWorkers( int nWorkers );
	void UninitWorkers();

	bool InitAcceptor();
	void UninitAcceptor();

	bool InitRecycle();
	void UninitRecycle();

	bool AddRecycle(CLD_IocpObj& pIocpObj);

	static unsigned int __stdcall static_callbackworker(CLD_ThreadBase*, WorkThreadinfo* wth );
	static unsigned int __stdcall static_callbackacceptor(CLD_ThreadBase*, CLD_IocpHandle *thisobj );
	static unsigned int __stdcall static_callbackrecycle(CLD_ThreadBase*, CLD_IocpHandle *thisobj );

	virtual void RecycleThreadCallBack()
	{
		if (m_iocpcallbcak)
		{
			m_iocpcallbcak(m_iocpcallbcakparam);
		}
	}
private:
	static int m_nProcessors;

	HANDLE					m_hIocp;
	int						m_nWorkerCnt;
	WorkThreadinfo			m_WorkerThreadinfos[MAX_WORKTHREADCOUNT];		//最大工作线程数
	HANDLE					m_hAddAccepter;									//增加接收的句柄
	HANDLE					m_hCloseAcceptor;	
	CLD_ThreadBase*			m_hAcceptor;									//线程的基础接收类
	stThreadDebugInfo*		m_AcceptorThreadDebuginfo;						//线程的调试信息
	CLD_ThreadBase*			m_hRecycle;
	bool					m_boTerminatedRecycle;
	stThreadDebugInfo*		m_RecycleThreadDebuginfo;

	typedef CSyncSet< CLD_IocpObj*  > CWaitRecycleObjList;				//等待循环列表
	CWaitRecycleObjList m_RecycleObjList;
	typedef CSyncList< CLD_IocpBaseAccepter* > CAccepterList;
	CAccepterList m_AccepterList;										//接收链表
	typedef CSyncList< CLD_IocpConnecterBase* > CConnecterList;
	CConnecterList m_ConnecterList;										//连接链表
	typedef vector< CLD_Socket* > CCallBackVector;						//回调的Vector
	CCallBackVector m_CallBackVector;

	bool m_boIocpCallback;
	int m_nIocpCallbackSleepTick;
	DWORD m_nIocpHandleCallbackTick;
	DWORD m_nIocpObjCallbackTick;

	bool m_boUseAcceptex;
	DWORD m_dwCheckConnectcTick;

	int m_AccepterCount;												
	int m_ClientCount;
	int m_ConnecterCount;
	int m_WaitRecycleCount;
	int m_WaitPrcessCount;

	pIocpHandleCallBacklFunc m_iocpcallbcak;
	void* m_iocpcallbcakparam;

	void AllIocpObjTimerCallBack();
	void WorkThread(WorkThreadinfo* pInfo);
	void AcceptThread();
	void RecycleThread();
};
//------------------------------------------------------------------------
//IOCPOBJ的扩展功能,加了个环形Buffer
class CLD_LoopbufIocpObj:public CLD_IocpObj
{
public:
	CLD_LoopbufIocpObj(CLD_IocpHandle* Owner,CLD_Socket* pSock,int nLoopbufSize);
	virtual ~CLD_LoopbufIocpObj();
public:
	virtual void Init();

	virtual int AddBuf_Send(const char* pbuf,int nlen);
	virtual int AddSendBuf(const char* pbuf,int nlen);
	virtual bool PostSendBuf();

	CLD_LoopBuf* GetRecvBufer(){return &m_recvbuf;}
protected:
	friend class CLD_LoopbufIocpConnecter;

	virtual bool PostIocpRecv(OLEX* pOverlapped=NULL,int nTransferred=0);
	virtual bool PostIocpSend(OLEX* pOverlapped=NULL,int nTransferred=0);
private:
	const int m_initloopbufsize;

	CLD_LoopBuf m_recvbuf;
	CIntLock m_bottomlocksendbuf;
	CLD_LoopBuf* m_sendbuf;
	CLD_LoopBuf* m_backsendbuf;
	CLD_LoopBuf m_sendbuffer;
	CLD_LoopBuf m_backsendbuffer;

	time_t  m_lastreducerecvmemtime;
	time_t  m_lastreducersendmemtime;
};
//------------------------------------------------------------------------
class CLD_LoopbufIocpClientSocket: public CLD_IocpClientSocket
{
public:
	CLD_LoopbufIocpClientSocket(CLD_IocpBaseAccepter* Owner,SOCKET s,int nLoopbufSize=DEF_LOOPBUF_SIZE)
		:m_IocpObj(&Owner->GetIocpObj().GetIocpHandle(),this,nLoopbufSize),
		CLD_IocpClientSocket(Owner,s){ m_IocpObj.Init(); };

	CLD_LoopBuf* GetRecvBufer(){return m_IocpObj.GetRecvBufer();}

	virtual CLD_IocpObj& GetIocpObj(){return m_IocpObj;};
protected:
	CLD_LoopbufIocpObj m_IocpObj;
};
//------------------------------------------------------------------------
class CLD_LoopbufIocpConnecter: public CLD_IocpConnecter
{
public:
	CLD_LoopbufIocpConnecter(CLD_IocpHandle* Owner,int nLoopbufSize=DEF_LOOPBUF_SIZE)
		:m_IocpObj(Owner,this,nLoopbufSize),CLD_IocpConnecter(){ m_IocpObj.Init(); }

	CLD_LoopBuf* GetRecvBufer(){return m_IocpObj.GetRecvBufer();}

	virtual CLD_IocpObj& GetIocpObj(){return m_IocpObj;};
	virtual void OnDisconnect(){ 
		m_IocpObj.m_recvbuf.Clear();
		INFOLOCK(m_IocpObj.m_bottomlocksendbuf);
		m_IocpObj.m_sendbuf->Clear();
		m_IocpObj.m_backsendbuf->Clear();
		UNINFOLOCK(m_IocpObj.m_bottomlocksendbuf);
	};
protected:
	CLD_LoopbufIocpObj m_IocpObj;
};
//------------------------------------------------------------------------