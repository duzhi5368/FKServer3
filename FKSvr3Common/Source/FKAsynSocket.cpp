/**
*	created:		2013-4-6   23:35
*	filename: 		FKAsynSocket
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/Network/FKAsynSocket.h"
#include <stdio.h>
#include <list>
#include <algorithm>
#include "../Include/FKThread.h"
#include <process.h>
#include <mstcpip.h>
#include "../Include/FKError.h"
#include "../Include/FKLogger.h"
#include "../Include/FKStringEx.h"
#include "../Include/FKTimeMonitor.h"
//------------------------------------------------------------------------
#pragma warning(disable: 4819)			
//------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------
void CLD_TObjectWndProcInstance::init()
{
	FUNCTION_BEGIN;

	CallNextCode[0]=0xE8;
	CallNextCode[1]=0x00;
	CallNextCode[2]=0x00;
	CallNextCode[3]=0x00;
	CallNextCode[4]=0x00;

	GetOffset=0x5A;                            

	GetOwnerObj[0]=0x8B;                      
	GetOwnerObj[1]=0x4A;
	GetOwnerObj[2]=0x07;

	JmpToWndProcAddr[0]=0xFF;                 
	JmpToWndProcAddr[1]=0x62;
	JmpToWndProcAddr[2]=0x0B;

	OwnerObj=0;                              
	WndProcAddr=0;                           
	Wnd=0;
	OldWndProcAddr=0;
};
//------------------------------------------------------------------------
CLD_TObjectWndProcInstance* AllocateHWnd(WNDPROC WndProcAddr,void* dwParam)
{
	FUNCTION_BEGIN;
	WNDCLASSA wc;

	bool ClassRegistered;

	HINSTANCE hInstance=(HINSTANCE)GetModuleHandle(0);

	ClassRegistered=(!(!GetClassInfoA(hInstance,"CLD_TPUtilWindow",&wc)));

	if(!ClassRegistered)
	{
		wc.style= 0;
		wc.lpfnWndProc= DefWindowProc;
		wc.cbClsExtra= 0;
		wc.cbWndExtra= 0;
		wc.hInstance= hInstance;
		wc.hIcon= 0;
		wc.hCursor= 0;
		wc.hbrBackground= 0;
		wc.lpszMenuName= 0;
		wc.lpszClassName="CLD_TPUtilWindow";	
		RegisterClassA(&wc);
	}
	HWND Wnd=CreateWindowEx(WS_EX_TOOLWINDOW,"CLD_TPUtilWindow","",WS_POPUP,0,0,0,0,0,0,hInstance,NULL);
	if(Wnd)
	{
		CLD_TObjectWndProcInstance* pObjectWndProcInstance=(CLD_TObjectWndProcInstance*)__mt_char_alloc.allocate(sizeof(CLD_TObjectWndProcInstance));
		pObjectWndProcInstance->init();
		if(pObjectWndProcInstance){
			pObjectWndProcInstance->Wnd=Wnd;
			pObjectWndProcInstance->OwnerObj=dwParam;
			pObjectWndProcInstance->WndProcAddr=WndProcAddr;
		}
		if (WndProcAddr){
			pObjectWndProcInstance->OldWndProcAddr=(WNDPROC) SetWindowLongA(Wnd,GWL_WNDPROC,(LONG)(&pObjectWndProcInstance->CallNextCode[0]));
		}
		return pObjectWndProcInstance;
	}
	return NULL;
};
//------------------------------------------------------------------------
bool DeallocateHWnd(CLD_TObjectWndProcInstance* pObjectWndProcInstance)
{
	FUNCTION_BEGIN;
	if(pObjectWndProcInstance && pObjectWndProcInstance->Wnd)
	{
		DestroyWindow(pObjectWndProcInstance->Wnd);
		pObjectWndProcInstance->Wnd=0;

		__mt_char_alloc.deallocate((char*)pObjectWndProcInstance);
		pObjectWndProcInstance=NULL;

		return true;
	}
	return false;
};
//------------------------------------------------------------------------
static sockaddr_in static_error_sockaddr_in;
//------------------------------------------------------------------------
CNet::CNet() 
{
	FUNCTION_BEGIN;
	AILOCKT(netreflock);  
	if(refcount==0){
		WSADATA wsaD;	
		int result = WSAStartup( MAKEWORD(2,2), &wsaD );
		memset(&static_error_sockaddr_in,0xff,sizeof(static_error_sockaddr_in));
		if( result != 0 ) {
			return;
		}
	}
	refcount++;
}
//------------------------------------------------------------------------
CNet::~CNet() 
{
	FUNCTION_BEGIN;
	AILOCKT(netreflock);   
	refcount--;
	if(refcount<=0){
		WSACleanup();
		refcount=0;
	}
}
//------------------------------------------------------------------------
char CLD_Socket::m_LocalHost[32]={0};
//------------------------------------------------------------------------
CLD_Socket::CLD_Socket()
{
	FUNCTION_BEGIN;
	m_ullocalport=0;
	m_LocalAddress[0]=0;  
	m_ulremoteport=0;
	m_RemoteHost[0]=0;
	m_RemoteAddress[0]=0;	
	m_btinfoflag=0;
	Attach(INVALID_SOCKET);
	m_boConnected=false;

	m_boErroring=false;
	m_lpParam=NULL;
	m_CanRecycle=true;
}
//------------------------------------------------------------------------
CLD_Socket::~CLD_Socket(){

	AILOCKT(m_bottomlock);
	if(SocketHandle()!=INVALID_SOCKET){
		closesocket(SocketHandle());
		Attach(INVALID_SOCKET);
	}
};
//------------------------------------------------------------------------
char* CLD_Socket::GetLocalHost(SOCKET s,char * Buf,int BufLen){
	if (!Buf || BufLen==0){
		_GET_TLS_LOOPCHARBUF(128);
		Buf=ptlsbuf;BufLen=ntlslen;
	}
	gethostname(Buf,BufLen-1);
	Buf[BufLen-1]=0;
	return Buf;
}
//------------------------------------------------------------------------
char* CLD_Socket::GetLocalAddress(SOCKET s,char * Buf,int BufLen){
	if (!Buf || BufLen==0){
		_GET_TLS_LOOPCHARBUF(128);
		Buf=ptlsbuf;BufLen=ntlslen;
	}
	sockaddr_in SockAddrIn;
	int Size=sizeof(sockaddr);
	getsockname(s, (SOCKADDR*)&SockAddrIn,&Size);
	strcpy_s(Buf,BufLen-1,inet_ntoa(SockAddrIn.sin_addr));
	Buf[BufLen-1]=0;
	return Buf;
}
//------------------------------------------------------------------------
int CLD_Socket::GetLocalPort(SOCKET s){
	sockaddr_in SockAddrIn;
	int Size=sizeof(sockaddr);
	getsockname(s, (SOCKADDR*)&SockAddrIn,&Size);
	return ntohs(SockAddrIn.sin_port);
}
//------------------------------------------------------------------------
char* CLD_Socket::GetRemoteHost(SOCKET s,char * Buf,int BufLen){
	if (!Buf || BufLen==0){
		_GET_TLS_LOOPCHARBUF(128);
		Buf=ptlsbuf;BufLen=ntlslen;
	}
	sockaddr_in SockAddrIn;
	hostent* hostEnt = NULL;
	int Size=sizeof(sockaddr);
	getpeername(s, (SOCKADDR*)&SockAddrIn,&Size);
	hostEnt = gethostbyaddr( (char*)&SockAddrIn.sin_addr.s_addr, 4 ,PF_INET );
	if( hostEnt != NULL ){
		strcpy_s(Buf,BufLen-1,hostEnt->h_name);
	}
	Buf[BufLen-1]=0;
	return Buf;
}
//------------------------------------------------------------------------
char* CLD_Socket::GetRemoteAddress(SOCKET s,char * Buf,int BufLen){
	if (!Buf || BufLen==0){
		_GET_TLS_LOOPCHARBUF(128);
		Buf=ptlsbuf;BufLen=ntlslen;
	}
	sockaddr_in SockAddrIn;
	int Size=sizeof(sockaddr);
	getpeername(s, (SOCKADDR*)&SockAddrIn,&Size);
	strcpy_s(Buf,BufLen-1,inet_ntoa(SockAddrIn.sin_addr));
	Buf[BufLen-1]=0;
	return Buf;
}
//------------------------------------------------------------------------
int CLD_Socket::GetRemotePort(SOCKET s){
	sockaddr_in SockAddrIn;
	int Size=sizeof(sockaddr);
	getpeername(s, (SOCKADDR*)&SockAddrIn,&Size);
	return ntohs( SockAddrIn.sin_port );
}
//------------------------------------------------------------------------
const sockaddr_in* CLD_Socket::GetRemoteAddr(SOCKET s,sockaddr_in* sin)
{	
	int Size=sizeof(sockaddr);
	getpeername(s,(SOCKADDR*)sin,&Size);
	return sin;
}
//------------------------------------------------------------------------
const sockaddr_in* CLD_Socket::GetLocalAddr(SOCKET s,sockaddr_in* sin)
{	
	int Size=sizeof(sockaddr);
	getsockname(s,(SOCKADDR*)sin,&Size);
	return sin;
}
//------------------------------------------------------------------------
const sockaddr_in* CLD_Socket::GetRemoteAddr(){
	if (remotesockaddrin==0){
		int Size=sizeof(sockaddr);
		if(CheckSocketResult(getpeername(SocketHandle(), (SOCKADDR*)&m_RemoteSockAddrIn,&Size),eeGeneral,"getpeername()")==0){
			remotesockaddrin=1;
		}else{
			remotesockaddrin=0;
			return &static_error_sockaddr_in;
		}
	}
	return &m_RemoteSockAddrIn;
}
//------------------------------------------------------------------------
const sockaddr_in* CLD_Socket::GetLocalAddr(){
	if (localsockaddrin==0){
		int Size=sizeof(sockaddr);
		if(CheckSocketResult(getsockname(SocketHandle(), (SOCKADDR*)&m_LocalSockAddrIn,&Size),eeGeneral,"getsockname") == 0 ){
			localsockaddrin=1;
		}else{
			remotesockaddrin=0;
			return &static_error_sockaddr_in;
		}
	}
	return &m_LocalSockAddrIn;
}
//------------------------------------------------------------------------
char* CLD_Socket::GetLocalHost(char * Buf,int BufLen)
{
	__try
	{
		if (m_LocalHost[0]==0){
			CheckSocketResult(gethostname(m_LocalHost,sizeof(m_LocalHost)-1),eeGeneral,"gethostname");
			m_LocalHost[sizeof(m_LocalHost)-1]=0;
		}
		if(Buf!=NULL && m_LocalHost[0]!=0){
			strcpy_s(Buf,BufLen-1,m_LocalHost);
			Buf[BufLen-1]=0;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return m_LocalHost;
}
//------------------------------------------------------------------------
char* CLD_Socket::GetLocalAddress(char * Buf,int BufLen)
{
	if (m_LocalAddress[0]!=0 && localaddress!=0){
		if(Buf!=NULL){
			strcpy_s(Buf,BufLen-1,m_LocalAddress);
			Buf[BufLen-1]=0;
		}
		return m_LocalAddress;
	}
	if (IsConnected()){
		__try
		{
			if (localsockaddrin!=0){
				strcpy_s(m_LocalAddress,sizeof(m_LocalAddress)-1,inet_ntoa(m_LocalSockAddrIn.sin_addr));
				m_LocalAddress[sizeof(m_LocalAddress)-1]=0;
				localaddress=1;
			}else{
				int Size=sizeof(sockaddr);
				if(CheckSocketResult(getsockname(SocketHandle(), (SOCKADDR*)&m_LocalSockAddrIn,&Size),eeGeneral,"getsockname") == 0 ){
					strcpy_s(m_LocalAddress,sizeof(m_LocalAddress)-1,inet_ntoa(m_LocalSockAddrIn.sin_addr));
					m_LocalAddress[sizeof(m_LocalAddress)-1]=0;
					localaddress=1;
					localsockaddrin=1;
					if(Buf!=NULL){
						strcpy_s(Buf,BufLen-1,m_LocalAddress);
						Buf[BufLen-1]=0;
					}
				}	
			}

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	return m_LocalAddress;
}
//------------------------------------------------------------------------
int CLD_Socket::GetLocalPort()
{
	if (m_ullocalport!=0 && localport!=0){return m_ullocalport;}
	int nRetPort=m_ullocalport;
	if (IsConnected()){
		__try
		{
			if (localsockaddrin!=0){
				nRetPort=ntohs(m_LocalSockAddrIn.sin_port);
				m_ullocalport=nRetPort;	
				localport=1;
			}else{
				int Size=sizeof(sockaddr);
				if (CheckSocketResult(getsockname(SocketHandle(), (SOCKADDR*)&m_LocalSockAddrIn,&Size),eeGeneral,"getsockname")==0){
					nRetPort=ntohs(m_LocalSockAddrIn.sin_port);
					m_ullocalport=nRetPort;	
					localport=1;
					localsockaddrin=1;
				}
			}

		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	return nRetPort;
}
//------------------------------------------------------------------------
char* CLD_Socket::GetRemoteHost(char * Buf,int BufLen)
{
	if (m_RemoteHost[0]!=0 && remotehost!=0){
		if(Buf!=NULL){
			strcpy_s(Buf,BufLen-1,m_RemoteHost);
			Buf[BufLen-1]=0;
		}
		return m_RemoteHost;
	}
	if (IsConnected() ){
		__try
		{
			if (remotesockaddrin!=0){
				hostent* hostEnt = NULL;
				hostEnt = gethostbyaddr( (char*)&m_RemoteSockAddrIn.sin_addr.s_addr, 4 ,PF_INET );
				if( hostEnt != NULL ){
					strcpy_s(m_RemoteHost,sizeof(m_RemoteHost)-1,hostEnt->h_name);
					m_RemoteHost[sizeof(m_RemoteHost)-1]=0;
					remotehost=1;
					if(Buf!=NULL){
						strcpy_s(Buf,BufLen-1,m_RemoteHost);
						Buf[BufLen-1]=0;
					}
				}
			}else{
				hostent* hostEnt = NULL;
				int Size=sizeof(sockaddr);
				if(CheckSocketResult(getpeername(SocketHandle(), (SOCKADDR*)&m_RemoteSockAddrIn,&Size),eeGeneral,"getpeername()")==0){
					hostEnt = gethostbyaddr( (char*)&m_RemoteSockAddrIn.sin_addr.s_addr, 4 ,PF_INET );
					if( hostEnt != NULL ){
						strcpy_s(m_RemoteHost,sizeof(m_RemoteHost)-1,hostEnt->h_name);
						m_RemoteHost[sizeof(m_RemoteHost)-1]=0;
						remotehost=1;
						remotesockaddrin=1;
						if(Buf!=NULL){
							strcpy_s(Buf,BufLen-1,m_RemoteHost);
							Buf[BufLen-1]=0;
						}
					}
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	return m_RemoteHost;
}
//------------------------------------------------------------------------
char* CLD_Socket::GetRemoteAddress(char * Buf,int BufLen)
{
	if (m_RemoteAddress[0]!=0 && remoteaddress!=0){
		if(Buf!=NULL){
			strcpy_s(Buf,BufLen-1,m_RemoteAddress);
			Buf[BufLen-1]=0;
		}
		return m_RemoteAddress;
	}
	if (IsConnected())
	{
		__try
		{
			if (remotesockaddrin!=0){
				strcpy_s(m_RemoteAddress,sizeof(m_RemoteAddress)-1,inet_ntoa(m_RemoteSockAddrIn.sin_addr));
				m_RemoteAddress[sizeof(m_RemoteAddress)-1]=0;
				remoteaddress=1;
			}else{
				int Size=sizeof(sockaddr);
				if (CheckSocketResult(getpeername(SocketHandle(), (SOCKADDR*)&m_RemoteSockAddrIn,&Size),eeGeneral,"getpeername()")==0){
					strcpy_s(m_RemoteAddress,sizeof(m_RemoteAddress)-1,inet_ntoa(m_RemoteSockAddrIn.sin_addr));
					m_RemoteAddress[sizeof(m_RemoteAddress)-1]=0;
					remoteaddress=1;
					remotesockaddrin=1;
					if(Buf!=NULL){
						strcpy_s(Buf,BufLen-1,m_RemoteAddress);
						Buf[BufLen-1]=0;
					}
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	return m_RemoteAddress;
}
//------------------------------------------------------------------------
int CLD_Socket::GetRemotePort()
{

	if (m_ulremoteport!=0 && remoteport!=0){return m_ulremoteport;}
	int nRetPort=m_ulremoteport;
	if (IsConnected()){
		__try
		{
			if (remotesockaddrin!=0){
				nRetPort= ntohs( m_RemoteSockAddrIn.sin_port );
				m_ulremoteport=nRetPort;
				remoteport=1;
			}else{
				int Size=sizeof(sockaddr);
				if (CheckSocketResult(getpeername(SocketHandle(), (SOCKADDR*)&m_RemoteSockAddrIn,&Size),eeGeneral,"getpeername()")==0){
					nRetPort= ntohs( m_RemoteSockAddrIn.sin_port );
					m_ulremoteport=nRetPort;
					remoteport=1;
					remotesockaddrin=1;
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	return nRetPort;
}
//------------------------------------------------------------------------
int CLD_Socket::ReceiveLength()
{

	u_long RecvSize=0;
	if (IsConnected())
	{
		__try
		{
			CheckSocketResult(ioctlsocket(SocketHandle(), FIONREAD, &RecvSize),eeGeneral,"ioctlsocket()");
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	return RecvSize;
}
//------------------------------------------------------------------------
int CLD_Socket::ReceiveBuf(char* Buf,int BufLen)
{
	FUNCTION_BEGIN;
	int nReceLen=0;
	if (IsConnected()){
		nReceLen=recv(SocketHandle(), Buf, BufLen, 0);
		if (nReceLen == SOCKET_ERROR){
			CheckSocketResult(WSAGetLastError(),eeReceive,"recv()");
			nReceLen=0;
		}
	}
	return nReceLen;
}
//------------------------------------------------------------------------
u_long CLD_Socket::LookupName(const char* szName)
{
	FUNCTION_BEGIN;
	hostent* pHostEnt=gethostbyname(szName);
	in_addr   in;
	if (pHostEnt && pHostEnt->h_addr_list[0])
	{
		in.s_addr= *((u_long *)pHostEnt->h_addr_list[0]);
		return in.s_addr;
	}
	return INADDR_NONE;
}
//------------------------------------------------------------------------
u_long CLD_Socket::LookupName(const char* szName,char* sIpBuf,int nIpBufLen)
{
	FUNCTION_BEGIN;
	in_addr  in;
	in.s_addr=LookupName(szName);
	if (in.s_addr!=INADDR_NONE){
		char* pip=inet_ntoa(in);
		if (pip){
			strcpy_s(sIpBuf,nIpBufLen,pip);
		}
	}
	return in.s_addr;
}
//------------------------------------------------------------------------
int CLD_Socket::SendText(const char* Buf,DWORD dwTimeWait)
{
	FUNCTION_BEGIN;
	return SendBuf(Buf,strlen(Buf),dwTimeWait);
}
//------------------------------------------------------------------------
int CLD_Socket::SendBuf(const char* Buf,int BufLen,DWORD dwTimeWait)
{
	FUNCTION_BEGIN;
	int nRet=0;
	if (dwTimeWait>0){
		do{
			AILOCKT(m_bottomlock);
			nRet=::send(SocketHandle(), Buf, BufLen, 0);
		}while(false);

		if(SOCKET_ERROR==nRet && WSAGetLastError()!=WSAEWOULDBLOCK){
			CheckSocketResult(WSAGetLastError(),eeSend,"send()");
			nRet=0;
		}
	}else{

		int thisSendCount=0;
		DWORD dwStartSend=::GetTickCount();

		INFOLOCK(m_bottomlock); 

		while (BufLen>0){
			if (!IsConnected()){break;}
			thisSendCount=::send(SocketHandle(), &Buf[nRet], BufLen, 0);
			if (thisSendCount>0){
				BufLen -= thisSendCount;
				nRet += thisSendCount;
				if ((dwTimeWait==0) || ((::GetTickCount()-dwStartSend)>=dwTimeWait) ){ break; }
			}else if(SOCKET_ERROR==thisSendCount && WSAGetLastError()!=WSAEWOULDBLOCK){
				UNINFOLOCK(m_bottomlock);
				CheckSocketResult(WSAGetLastError(),eeSend,"send()");
				nRet=0;
				return nRet;
			}else if ((dwTimeWait==0) || ((::GetTickCount()-dwStartSend)>=dwTimeWait) ){ break; }
		}
		UNINFOLOCK(m_bottomlock);
	}
	return nRet;
}
//------------------------------------------------------------------------
void CLD_Socket::OnError(CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg)
{

}
//------------------------------------------------------------------------
void CLD_Socket::DoDisconnect()
{
	FUNCTION_BEGIN;

	if ( SocketHandle()!=INVALID_SOCKET || m_boConnected ){

		bool do_ondisconnected=false;
		do{
			AILOCKT(m_bottomlock);
			if ( m_boConnected ){ 
				m_boConnected=false;
				do_ondisconnected=true; 
			}
		}while(false);
		if(do_ondisconnected){
			OnDisconnect();
		}
	}
}
//------------------------------------------------------------------------
void CLD_Socket::Error(CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg)
{
	FUNCTION_BEGIN;
	OnError(ErrEvent,nErrCode,sErrMsg);


}
//------------------------------------------------------------------------
int CLD_Socket::CheckSocketResult(int nResultCode,CLD_TErrorEvent ErrEvent,char* sOp)
{
	int nRet=0;
	if( nResultCode != 0 ){
		nRet=WSAGetLastError();
		if(nRet!=WSAEWOULDBLOCK){

			char sWinErrMsgBuf[512]={0};
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY,NULL,nRet!=0?nRet:nResultCode,0,sWinErrMsgBuf,sizeof(sWinErrMsgBuf)-1,NULL);

			char* p=strrchr( sWinErrMsgBuf, '\r' );
			if (p){
				*(p) = ' ';
			}
			char sErrMsgBuf[512]={0};
			sprintf_s(sErrMsgBuf,sizeof(sErrMsgBuf)-1,"(%d)Windows Socket error: %s ( %d : %d ), on API -->  %s",::GetCurrentProcessId(),sWinErrMsgBuf,nResultCode,nRet,sOp);
			bool boerroring=false;
			{
				AILOCKT(m_bottomlock);
				if ( !m_boErroring ){ m_boErroring=true; }
				else{ boerroring=true; }
			}
			if(!boerroring){

				nRet=nResultCode;
				Error(ErrEvent,nRet,sErrMsgBuf);
				{
					AILOCKT(m_bottomlock);
					m_boErroring=false;
				}
				if(nRet!=0){

					MessageBox(0,sErrMsgBuf,"SocketError",MB_OK | MB_ICONSTOP);
					throw CLDError(sErrMsgBuf,ERROR_CLOSESOCKET);
				}
			}
			return nResultCode;
		}
	}
	return 0;
}
//------------------------------------------------------------------------
#include <malloc.h>
//------------------------------------------------------------------------
int CLD_Socket::GetAdapterInfo(PIP_ADAPTER_INFO pOutAdapter,int nSize)
{
	FUNCTION_BEGIN;
	char tempChar;
	ULONG uListSize=1;
	PIP_ADAPTER_INFO pAdapter;   

	int nAdapterIndex = 0;

	DWORD dwRet = GetAdaptersInfo((PIP_ADAPTER_INFO)&tempChar,&uListSize);		//返回与系统关联的IP地址信息,填充一个IP_ADAPTER_INFO结构指针。

	if (dwRet == ERROR_BUFFER_OVERFLOW){
		STACK_ALLOCA(char*,pbuffer,uListSize+128);
		PIP_ADAPTER_INFO pAdapterListBuffer=(PIP_ADAPTER_INFO)pbuffer;
		if (pAdapterListBuffer){
			dwRet = GetAdaptersInfo(pAdapterListBuffer, &uListSize);
			if (dwRet == ERROR_SUCCESS){
				pAdapter = pAdapterListBuffer;
				while (pAdapter && nAdapterIndex<nSize){
					if (pAdapter->Type== MIB_IF_TYPE_ETHERNET){
						char strTemp[MAX_PATH] = {0};
						sprintf_s(strTemp,sizeof(strTemp),"\\Device\\NPF_%s",pAdapter->AdapterName);

						pOutAdapter[nAdapterIndex]=(*pAdapter);
						pOutAdapter[nAdapterIndex].Next=NULL;
						strcpy_s(pOutAdapter[nAdapterIndex].AdapterName,(sizeof(pOutAdapter[nAdapterIndex].AdapterName)),strTemp);
					}
					pAdapter = pAdapter->Next;
					nAdapterIndex ++;
				}
			}
		}
	}
	return nAdapterIndex;
}
//------------------------------------------------------------------------
char *CLD_Socket::getIPByIfName(const char *ifName)
{
	FUNCTION_BEGIN;
	IP_ADAPTER_INFO Adapter[16];
	int nCount=GetAdapterInfo(Adapter,16);
	for (int i=0;i<nCount;i++)
	{
		if (strcmp(ifName,Adapter[i].AdapterName)==0)
		{
			return NULL;
		}
	}
	return NULL;
}
//------------------------------------------------------------------------
void  CLD_Socket::SetBindParam(void* lpParam)
{
	FUNCTION_BEGIN;
	m_lpParam=lpParam;
}
//------------------------------------------------------------------------
void*  CLD_Socket::GetBindParam()
{
	FUNCTION_BEGIN;
	return m_lpParam;
}
//------------------------------------------------------------------------
void CLD_Socket::Close()
{
	FUNCTION_BEGIN;
	DoDisconnect();
	AILOCKT(m_bottomlock);
	closesocket_safe(SocketHandle(),true);
	Attach(INVALID_SOCKET);
	m_btinfoflag=0;
}
//------------------------------------------------------------------------
void CLD_Socket::SetKeepAlive(bool bokeepalive,u_long keepalive_time,u_long keepalive_interval)
{
	FUNCTION_BEGIN;

	if (SocketHandle()!=INVALID_SOCKET)
	{
		DWORD	nBytesReturned=0;
		tcp_keepalive		keepAlive = { bokeepalive==true?1:0 , keepalive_time, keepalive_interval };
		WSAIoctl( SocketHandle(), SIO_KEEPALIVE_VALS, &keepAlive, sizeof( keepAlive ), 
			0, 0, &nBytesReturned, NULL, NULL );
	}
	return;
}
//------------------------------------------------------------------------
void CLD_Socket::Attach(SOCKET s)
{
	AILOCKT(m_bottomlock); 
	m_Socket=(s==0?INVALID_SOCKET:s);
	m_boConnected=(m_Socket!=INVALID_SOCKET);
	if (m_boConnected){

		m_btinfoflag=0;
	} 
}
//------------------------------------------------------------------------
int CLD_Socket::closesocket_safe(SOCKET sock,bool boshutdown)
{
	if (sock!=INVALID_SOCKET){
		if (boshutdown){
			LINGER lingerStruct;
			lingerStruct.l_onoff = 1;
			lingerStruct.l_linger = 0;
			setsockopt(sock, SOL_SOCKET, SO_LINGER,
				(char *)&lingerStruct, sizeof(lingerStruct) );

			shutdown(sock, SD_BOTH);
		}
		return closesocket(sock);
	}
	return 0;
}
//------------------------------------------------------------------------
SOCKET CLD_Socket::accept_safe(SOCKET srvsock)
{
	sockaddr_in SockAddrIn;
	int Size=sizeof(sockaddr_in);
	return accept(srvsock,(SOCKADDR*)&SockAddrIn,&Size);
}
//------------------------------------------------------------------------
SOCKET CLD_Socket::socket_safe()
{
	FUNCTION_BEGIN;
	bool boOpenFlag=false;
	m_btinfoflag=0;
	Close();
	do{
		AILOCKT(m_bottomlock);

	}while(false);
	Attach(socket(PF_INET, SOCK_STREAM, IPPROTO_IP));
	if (SocketHandle()==INVALID_SOCKET){
		CheckSocketResult(WSAGetLastError(),eeConnect,"creat open Socket()");
		boOpenFlag=false;
	}else{	
		boOpenFlag= true;
	}
	return boOpenFlag;
}
//------------------------------------------------------------------------
void CLD_Socket::Lookup(char* szLookupIp)
{
	FUNCTION_BEGIN;
	OnLookup(szLookupIp);
}
//------------------------------------------------------------------------
CLD_AsyncHandle::CLD_AsyncHandle()
{
	FUNCTION_BEGIN;
	m_pSocke=NULL;
	ZeroMemory(m_sAddr,sizeof(m_sAddr));
	m_wport=0;
	m_ObjectWndProcInstance=NULL;
	m_LookupState=lsIdle;
	m_LookupHandle=0;
	m_GetHostData=NULL;
	m_AsyncStyles=0;
}
//------------------------------------------------------------------------
CLD_AsyncHandle::~CLD_AsyncHandle()
{
	FUNCTION_BEGIN;
	ResetAsync();
	if(m_ObjectWndProcInstance){
		DeallocateHWnd(m_ObjectWndProcInstance);
		m_ObjectWndProcInstance=NULL;
	}
	if(m_GetHostData){
		__mt_char_alloc.deallocate(m_GetHostData);
		m_GetHostData=NULL;
	}
	m_pSocke=NULL;
}
//------------------------------------------------------------------------
int CLD_AsyncHandle::SetAsyncStyles(int Styles)
{
	FUNCTION_BEGIN;
	int nRet=m_AsyncStyles;

	m_AsyncStyles=Styles;
	if (m_pSocke && m_pSocke->SocketHandle()!=INVALID_SOCKET){
		m_pSocke->CheckSocketResult(WSAAsyncSelect(m_pSocke->SocketHandle(),GetHandle(),CM_SOCKETMESSAGE,m_AsyncStyles),eeGeneral,"WSAAsyncSelect()");
	}
	return nRet;
}
//------------------------------------------------------------------------
void CLD_AsyncHandle::ResetAsync()
{

	if(m_LookupHandle!=0){
		__try
		{
			if (m_pSocke){
				m_pSocke->CheckSocketResult(WSACancelAsyncRequest(m_LookupHandle),eeDisconnect,"WSACancelAsyncRequest()");
			}else{
				WSACancelAsyncRequest(m_LookupHandle);
			}
		}__except(EXCEPTION_EXECUTE_HANDLER){
		}
		m_LookupState=lsIdle;
		m_LookupHandle=0;	
	}
	ZeroMemory(m_sAddr,sizeof(m_sAddr));
	m_wport=0;
}
//------------------------------------------------------------------------
int CLD_AsyncHandle::GetAsyncStyles()
{
	FUNCTION_BEGIN;
	return m_AsyncStyles;
}
//------------------------------------------------------------------------
HWND CLD_AsyncHandle::GetHandle()
{
	FUNCTION_BEGIN;
	HWND Wnd=0;
	if(m_AsyncStyles!=0){
		if(!m_ObjectWndProcInstance){
			WNDPROC lpCallBackWndProc=NULL;

			_asm push CallBackWndProc
				_asm pop lpCallBackWndProc
				m_ObjectWndProcInstance=AllocateHWnd(lpCallBackWndProc,this);
		}
		if(m_ObjectWndProcInstance){
			Wnd=m_ObjectWndProcInstance->Wnd;
		}	
	}
	return Wnd;
}
//------------------------------------------------------------------------
bool CLD_AsyncHandle::ASyncInitSocket(const char * sAddr,u_short nPort,bool boIsIp)
{
	FUNCTION_BEGIN;

	bool boFlag=false;
	if (m_pSocke)
	{
		try
		{
			switch( m_LookupState)
			{
			case lsIdle:
				{

					if(sAddr!=NULL){
						m_LookupState=lsLookupAddress;
						m_wport=nPort;
						m_pSocke->m_ulremoteport=nPort;
						if(!boIsIp){
							if(!m_GetHostData){
								m_GetHostData=__mt_char_alloc.allocate(MAXGETHOSTSTRUCT);
							}
							if (!m_LookupHandle){
								WSACancelAsyncRequest(m_LookupHandle);m_LookupHandle=0;
							}
							m_LookupHandle=WSAAsyncGetHostByName(GetHandle(),CM_LOOKUPCOMPLETE,sAddr,(char *)m_GetHostData,MAXGETHOSTSTRUCT);


							m_pSocke->CheckSocketResult((int)(m_LookupHandle == 0),eeLookup, "WSAAsyncGetHostByName()");

							strcpy_s(m_pSocke->m_RemoteHost,sizeof(m_pSocke->m_RemoteHost)-1,sAddr);
							m_pSocke->m_RemoteHost[sizeof(m_pSocke->m_RemoteHost)-1]=0;

							return true;
						}else{
							strcpy_s(m_sAddr,sizeof(m_sAddr)-1,sAddr); 
							m_sAddr[sizeof(m_sAddr)-1]=0;

							strcpy_s(m_pSocke->m_RemoteAddress,sizeof(m_pSocke->m_RemoteAddress)-1,sAddr);
							m_pSocke->m_RemoteAddress[sizeof(m_pSocke->m_RemoteAddress)-1]=0;
						}
					}
				}
				break;
			case lsLookupAddress:
				{
					m_LookupState=lsLookupService;

				}
				break;
			case lsLookupService:
				{
					m_LookupState=lsIdle;
					m_pSocke->DoOpen();

				}
				break;
			}
			if(m_LookupState!=lsIdle){
				ASyncInitSocket(sAddr,nPort,boIsIp);
			}
			boFlag=  true;
		}catch(CLDError& Err){
			if(Err.GetErrorCode()==ERROR_CLOSESOCKET && m_pSocke){
				m_pSocke->Close();
			}
			boFlag= false;
		}catch (...){
			if(m_pSocke){
				m_pSocke->Close();
			}
			boFlag= false;
		}
	}
	return boFlag;
}
//------------------------------------------------------------------------
LPARAM CLD_AsyncHandle::CallBackWndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return this->WndProc(hWnd,nMsg,wParam,lParam);
}
//------------------------------------------------------------------------
void CLD_AsyncHandle::OnSocketMsg(SOCKET s,WORD wSelectEvent,WORD wSelectError)
{
	int nErrCode=wSelectError;
	char sErrMsg[128];	
	if(wSelectError!=0){
		sprintf_s(sErrMsg,sizeof(sErrMsg),"Asynchronous Socket error %d",nErrCode);
	}
	switch(wSelectEvent)
	{
	case FD_CONNECT:
		{
			if(wSelectError!=0){
				m_pSocke->Error(eeConnect,nErrCode,sErrMsg);
			}else{
				m_pSocke->m_boConnected=true;
				m_pSocke->m_btinfoflag=0;
				m_pSocke->DoConnect();			
			}
		}
		break;
	case FD_CLOSE:
		{
			if(wSelectError!=0){
				m_pSocke->Error(eeDisconnect,nErrCode,sErrMsg);
			}else{
				m_pSocke->Close();
			}
		}
		break;
	case FD_READ:
		{
			if(wSelectError!=0){
				m_pSocke->Error(eeReceive,nErrCode,sErrMsg);
			}else{
				m_pSocke->DoRead();		
			}
		}
		break;
	case FD_WRITE:
		{
			if(wSelectError!=0){
				m_pSocke->Error(eeSend,nErrCode,sErrMsg);
			}else{
				m_pSocke->DoWrite();
			}
		}
		break;
	case FD_ACCEPT:
		{
			if(wSelectError!=0){
				m_pSocke->Error(eeAccept,nErrCode,sErrMsg);
			}else{
				m_pSocke->Accept();
			}
		}
		break;
	default:
		{
		}
		break;
	}	
}
//------------------------------------------------------------------------
void CLD_AsyncHandle::OnCMDeferFree(CLD_Socket* SockObj)
{
	SAFE_DELETE(SockObj);
	SockObj=NULL;
}
//------------------------------------------------------------------------
void CLD_AsyncHandle::OnCMLookupComplete(HANDLE LookupHandle,WORD wAsyncBufLen,WORD wAsyncError) 
{
	if(wAsyncError!=0){
		m_LookupHandle=0;
		m_pSocke->CheckSocketResult(wAsyncError,eeLookup,"ASync Lookup Msg");
		m_pSocke->Close();
	}else{
		if((m_LookupState==lsLookupAddress)&&(LookupHandle==m_LookupHandle))
		{
			if(m_GetHostData!=NULL)
			{
				m_LookupHandle=0;
				hostent* phe=(hostent*)m_GetHostData;
				m_sAddr[0]=0;
				if (phe->h_addr_list[0] != NULL)
				{
					strcpy_s(m_sAddr,sizeof(m_sAddr)-1,inet_ntoa(*((in_addr*)phe->h_addr_list[0])));
				}
				m_sAddr[sizeof(m_sAddr)-1]=0;
				strcpy_s(m_pSocke->m_RemoteAddress,sizeof(m_pSocke->m_RemoteAddress)-1,m_sAddr);
				m_pSocke->m_RemoteAddress[sizeof(m_pSocke->m_RemoteAddress)-1]=0;
				m_pSocke->Lookup(m_sAddr);
				ASyncInitSocket(m_sAddr,m_wport,true);
				__mt_char_alloc.deallocate(m_GetHostData);
				m_GetHostData=NULL;
			}
		}
	}
}
//------------------------------------------------------------------------
LPARAM APIENTRY CLD_AsyncHandle::WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd==GetHandle() && m_pSocke )
	{
		switch(nMsg)
		{
		case CM_SOCKETMESSAGE:
			{

				if(((SOCKET)wParam==m_pSocke->SocketHandle()) 
					&&((SOCKET)wParam!=INVALID_SOCKET)
					&& (m_pSocke->IsConnected() || FD_CONNECT==WSAGETSELECTEVENT(lParam)))
				{
					OnSocketMsg(m_pSocke->SocketHandle(),WSAGETSELECTEVENT(lParam),WSAGETSELECTERROR(lParam));
					return 1;
				}
			}
			break;
		case CM_LOOKUPCOMPLETE:
			{
				OnCMLookupComplete(m_LookupHandle,WSAGETSELECTEVENT(lParam),WSAGETSELECTERROR(lParam));
				return 1;
			}
			break;
		case CM_DEFERFREE:
			{
				OnCMDeferFree((CLD_Socket*)MAKELONG(wParam,lParam));
				return 1;
			}
			break;
		default:
			{
			}
			break;
		}
	}
	return m_ObjectWndProcInstance->OldWndProcAddr(hWnd,nMsg,wParam,lParam);
}
//------------------------------------------------------------------------
CLD_ClientSocket::CLD_ClientSocket(CLD_Accepter* Owner,SOCKET s)
:CLD_Socket()
{
	FUNCTION_BEGIN;
	Attach(s);
	if (Owner)
	{
		m_Owner=Owner;

	}
}
//------------------------------------------------------------------------
CLD_ClientSocket::~CLD_ClientSocket()
{
	FUNCTION_BEGIN;
	if (m_Owner)
	{
		m_Owner->RemoveClient(this);
		m_Owner=NULL;
	}
}
//------------------------------------------------------------------------
void CLD_ClientSocket::DoRead()
{
	FUNCTION_BEGIN;
	if (m_Owner)
	{
		m_Owner->OnClientRead(this);
	}
}
//------------------------------------------------------------------------
void CLD_ClientSocket::DoWrite()
{
	FUNCTION_BEGIN;
	if (m_Owner)
	{
		m_Owner->OnClientWrite(this);
	}
}
//------------------------------------------------------------------------
void CLD_ClientSocket::Error(CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg)
{
	FUNCTION_BEGIN;
	if (m_Owner){
		m_Owner->OnClientError(this,ErrEvent,nErrCode,sErrMsg);
	}else{
		OnError(ErrEvent,nErrCode,sErrMsg);
	}
}
//------------------------------------------------------------------------
void CLD_ClientSocket::OnDisconnect(){
	if (m_Owner){
		m_Owner->OnClientDisconnect(this);
	}
};
//------------------------------------------------------------------------
void CLD_ClientSocket::DoDisconnect()
{
	FUNCTION_BEGIN;
	CLD_Socket::DoDisconnect();
	DeferFree();
}
//------------------------------------------------------------------------
void CLD_ClientSocket::DeferFree()
{
	if (this){delete this;};
}
//------------------------------------------------------------------------
bool CLD_Accepter::Open(const char * sAddr,u_short nPort)
{
	FUNCTION_BEGIN;
	try
	{
		if (socket_safe()!=INVALID_SOCKET)
		{
			sockaddr_in SockAddrIn;

			SockAddrIn.sin_family=PF_INET;
			SockAddrIn.sin_addr.s_addr  =inet_addr(sAddr);
			SockAddrIn.sin_port=htons(nPort);
			m_ullocalport=nPort;
			strcpy_s(m_LocalAddress,sizeof(m_LocalAddress)-1,sAddr);
			m_LocalAddress[sizeof(m_LocalAddress)-1]=0;
			if (CheckSocketResult(bind(SocketHandle(),(SOCKADDR*)&SockAddrIn,sizeof(sockaddr_in)),eeListen,"bind()")==0)
			{
				if (CheckSocketResult(listen(SocketHandle(),SOMAXCONN),eeListen,"listen()")==0)
				{
					m_boConnected=true;
					m_btinfoflag=0;
					return true;
				}
			}
		}
	}
	catch(CLDError& Err){
		if(Err.GetErrorCode()==ERROR_CLOSESOCKET){
			Close();
		}
	}
	catch (...){
		Close();
	}
	return false;
}
//------------------------------------------------------------------------
CLD_Socket*  CLD_Accepter::Accept()
{
	FUNCTION_BEGIN;
	try
	{
		SOCKET ClientWinSocket;
		ClientWinSocket=accept_safe(SocketHandle());
		if (ClientWinSocket!=INVALID_SOCKET)
		{
			OnAccept(ClientWinSocket);
			CLD_ClientSocket * pClient=CreateClient(ClientWinSocket);
			if (pClient)
			{				
				if (!AddClient(pClient))
				{
					SAFE_DELETE(pClient);
					return NULL;
				}
				else
				{					
					OnClientConnect(pClient);
					return pClient;
				}
			}
			else
			{
				int ErrorCode=0;
				Error(eeAcceptCreatClient,ErrorCode,"accepter creatclient error!");
				CheckSocketResult(closesocket_safe(ClientWinSocket,true),eeDisconnect,"Closesocket()");
			}
		}
		else
		{
			CheckSocketResult(WSAGetLastError(),eeAccept,"Accept()");
		}
	}
	catch (...)
	{
		g_logger.error("error at CLD_Accepter::Accept()");
	}
	return NULL;
}
//------------------------------------------------------------------------
CLD_Accepter::~CLD_Accepter(){
	CloseAllClient();

	CServerClientSocketList::iterator item;
	CLD_ClientSocket* pClient=NULL;
	do {
		AILOCKT(m_ClientList);
		for (item=m_ClientList.begin();item!=m_ClientList.end();item++)
		{
			pClient=(*item);
			if (pClient){pClient->m_Owner=NULL;}
		}
		m_ClientList.clear();
	} while (false);
}
//------------------------------------------------------------------------
bool CLD_Accepter::AddClient(CLD_ClientSocket * Client)
{
	FUNCTION_BEGIN;
	AILOCKT(m_ClientList);
	try
	{
		if ( m_ClientList.find(Client)==m_ClientList.end() && IsConnected() )
		{
			Client->m_Owner=this;
			m_ClientList.insert(Client);	
			return true;
		}
	}
	catch(...)
	{
	}
	return false;
}
//------------------------------------------------------------------------
bool CLD_Accepter::RemoveClient(CLD_ClientSocket *Client)
{
	FUNCTION_BEGIN;
	AILOCKT(m_ClientList);
	try
	{
		if(!m_ClientList.empty())
		{
			CServerClientSocketList::tbase::iterator it=m_ClientList.find(Client);
			if (it!=m_ClientList.end())
			{
				(*it)->m_Owner=NULL;
				m_ClientList.erase(it);
			}
		}
	}
	catch(...)
	{
	}
	return true;
}
//------------------------------------------------------------------------
void CLD_Accepter::CloseClient(CLD_Socket* Socket){
	FUNCTION_BEGIN;
	if (Socket){
		Socket->Close();
	}
}
//------------------------------------------------------------------------
void CLD_Accepter::CloseAllClient(){

	FUNCTION_BEGIN;

	CServerClientSocketList::iterator item;
	CLD_ClientSocket* pClient=NULL;

	list< CLD_ClientSocket* >::iterator tmpitem;
	list< CLD_ClientSocket* > templist;

	do{
		AILOCKT(m_ClientList);
		for (item=m_ClientList.begin();item!=m_ClientList.end();item++){
			pClient=(*item);
			if (pClient&& pClient->IsConnected()){
				templist.push_back(pClient);
			}
		}
	}while(false);

	for (tmpitem=templist.begin();tmpitem!=templist.end();tmpitem++){
		pClient=(*tmpitem);
		if (pClient){


			pClient->Close();
		}
	}
}
//------------------------------------------------------------------------
void CLD_Accepter::DoDisconnect()
{
	FUNCTION_BEGIN;
	CLD_Socket::DoDisconnect();

	CloseAllClient();
}
//------------------------------------------------------------------------
bool CLD_Connecter::Open(const char * sAddr,u_short nPort)
{
	FUNCTION_BEGIN;
	try
	{
		if (socket_safe()!=INVALID_SOCKET){
			m_boConnected=false;
			sockaddr_in SockAddrIn;

			u_long naddr=inet_addr(sAddr);
			if (naddr==INADDR_NONE){
				naddr=LookupName(sAddr,m_RemoteAddress,sizeof(m_RemoteAddress));
			}else{
				strcpy_s(m_RemoteAddress,sizeof(m_RemoteAddress)-1,sAddr);
				m_RemoteAddress[sizeof(m_RemoteAddress)-1]=0;
			}

			m_ulremoteport=nPort;
			SockAddrIn.sin_family=PF_INET;
			SockAddrIn.sin_addr.s_addr  =naddr;
			SockAddrIn.sin_port=htons(nPort);

			if (CheckSocketResult(connect(SocketHandle(),(SOCKADDR*)&SockAddrIn,sizeof(sockaddr_in)),eeConnect,"connect()")==0){
				m_boConnected=true;
				m_btinfoflag=0;
				DoConnect();
				return true;
			}
		}
	}
	catch (...){
		Close();
	}
	return false;
}
//------------------------------------------------------------------------
void CLD_Connecter::DoDisconnect()
{
	FUNCTION_BEGIN;
	CLD_Socket::DoDisconnect();
}
//------------------------------------------------------------------------
CLD_AsyncClientSocket::CLD_AsyncClientSocket(CLD_Accepter* Owner,SOCKET s)
:CLD_ClientSocket(Owner,s)
{
	FUNCTION_BEGIN;
	m_AsyncHandle.m_pSocke=this;
	m_AsyncHandle.SetAsyncStyles(CLIENT_SOCKET_EVENT);
}
//------------------------------------------------------------------------
CLD_AsyncClientSocket::~CLD_AsyncClientSocket()
{
	FUNCTION_BEGIN;
}
//------------------------------------------------------------------------
void CLD_AsyncClientSocket::DeferFree()
{
	FUNCTION_BEGIN;
	if (m_AsyncHandle.GetHandle())
	{
		::PostMessageA(m_AsyncHandle.GetHandle() ,CM_DEFERFREE,(WPARAM)LOWORD((DWORD)this),(LPARAM)HIWORD((DWORD)this));
	}
}
//------------------------------------------------------------------------
CLD_AsyncAccepter::CLD_AsyncAccepter()
	:CLD_Accepter()
{
	FUNCTION_BEGIN;
	m_AsyncHandle.m_pSocke=this;
	m_AsyncHandle.SetAsyncStyles(SERVER_SOCKET_EVENT);
}
//------------------------------------------------------------------------
CLD_AsyncAccepter::~CLD_AsyncAccepter()
{
	FUNCTION_BEGIN;
}
//------------------------------------------------------------------------
bool CLD_AsyncAccepter::Open(const char * sAddr,u_short nPort)
{
	FUNCTION_BEGIN;
	if (socket_safe()!=INVALID_SOCKET){
		return m_AsyncHandle.ASyncInitSocket(sAddr,nPort,(inet_addr(sAddr)!=INADDR_NONE));
	}
	return false;
}
//------------------------------------------------------------------------
void CLD_AsyncAccepter::DoOpen()
{
	FUNCTION_BEGIN;
	if (SocketHandle()!=INVALID_SOCKET)
	{
		sockaddr_in SockAddrIn;

		SockAddrIn.sin_family=PF_INET;
		SockAddrIn.sin_addr.s_addr  =inet_addr(m_AsyncHandle.m_sAddr);
		SockAddrIn.sin_port=htons(m_AsyncHandle.m_wport);
		m_ullocalport=m_AsyncHandle.m_wport;
		strcpy_s(m_LocalAddress,sizeof(m_LocalAddress)-1,m_AsyncHandle.m_sAddr);
		m_LocalAddress[sizeof(m_LocalAddress)-1]=0;

		if (CheckSocketResult(bind(SocketHandle(),(SOCKADDR*)&SockAddrIn,sizeof(sockaddr_in)),eeListen,"bind()")==0){
			m_AsyncHandle.SetAsyncStyles(m_AsyncHandle.m_AsyncStyles);

			if (CheckSocketResult(listen(SocketHandle(),SOMAXCONN),eeListen,"listen()")==0){
				m_boConnected=true;
				m_btinfoflag=0;
			}
		}
		m_AsyncHandle.m_LookupState=lsIdle;
	}
}
//------------------------------------------------------------------------
void CLD_AsyncAccepter::DoDisconnect()
{
	FUNCTION_BEGIN;
	CLD_Accepter::DoDisconnect();
	m_AsyncHandle.ResetAsync();
}
//------------------------------------------------------------------------
CLD_AsyncConnecter::CLD_AsyncConnecter()
{
	FUNCTION_BEGIN;
	m_AsyncHandle.m_pSocke=this;
	m_AsyncHandle.SetAsyncStyles(CONNETER_SOCKET_EVENT);
}
//------------------------------------------------------------------------
CLD_AsyncConnecter::~CLD_AsyncConnecter()
{
	FUNCTION_BEGIN;
}
//------------------------------------------------------------------------
bool CLD_AsyncConnecter::Open(const char * sAddr,u_short nPort)
{
	FUNCTION_BEGIN;
	if (socket_safe()!=INVALID_SOCKET){
		m_boConnected=false;
		return m_AsyncHandle.ASyncInitSocket(sAddr,nPort,(inet_addr(sAddr)!=INADDR_NONE));
	}
	return false;
}
//------------------------------------------------------------------------
void CLD_AsyncConnecter::DoOpen()
{
	FUNCTION_BEGIN;
	if (SocketHandle()!=INVALID_SOCKET)
	{	
		m_AsyncHandle.SetAsyncStyles(m_AsyncHandle.m_AsyncStyles);

		sockaddr_in SockAddrIn;
		SockAddrIn.sin_family=PF_INET;
		SockAddrIn.sin_addr.s_addr  =inet_addr(m_AsyncHandle.m_sAddr);
		SockAddrIn.sin_port=htons(m_AsyncHandle.m_wport);

		CheckSocketResult(connect(SocketHandle(),(SOCKADDR*)&SockAddrIn,sizeof(sockaddr_in)),eeConnect,"connect()");

		m_AsyncHandle.m_LookupState=lsIdle;
	}
}
//------------------------------------------------------------------------
void CLD_AsyncConnecter::DoDisconnect()
{
	FUNCTION_BEGIN;
	CLD_Connecter::DoDisconnect();
	m_AsyncHandle.ResetAsync();
}
//------------------------------------------------------------------------