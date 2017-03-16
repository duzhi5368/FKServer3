/**
*	created:		2013-4-6   23:29
*	filename: 		FKAsynSocket
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#define BASE_SERVERSOCK_MSG     WM_USER + 1000
#define CM_SOCKETMESSAGE		BASE_SERVERSOCK_MSG + 1
#define CM_DEFERFREE			BASE_SERVERSOCK_MSG + 2
#define CM_LOOKUPCOMPLETE		BASE_SERVERSOCK_MSG + 3

#define SERVER_SOCKET_EVENT   FD_ACCEPT | FD_CONNECT | FD_CLOSE
#define CLIENT_SOCKET_EVENT   FD_CLOSE | FD_READ | FD_WRITE

#define CONNETER_SOCKET_EVENT FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE

#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)

#define ERROR_CLOSESOCKET			-1
#define MAX_IPLEN					24
#define KEEPALIVE_TIME				5000	
#define KEEPALIVE_INTERVAL			2000	
//------------------------------------------------------------------------
#pragma warning(disable:4201)			
//------------------------------------------------------------------------
#include "../Dump/FKDumpErrorBase.h"
#include <winsock2.h>
#include <mswsock.h>
#include <stdio.h>
#include "../FKSyncObjLock.h"
#include <list>
#include "../STLTemplate/FKSyncList.h"
#include <Iphlpapi.h>
#include "../FKVsVer8Define.h"
#include "../FKThread.h"
#include "FKLoopBuffer.h"
//------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4355)		
#pragma warning(disable:4100)		
#pragma warning(disable:4511)
#pragma warning(disable:4512)
//------------------------------------------------------------------------
#pragma pack(push,1)
struct CLD_TObjectWndProcInstance
{
	BYTE CallNextCode[5];          
	BYTE GetOffset;                
	BYTE GetOwnerObj[3];           
	BYTE JmpToWndProcAddr[3];      
	void* OwnerObj;                
	WNDPROC WndProcAddr;           
	HWND Wnd;
	WNDPROC OldWndProcAddr;

	void init();
};
#pragma pack(pop)
//------------------------------------------------------------------------
CLD_TObjectWndProcInstance* AllocateHWnd(WNDPROC WndProcAddr,void* dwParam);	//创建了一个隐藏窗口
bool DeallocateHWnd(CLD_TObjectWndProcInstance* pObjectWndProcInstance=NULL);	//删除一个隐藏窗口
//------------------------------------------------------------------------
enum CLD_TLookupState
{
	lsIdle,                //空闲
	lsLookupAddress,       //解析IP
	lsLookupService        //可以启动服务
};
//------------------------------------------------------------------------
enum CLD_TErrorEvent
{
	eeSend, 
	eeReceive, 
	eeConnect, 
	eeDisconnect, 
	eeAccept, 
	eeAcceptCreatClient,
	eeLookup,
	eeListen,
	eeGeneral,
	eeSetKeep,
	eeCreateIocp,
	eeNoEnoughRecvbuf ,
	eeNoEnoughSendbuf,
	eeCreateAsync,
	eeAsyncWaitTimeout
};
//------------------------------------------------------------------------
enum CLD_SocketType
{
	SOCKET_NONE=-1,
	SOCKET_CLIENT,
	SOCKET_ACCEPTER,	
	SOCKET_CONNETER
};
//------------------------------------------------------------------------
class CNet:private zNoncopyable
{
public:
	CNet();
	virtual ~CNet();
protected:
	static volatile unsigned long int refcount;
	static CIntLock netreflock;
};
//------------------------------------------------------------------------
class CLD_IocpObj;
class CLD_AsyncHandle;
class CLD_IocpHandle;
//------------------------------------------------------------------------
class CLD_Socket:public CNet
{
public:
	CLD_Socket();
	virtual ~CLD_Socket();

	static char* GetLocalHost(SOCKET s,char * Buf=NULL,int BufLen=0);				//获得主机的名字
	static char* GetLocalAddress(SOCKET s,char * Buf=NULL,int BufLen=0);			//获得本地的地址		
	static int GetLocalPort(SOCKET s);												//获得本地的端口

	static char* GetRemoteHost(SOCKET s,char * Buf=NULL,int BufLen=0);				//获得远程的名字
	static char* GetRemoteAddress(SOCKET s,char * Buf=NULL,int BufLen=0);			//获得远程的地址	
	static int GetRemotePort(SOCKET s);												//获得远程的端口

	static const sockaddr_in* GetRemoteAddr(SOCKET s,sockaddr_in* sin);				//获得远程的sockaddr
	static const sockaddr_in* GetLocalAddr(SOCKET s,sockaddr_in* sin);				//获得本地的scokaddr

	char* GetLocalHost(char * Buf=NULL,int BufLen=0);
	char* GetLocalAddress(char * Buf=NULL,int BufLen=0);
	int GetLocalPort();

	char* GetRemoteHost(char * Buf=NULL,int BufLen=0);
	char* GetRemoteAddress(char * Buf=NULL,int BufLen=0);
	int GetRemotePort();

	const sockaddr_in* GetRemoteAddr();
	const sockaddr_in* GetLocalAddr();

	int ReceiveLength();															//接收到数据的长度
	int ReceiveBuf(char* Buf,int BufLen);											//接收数据的缓冲区

	int SendBuf(const char* Buf,int BufLen,DWORD dwTimeWait=0);						//发送数据
	int SendText(const char* Buf,DWORD dwTimeWait=0);

	static u_long LookupName(const char* szName);									//获得地址
	static u_long LookupName(const char* szName,char* sIpBuf,int nIpBufLen);

	bool IsConnected()																//是否连接
	{
		return (m_boConnected   && (m_Socket!=INVALID_SOCKET));
	};

	void Attach(SOCKET s);															//给sock赋值

	__inline SOCKET SocketHandle(){													//返回一个socket
		return m_Socket==0?INVALID_SOCKET:m_Socket;
	}

	static int GetAdapterInfo(PIP_ADAPTER_INFO pOutAdapter,int nSize);
	static char *getIPByIfName(const char *ifName);

	void SetBindParam(void* lpParam);
	void* GetBindParam();

	//保证 DoDisconnect 只被调用一次
	//在完成端口多线程情况下不能直接调用该函数 该函数会调用没有做线程同步的回调函数
	__inline void Close();

	virtual bool SocketCanRecycle(){return m_CanRecycle;};

	int CheckSocketResult(int nResultCode,CLD_TErrorEvent ErrEvent,char* sOp);			//接口的操作函数

	virtual void Error(CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg);
	virtual bool Open(const char * sAddr,u_short nPort){return false;};
protected:
	friend class CLD_AsyncHandle;
	friend class CLD_IocpHandle;
	friend class CLD_IocpObj;

	virtual bool IsClient(){return false;};
	virtual bool IsConnecter(){return false;};
	virtual bool IsAccepter(){return false;};

	virtual void DoOpen(){};	

	virtual SOCKET socket_safe();
	virtual int closesocket_safe(SOCKET sock,bool boshutdown=false);
	virtual SOCKET accept_safe(SOCKET srvsock);

	virtual CLD_Socket* Accept(){return NULL;};
	virtual void DoConnect(){};

	virtual void DoRead(){};
	virtual void DoWrite(){};

	virtual void Lookup(char* szLookupIp);

	virtual void DoDisconnect();
	virtual void OnDisconnect(){};

	virtual void OnError(CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg);
	virtual  void OnLookup(char* szLookupIp){};

	void SetKeepAlive(bool bokeepalive=true,u_long keepalive_time=KEEPALIVE_TIME,u_long keepalive_interval=KEEPALIVE_INTERVAL);

	bool m_CanRecycle;
	bool m_boConnected;
	bool m_boErroring;

	union{
		struct{
			BYTE localport:1;
			BYTE localaddress:1;

			BYTE localsockaddrin:1;

			BYTE remoteport:1;
			BYTE remotehost:1;
			BYTE remoteaddress:1;

			BYTE remotesockaddrin:1;
		};
		BYTE m_btinfoflag;
	};
protected:
	u_long m_ullocalport;						//本地的端口，名字，地址
	static char m_LocalHost[32];
	char m_LocalAddress[24];

	u_long m_ulremoteport;						//远程的端口，名字，地址
	char m_RemoteHost[32];
	char m_RemoteAddress[24];

	sockaddr_in m_RemoteSockAddrIn;
	sockaddr_in m_LocalSockAddrIn;

	SOCKET m_Socket; 
	void* m_lpParam;
	CIntLock m_bottomlock;
};
//------------------------------------------------------------------------
//只包装了异步消息处理
class CLD_AsyncHandle
{
public:
	CLD_AsyncHandle();
	virtual ~CLD_AsyncHandle();
public:
	int SetAsyncStyles(int Styles);
	int GetAsyncStyles();

	HWND GetHandle();
	virtual void ResetAsync();
	bool ASyncInitSocket(const char * sAddr,u_short nPort,bool boIsIp);
public:
	char				m_sAddr[32];
	u_short				m_wport;
	int					m_AsyncStyles;
	CLD_TLookupState	m_LookupState ;
	CLD_Socket*			m_pSocke;
	HANDLE				m_LookupHandle;
protected:
	virtual LPARAM APIENTRY WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnSocketMsg(SOCKET s,WORD wSelectEvent,WORD wSelectError);
	virtual void OnCMLookupComplete(HANDLE LookupHandle,WORD wAsyncBufLen,WORD wAsyncError);
	virtual void OnCMDeferFree(CLD_Socket* SockObj);
private:
	char* m_GetHostData;

	LPARAM CallBackWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

	CLD_TObjectWndProcInstance* m_ObjectWndProcInstance;
};
//------------------------------------------------------------------------
class CLD_Accepter;
//------------------------------------------------------------------------
class CLD_ClientSocket: public CLD_Socket
{
public:
	CLD_Accepter* GetAccepter(){return m_Owner;};

	virtual bool IsClient(){return true;};
protected:
	virtual void Error(CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg);
	virtual void DoRead();
	virtual void DoWrite();
	virtual void DoDisconnect();
	virtual void DeferFree();

	friend class CLD_IocpHandle;

	friend class CLD_Accepter;
	CLD_ClientSocket(CLD_Accepter* Owner,SOCKET s);
	virtual ~CLD_ClientSocket();

	CLD_Accepter* m_Owner;
private:
	virtual void OnDisconnect();
};
//------------------------------------------------------------------------
typedef CSyncSet< CLD_ClientSocket* > CServerClientSocketList; 

class CLD_Accepter: public CLD_Socket
{
public:
	CLD_Accepter():CLD_Socket(){}
	virtual ~CLD_Accepter();

	virtual bool Open(const char * sAddr,u_short nPort);

	void CloseClient(CLD_Socket* Socket);
	void CloseAllClient();

	typedef CServerClientSocketList::tbase::iterator iterator;

	CServerClientSocketList* GetClientList(){return &m_ClientList;}
	iterator begin() {return m_ClientList.begin();}
	iterator end() {return m_ClientList.end();}

	int GetClientsCount()
	{
		AILOCKT(m_ClientList); 
		return ((int)m_ClientList.size());
	}
	virtual bool IsAccepter(){return true;};
	virtual bool SocketCanRecycle(){ AILOCKT(m_ClientList); return (m_CanRecycle && m_ClientList.empty() ); };// 子连接没有全部删除 不能删除自己
protected:
	friend class CLD_ClientSocket;
	friend class CLD_IocpHandle;

	virtual CLD_Socket* Accept();

	virtual void OnAccept(SOCKET &s){};
	virtual void OnClientConnect(CLD_Socket* Socket){};
	virtual void OnClientDisconnect(CLD_Socket* Socket){};
	virtual void OnClientError(CLD_Socket* Socket,CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg){};
	virtual void OnClientRead(CLD_Socket* Socket){};
	virtual void OnClientWrite(CLD_Socket* Socket){};

	bool AddClient(CLD_ClientSocket * Client);
	bool RemoveClient(CLD_ClientSocket * Client);

	virtual CLD_ClientSocket * CreateClient(SOCKET s)=0;		//必须继承,生成服务器的客户端类对象
	
	virtual void DoDisconnect();

	CServerClientSocketList m_ClientList;
private:
	virtual void OnDisconnect(){};
};
//------------------------------------------------------------------------
class CLD_Connecter:public CLD_Socket
{
public:
	CLD_Connecter():CLD_Socket(){}

	virtual bool Open(const char * sAddr,u_short nPort);
	virtual bool IsConnecter(){return true;};
protected:
	virtual void DoConnect(){ OnConnect(); }
	virtual void DoRead(){OnRead();}
	virtual void DoWrite(){OnWrite();}

	virtual void DoDisconnect();

	virtual void OnConnect(){};
	virtual void OnRead(){};
	virtual void OnWrite(){};
	virtual void OnDisconnect(){};

	friend class CLD_IocpHandle;
};
//------------------------------------------------------------------------
class CLD_AsyncClientSocket : public CLD_ClientSocket
{
protected:
	friend class CLD_AsyncAccepter;

	CLD_AsyncClientSocket(CLD_Accepter* Owner,SOCKET s);
	virtual ~CLD_AsyncClientSocket();

	virtual void DeferFree();

protected:
	CLD_AsyncHandle m_AsyncHandle;

private:
	virtual void DoOpen(){};
};
//------------------------------------------------------------------------
class CLD_AsyncAccepter : public CLD_Accepter
{
public:
	CLD_AsyncAccepter();
	virtual ~CLD_AsyncAccepter();

	virtual bool Open(const char * sAddr,u_short nPort);

protected:
	CLD_AsyncHandle m_AsyncHandle;
protected:

	virtual void OnClientConnect(CLD_Socket* Socket){};
	virtual void OnClientDisconnect(CLD_Socket* Socket){};
	virtual void OnClientError(CLD_Socket* Socket,CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg){};
	virtual void OnClientRead(CLD_Socket* Socket){};
	virtual void OnClientWrite(CLD_Socket* Socket){};

	virtual void DoOpen();
	virtual void DoDisconnect();
};
//------------------------------------------------------------------------
class CLD_AsyncConnecter :public CLD_Connecter
{
public:
	CLD_AsyncConnecter();
	virtual ~CLD_AsyncConnecter();

	virtual bool Open(const char * sAddr,u_short nPort);

protected:
	CLD_AsyncHandle m_AsyncHandle;

	virtual void DoOpen();
	virtual void DoDisconnect();
};
//------------------------------------------------------------------------