/**
*	created:		2013-4-7   23:34
*	filename: 		FKSocketTask
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../Endec/FKEncDec.h"
#include "FKIocp.h"
#include "FKPacket.h"
#include "../FKMisc.h"
#include "../FKLogger.h"
#include "../STLTemplate/FKStreamQueue.h"
//------------------------------------------------------------------------
extern unsigned char connect_def_key_16_byte[16];
//------------------------------------------------------------------------
//检查连接是否正常 cmd=255  subcmd=254
#define _CHECK_SIGNAL_CMD_					0xff
#define _CHECK_SIGNAL_SUBCMDCMD_			0xfe
//------------------------------------------------------------------------
struct stCheckSignalCmd:public stCmdBase<_CHECK_SIGNAL_CMD_,_CHECK_SIGNAL_SUBCMDCMD_>{
	bool isneedACK;
	BYTE checknum;
	stCheckSignalCmd(bool isfirstsend,BYTE ch=0):isneedACK(isfirstsend),checknum(ch){};
	stCheckSignalCmd(const stCheckSignalCmd* psrc):isneedACK(false),checknum((psrc!=NULL)?psrc->checknum:0){};
};
//------------------------------------------------------------------------
#define _CHECK_SPEED_SUBCMDCMD_			0xfd
//------------------------------------------------------------------------
struct stCheckSpeedCmd:public stCmdBase<_CHECK_SIGNAL_CMD_,_CHECK_SPEED_SUBCMDCMD_>{
	DWORD dwLocalTick;
	DWORD dwCheckIndex;
	DWORD dwProxyCount;
	DWORD dwCurtime;
	char szCheckStr[128];
#define _MAX_LOG_COUNT_		16
	struct{
		DWORD svr_id_type;
		DWORD dwLoaclTick;
		DWORD dwCurtime;
	}ProcessLogs[_MAX_LOG_COUNT_];
	stCheckSpeedCmd(DWORD dwidx,const char* pszCS){
		dwCheckIndex=dwidx;
		strcpy_s(szCheckStr,sizeof(szCheckStr)-1,pszCS);
		dwLocalTick=::GetTickCount();
	}
	void setLog(DWORD svr_it){
		dwProxyCount++;
		return;
	}
};
//------------------------------------------------------------------------
enum eCheckSignalState{
	checksignalstate_no,
	checksignalstate_waitrecv,
};
//------------------------------------------------------------------------
enum TerminateMethod {
	terminate_no,
	terminate_client_active,
	terminate_server_active,
};
//------------------------------------------------------------------------
struct stQueueMsg{
	DWORD cmdsize;
	DWORD pluscmdoffset;
	stBaseCmd cmdBuffer;

	__inline stBaseCmd* pluscmd(){
		return (stBaseCmd*)((DWORD)(&cmdBuffer)+pluscmdoffset);
	}
	__inline DWORD pluscmdsize(){
		return (cmdsize-pluscmdoffset);
	}
};
//------------------------------------------------------------------------
struct stQueueMsgParam
{
	stQueueMsg* pQueueMsgBuffer;
	bool bofreebuffer;
	stQueueMsgParam(stQueueMsg* p,bool bo):bofreebuffer(bo),pQueueMsgBuffer(p){};
};
//------------------------------------------------------------------------
template < class _RET >
__inline bool NewPacketBuffer(_RET*& msg,size_t size){
	return CSafeMsgQueue::NewPushBuffer(msg,size);
};
//------------------------------------------------------------------------
template < class _RET >
__inline bool FreePacketBuffer(_RET*& msg){
	return CSafeMsgQueue::FreePushBuffer(msg);
};
//------------------------------------------------------------------------
int __stdcall getmsg2buf(const char*& pbuf, unsigned int& nbuflen, CEncrypt* enc, unsigned int& _de_size,stBaseCmd* pmsg,int nmsgmaxlen);
int __stdcall packetbuf(unsigned char* pin, unsigned long ninlen, unsigned char* pout,unsigned long nmaxlen, CEncrypt* enc, int zliblevel,bool issplit);
//------------------------------------------------------------------------
extern _TH_VAR_ARRAY(char,tls_sendpacket_charbuffer,_MAX_SEND_PACKET_SIZE_+1);
__inline int __stdcall getsafepacketbuflen(){ return _TH_VAR_SIZEOF(tls_sendpacket_charbuffer); };
__inline char* __stdcall getsafepacketbuf(){ return ((char*)_TH_VAR_PTR(tls_sendpacket_charbuffer)); };
//------------------------------------------------------------------------
class CLoopbufIocpClientSocketTask : public CLD_LoopbufIocpClientSocket 
{
protected:
	time_t m_conntime;
	TerminateMethod terminate;
	DWORD m_dwMaxRecvPacketLen;

	CEncrypt* m_pUpperEncodeSetter;
	unsigned int _de_size;						
	unsigned int _rcv_size_last;				
	time_t m_checkconntime;
	BYTE m_checksignalstate;

	bool m_boaddcmds;
	CSafeMsgQueue m_msg;
public:
	CLoopbufIocpClientSocketTask(CLD_IocpBaseAccepter* Owner, SOCKET s,int nLoopbufSize=DEF_LOOPBUF_SIZE);

	static void RecycleThreadCallBackProxy(void* pobj){
		((CLoopbufIocpClientSocketTask *) pobj)->RecycleThreadCallBack();
	}
	virtual void RecycleThreadCallBack();

	virtual void DoRead(); 
	virtual void DoDisconnect();

	virtual void run();
	virtual bool msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){ return true; }
	virtual void pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	virtual bool packetCheck(stBasePacket* ppacket,bool isfullpacket);

	bool isTerminate() const{	return terminate_no != terminate;}
	virtual void Terminate(const char* ffline="",const TerminateMethod method = terminate_server_active);
	virtual bool isvalid(){return true;};
	virtual time_t valid_timeout();
	virtual time_t check_signal_interval();
	virtual time_t check_signal_waittime();

	bool sendcmd(void* pbuf, unsigned int nsize, int zliblevel = Z_DEFAULT_COMPRESSION );

	template< class cmdT> bool SendCmdEx(cmdT& cmd, int zliblevel = Z_DEFAULT_COMPRESSION  ){
		return sendcmd(&cmd, sizeof(cmd),zliblevel);
	}

	bool addcmd(void* pbuf, unsigned int nsize, int zliblevel = Z_DEFAULT_COMPRESSION );

	template< class cmdT> bool addcmdex(cmdT& cmd, int zliblevel = Z_DEFAULT_COMPRESSION  ){
		return addcmd(&cmd, sizeof(cmd),zliblevel);
	}

	bool postcmds();
};
//------------------------------------------------------------------------
class CLoopbufIocpConnecterTask : public CLD_LoopbufIocpConnecter
{
protected:
	time_t m_conntime;
	TerminateMethod terminate;
	DWORD m_dwMaxRecvPacketLen;

	CEncrypt* m_penc;
	unsigned int _de_size;					
	unsigned int _rcv_size_last;			
	time_t m_checkconntime;
	BYTE m_checksignalstate;
	bool m_boaddcmds;

	CSafeMsgQueue m_msg;
public:
	CLoopbufIocpConnecterTask(CLD_IocpHandle* Owner,int nLoopbufSize=DEF_LOOPBUF_SIZE);

	static void RecycleThreadCallBackProxy(void* pobj){
		((CLoopbufIocpConnecterTask *) pobj)->RecycleThreadCallBack();
	}
	virtual void RecycleThreadCallBack();

	virtual void OnRead();
	virtual void OnIocpConnect();

	virtual void OnDisconnect();
	virtual void OnError(CLD_TErrorEvent ErrEvent, OUT int& nErrCode, char* sErrMsg);

	virtual void run();
	virtual bool msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){ return true; }
	virtual void pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	virtual bool packetCheck(stBasePacket* ppacket,bool isfullpacket);

	bool isTerminate() const{return terminate_no != terminate;}
	virtual void Terminate(const char* ffline="",const TerminateMethod method = terminate_server_active);

	virtual bool isvalid(){	return true;};
	virtual time_t valid_timeout();
	virtual time_t check_signal_interval();
	virtual time_t check_signal_waittime();

	bool sendcmd(void* pbuf, unsigned int nsize, int zliblevel = Z_DEFAULT_COMPRESSION );

	template< class cmdT> bool sendcmdex(cmdT& cmd, int zliblevel = Z_DEFAULT_COMPRESSION ){
		return sendcmd(&cmd, sizeof(cmd),zliblevel);
	}

	bool addcmd(void* pbuf, unsigned int nsize, int zliblevel = Z_DEFAULT_COMPRESSION );

	template< class cmdT> bool addcmdex(cmdT& cmd, int zliblevel = Z_DEFAULT_COMPRESSION  ){
		return addcmd(&cmd, sizeof(cmd),zliblevel);
	}

	bool postcmds();
};
//------------------------------------------------------------------------
class CClientConnecter:public CLD_Connecter
{
public:
	CClientConnecter();
	~CClientConnecter();
	virtual void OnError(CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg);
	virtual void OnConnect();
	virtual void OnDisconnect();

	bool setserver(const char* ip,int port,bool autoreconn=false);

	bool sendcmd(void* pbuf, unsigned int nsize, int zliblevel = Z_DEFAULT_COMPRESSION,DWORD dwTimeWait=0 );

	template< class cmdT> bool sendcmdex(cmdT& cmd, int zliblevel = Z_DEFAULT_COMPRESSION,DWORD dwTimeWait=0 ){
		return sendcmd(&cmd, sizeof(cmd),zliblevel,dwTimeWait);
	}

	virtual void run();
	virtual bool msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){ return true; }
	virtual void pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	virtual bool packetCheck(stBasePacket* ppacket,bool isfullpacket);

	bool isTerminate() const{return terminate_no != terminate;}
	virtual void Terminate(const char* ffline="",const TerminateMethod method = terminate_server_active);

	virtual bool isvalid(){return true;};
	virtual time_t valid_timeout();
	virtual time_t check_signal_interval();
	virtual time_t check_signal_waittime();
protected:
	unsigned int __stdcall thrun(CLD_ThreadBase* pthread,void* param);

	char m_szIp[32];
	int m_nport;
	CEncrypt* m_penc;
	CSafeMsgQueue m_msg;
	time_t m_conntime;
	unsigned int _de_size;						
	unsigned int _rcv_size_last;				
	time_t m_checkconntime;
	BYTE m_checksignalstate;
	bool m_autoreconn;
	bool m_boinitconn;
	CLD_LoopBuf m_recvbuf;
	CLD_ThreadBase* m_runthread;
	TerminateMethod terminate;
	DWORD m_dwMaxRecvPacketLen;
}; 
//------------------------------------------------------------------------