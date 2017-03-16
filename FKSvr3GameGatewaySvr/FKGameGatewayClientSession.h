/**
*	created:		2013-4-9   17:15
*	filename: 		FKGameGatewayClientSession
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "FKGameGatewayBase.h"
//------------------------------------------------------------------------
class CGateWayConnecter;
//------------------------------------------------------------------------
class stGateWayConnecterUser:public CIntLock
{
public:
	CGateWayConnecter* pGateWay;
private:
	friend class CGateWayConnecter;
	SOCKET	socket_handle;
	long	socket_idx;
	static CIntLock s_socket_idx_lock;
	static long s_socket_idx;
	short int	wgindex;		
	short int   wsvridx;
protected:
	bool fullGateWayConnecterUser(CGateWayConnecter* pgate,SOCKET s);
	bool initGateWayConnecterUser();
public:

	const SOCKET ClientSocketHandle(){ return socket_handle; }
	const long ClientSocketIdx(){ return socket_idx; }
	const WORD Gateidx(){ return wgindex; }
	const WORD Svridx(){ return wsvridx; }

	stGateWayConnecterUser();
	virtual ~stGateWayConnecterUser();

	bool isSocketClose(){ AILOCKT(*this); return (socket_idx==INVALID_SOCKET && pGateWay==NULL); }
	virtual bool svrmsgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam)=0;
	virtual bool OnsvrKickUser()=0;


	virtual bool notifySvrAddUser(CGateWayConnecter* pgate,SOCKET	s,__int64	addparam,void* pcmd, unsigned int ncmdlen);
	virtual bool notifySvrRemoveUser();

	bool SendProxyCmd(void* pbuf, unsigned int nsize, bool bozlib=true,int zliblevel = Z_DEFAULT_COMPRESSION );

	template< class cmdT> bool SendProxyCmdEx(cmdT& cmd, bool bozlib=true,int zliblevel = Z_DEFAULT_COMPRESSION ){
		return SendProxyCmd(&cmd, sizeof(cmd),bozlib,zliblevel);
	}
};
//------------------------------------------------------------------------
class CGateWayConnecter : public CLoopbufIocpConnecterTask 
{
public:
	CGateWayConnecter(CLD_IocpHandle* Owner,int nLoopbufSize=DEF_LOOPBUF_SIZE):CLoopbufIocpConnecterTask(Owner,nLoopbufSize) {};
	virtual ~CGateWayConnecter();

	virtual bool SocketCanRecycle();
	virtual bool svr2GatemsgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam)=0;
	virtual bool msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);

	bool SendSvrCmd(void* pcmd, unsigned int ncmdlen, int zliblevel = Z_DEFAULT_COMPRESSION );

	bool OnstOpenRetCmd(stOpenRetCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* param);
	bool OnstKickCmd(stKickCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* param);
	bool OnstProxyDataCmd(stProxyDataCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* param);
	bool OnstProxySvrCmd(stProxySvrCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam);
	bool OnstProxyData2UsersCmd(stProxyData2UsersCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam);
	bool OnstBuildedProxyDataCmd(stGameSvrBuildMsg* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam);
	bool svr2GateBuildedMsgParse(const char * pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);

	virtual void run();
protected:
	friend class stGateWayConnecterUser;
	CGatewayConnManage m_userlist;

	bool SendOpenUser(stGateWayConnecterUser* puser,__int64	addparam,void* pcmd, unsigned int ncmdlen);
	bool SendUserClose(long s,int wgindex,int wsvridx,void* pcmd, unsigned int ncmdlen);
};
//------------------------------------------------------------------------