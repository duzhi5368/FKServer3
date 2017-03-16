/**
*	created:		2013-4-9   17:27
*	filename: 		FKGameGatewayClientSesion
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKGameGatewayClientSession.h"
//------------------------------------------------------------------------
CIntLock stGateWayConnecterUser::s_socket_idx_lock;
//------------------------------------------------------------------------
long stGateWayConnecterUser::s_socket_idx=_random(1000)+100;
//------------------------------------------------------------------------
stGateWayConnecterUser::stGateWayConnecterUser(){
	pGateWay=NULL;
	socket_idx=INVALID_SOCKET;
	socket_handle=INVALID_SOCKET;
	wsvridx=-1;
	wgindex=-1;

	do {
		AILOCKT(s_socket_idx_lock);
		s_socket_idx++;
		if (s_socket_idx>0x7ffffff0){ s_socket_idx=_random(1000)+100; }
		socket_idx=s_socket_idx;
	} while (false);
}
//------------------------------------------------------------------------
stGateWayConnecterUser::~stGateWayConnecterUser()
{
	notifySvrRemoveUser();
}
//------------------------------------------------------------------------
extern _TH_VAR_ARRAY(char,tls_gatewayproxydata_charbuffer,_MAX_SEND_PACKET_SIZE_+1);
//------------------------------------------------------------------------
bool stGateWayConnecterUser::SendProxyCmd(void* pbuf, unsigned int nsize, bool bozlib,int zliblevel )
{
	if (nsize>0 && pbuf)
	{
		char* cmdbuffer=(char*)_TH_VAR_PTR(tls_gatewayproxydata_charbuffer);
		stProxyDataCmd* pproxydatacmd=(stProxyDataCmd*)&cmdbuffer[0];
		constructInPlace(pproxydatacmd);
		pproxydatacmd->sidx=this->socket_idx;
		pproxydatacmd->wsvridx=this->wsvridx;
		pproxydatacmd->wgindex=this->wgindex;

		pproxydatacmd->pluscmd.push_back((const char *)pbuf,nsize);

		AILOCKT(*this);
		if (pGateWay && wgindex>=0 && INVALID_SOCKET!=socket_idx)
		{
			return pGateWay->sendcmd(pproxydatacmd,sizeof(*pproxydatacmd)+pproxydatacmd->pluscmd.getarraysize(),zliblevel);
		}
	}
	return false;
}
//------------------------------------------------------------------------
bool stGateWayConnecterUser::fullGateWayConnecterUser(CGateWayConnecter* pgate,SOCKET s)
{
	stGateWayConnecterUser* pUser=this;
	CGateWayConnecter* ptmpgateway=NULL;
	do {
		AILOCKT(*this);
		ptmpgateway=pGateWay;
	}while(false);
	int wg=-1;
	if (ptmpgateway)
	{
		ptmpgateway->m_userlist.remove(wgindex,socket_idx);
		pUser->wgindex=wg;
	}

	do{
		if (pgate)
		{
			if (pgate->IsConnected()){ wg=pgate->m_userlist.add(socket_idx,this); }
		}
	}while(false);

	do {
		AILOCKT(*this);
		pUser->socket_handle=s;
		pUser->pGateWay=pgate;
		pUser->wgindex=wg;
		pUser->wsvridx=-1;
	}while(false);
	return true;
}
//------------------------------------------------------------------------
bool stGateWayConnecterUser::initGateWayConnecterUser()
{
	stGateWayConnecterUser* pUser=this;
	CGateWayConnecter* ptmpgateway=NULL;
	do 
	{
		AILOCKT(*this);
		ptmpgateway=pGateWay;
	}while(false);

	if (ptmpgateway)
	{
		ptmpgateway->m_userlist.remove(wgindex,socket_idx);
	}
	do 
	{
		AILOCKT(*this);
		pUser->socket_idx=INVALID_SOCKET;
		pUser->socket_handle=INVALID_SOCKET;
		pUser->pGateWay=NULL;
		pUser->wsvridx=(WORD)-1;
		pUser->wgindex=(WORD)-1;
	}while(false);

	return true;
}
//------------------------------------------------------------------------
bool stGateWayConnecterUser::notifySvrAddUser(CGateWayConnecter* pgate,SOCKET	s,__int64	addparam,void* pcmd, unsigned int ncmdlen)
{
	bool addok=false;
	stGateWayConnecterUser* pUser=this;
	if (pUser && s!=INVALID_SOCKET && pgate && socket_idx!=INVALID_SOCKET )
	{
		do{
			if (pgate->IsConnected())
			{ 
				pUser->fullGateWayConnecterUser(pgate,s);
			}
		}while (false);

		if ( pUser->wgindex>=0)
		{
			addok= pgate->SendOpenUser(pUser,addparam,pcmd,ncmdlen);
		}
	}
	if (!addok){ notifySvrRemoveUser(); }
	return addok;
}
//------------------------------------------------------------------------
bool stGateWayConnecterUser::notifySvrRemoveUser()
{
	bool boret=false;
	CGateWayConnecter* ptmpgateway=NULL;
	do 
	{
		AILOCKT(*this);
		if (pGateWay && wsvridx>=0 && INVALID_SOCKET!=socket_idx)
		{
			ptmpgateway=pGateWay;
		}
	}while(false);

	if (ptmpgateway)
	{
		boret=ptmpgateway->SendUserClose(socket_idx,wgindex,wsvridx,NULL,0);
	}
	initGateWayConnecterUser();
	return boret;
}
//------------------------------------------------------------------------
CGateWayConnecter::~CGateWayConnecter()
{
	m_userlist.removeall();
}
//------------------------------------------------------------------------
bool CGateWayConnecter::SocketCanRecycle()
{
	if ( !m_CanRecycle || m_userlist.size()>0 ){return false;}
	return true;
}
//------------------------------------------------------------------------
void CGateWayConnecter::run()
{
	__super::run();
}
//------------------------------------------------------------------------
bool CGateWayConnecter::msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam)
{
	switch (pcmd->value)
	{
	case stGameSvrBuildMsg::_value:
		{
			return this->OnstBuildedProxyDataCmd( ( stGameSvrBuildMsg* ) pcmd, ncmdlen, bufferparam ) ;
		}
	case stProxyDataCmd::_value:
		{
			return this->OnstProxyDataCmd((stProxyDataCmd*)pcmd,ncmdlen,bufferparam);
		}
		break;
	case stOpenRetCmd::_value:
		{
			return this->OnstOpenRetCmd((stOpenRetCmd*)pcmd,ncmdlen,bufferparam);
		}
		break;
	case stKickCmd::_value:
		{
			return this->OnstKickCmd((stKickCmd*)pcmd,ncmdlen,bufferparam);
		}
		break;
	case stProxySvrCmd::_value:
		{
			return this->OnstProxySvrCmd((stProxySvrCmd*)pcmd,ncmdlen,bufferparam);
		}
		break;
	case stProxyData2UsersCmd::_value:
		{
			return this->OnstProxyData2UsersCmd((stProxyData2UsersCmd*)pcmd,ncmdlen,bufferparam);
		}
		break;
	}
	return true;
}
//------------------------------------------------------------------------
bool CGateWayConnecter::SendOpenUser(stGateWayConnecterUser* puser,__int64	addparam,void* pcmd, unsigned int ncmdlen)
{
	char* cmdbuffer=(char*)_TH_VAR_PTR(tls_gatewayproxydata_charbuffer);
	stOpenCmd* popencmd=(stOpenCmd*)&cmdbuffer[0];
	constructInPlace(popencmd);
	popencmd->sidx=puser->socket_idx;
	popencmd->socket_handle=puser->socket_handle;
	popencmd->addparam=addparam;

	char buffer[sizeof(sockaddr_in)+128]={0};
	sockaddr_in* tmp=(sockaddr_in*)&buffer;
	CLD_Socket::GetRemoteAddr(puser->socket_handle,tmp);
	popencmd->clientip.s_addr=tmp->sin_addr.s_addr;
	popencmd->wclientport=CLD_Socket::GetRemotePort(puser->socket_handle);

	CLD_Socket::GetLocalAddr(puser->socket_handle,tmp);
	popencmd->gateip.s_addr=tmp->sin_addr.s_addr;
	popencmd->wgateport=CLD_Socket::GetLocalPort(puser->socket_handle);

	popencmd->wgindex=puser->wgindex;
	if (ncmdlen>0 && pcmd)
	{
		popencmd->pluscmd.push_back((const char *)pcmd,ncmdlen);
	}
	g_logger.debug("%s:%d->网关会话(%d-%d-%d)通知服务器建立会话!",CLD_Socket::GetRemoteAddress(puser->socket_handle) ,CLD_Socket::GetRemotePort(puser->socket_handle), popencmd->wgindex,popencmd->sidx,popencmd->socket_handle);
	return sendcmd(popencmd,sizeof(*popencmd)+popencmd->pluscmd.getarraysize());
}
//------------------------------------------------------------------------
bool CGateWayConnecter::SendUserClose(long sidx,int wgindex,int wsvridx,void* pcmd, unsigned int ncmdlen)
{
	m_userlist.remove(wgindex,sidx);
	char* cmdbuffer=(char*)_TH_VAR_PTR(tls_gatewayproxydata_charbuffer);
	stCloseCmd* pclosecmd=(stCloseCmd*)&cmdbuffer[0];
	constructInPlace(pclosecmd);
	pclosecmd->sidx=sidx;
	pclosecmd->wsvridx=wsvridx;
	pclosecmd->wgindex=wgindex;
	if (ncmdlen>0 && pcmd)
	{
		pclosecmd->pluscmd.push_back((const char *)pcmd,ncmdlen);
	}
	g_logger.debug("网关会话(%d-%d)通知服务器关闭会话(%d)!",pclosecmd->wgindex,pclosecmd->sidx,pclosecmd->wsvridx);
	return sendcmd(pclosecmd,sizeof(*pclosecmd)+pclosecmd->pluscmd.getarraysize());
}
//------------------------------------------------------------------------
bool CGateWayConnecter::SendSvrCmd(void* pcmd, unsigned int ncmdlen, int zliblevel )
{
	if (ncmdlen>0 && pcmd)
	{
		char* cmdbuffer=(char*)_TH_VAR_PTR(tls_gatewayproxydata_charbuffer);
		stProxySvrCmd* pproxysvrcmd=(stProxySvrCmd*)&cmdbuffer[0];
		constructInPlace(pproxysvrcmd);

		pproxysvrcmd->pluscmd.push_back((const char *)pcmd,ncmdlen);
		return sendcmd(pproxysvrcmd,sizeof(*pproxysvrcmd)+pproxysvrcmd->pluscmd.getarraysize(),zliblevel);
	}
	return false;
}
//------------------------------------------------------------------------
bool CGateWayConnecter::OnstOpenRetCmd(stOpenRetCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam)
{
	if (pcmd->numCheckSum==GW_PACKCHECKSUM)
	{
		stGateWayConnecterUser* tmp=NULL;
		g_logger.debug("服务器会话(%d)关联网关会话(%d-%d)成功!",pcmd->wsvridx,pcmd->wgindex,pcmd->sidx);
		if (m_userlist.get(pcmd->wgindex,pcmd->sidx,tmp))
		{
			if (tmp)
			{		
				stBaseCmd* pluscmd=(stBaseCmd*)pcmd->pluscmd.getptr();
				int npluscmdlen=pcmd->pluscmd.size;

				tmp->wsvridx=pcmd->wsvridx;
				if (npluscmdlen>=sizeof(stBaseCmd))
				{
					bool boret= tmp->svrmsgParse(pluscmd,npluscmdlen,bufferparam);
					return boret;
				}
			}
		}
		if (!tmp){ SendUserClose(pcmd->sidx,-1,pcmd->wsvridx,NULL,0);	}
	}
	return true;
}
//------------------------------------------------------------------------
bool CGateWayConnecter::OnstKickCmd(stKickCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam)
{
	if (pcmd->numCheckSum==GW_PACKCHECKSUM)
	{
		stGateWayConnecterUser* tmp=NULL;
		g_logger.debug("服务器会话(%d)通知关闭网关会话(%d-%d)!",pcmd->wsvridx,pcmd->wgindex,pcmd->sidx);
		if (m_userlist.get(pcmd->wgindex,pcmd->sidx,tmp))
		{
			if( !tmp )
				return true;

			tmp->OnsvrKickUser();
			tmp->initGateWayConnecterUser();
			SendUserClose(pcmd->sidx,pcmd->wgindex,pcmd->wsvridx,NULL,0);
			return true;
		}
	}
	return true;
}
//------------------------------------------------------------------------
bool CGateWayConnecter::OnstProxyDataCmd(stProxyDataCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam)
{
	if (pcmd->numCheckSum==GW_PACKCHECKSUM)
	{
		stGateWayConnecterUser* tmp=NULL;
		if (m_userlist.get(pcmd->wgindex,pcmd->sidx,tmp))
		{
			if (tmp)
			{
				stBaseCmd* pluscmd=(stBaseCmd*)pcmd->pluscmd.getptr();
				int npluscmdlen=pcmd->pluscmd.size;
				if (npluscmdlen>=sizeof(stBaseCmd))
				{
					bool boret= tmp->svrmsgParse(pluscmd,npluscmdlen,bufferparam);
					return boret;
				}
			}
		}
		if (!tmp){ SendUserClose(pcmd->sidx,-1,pcmd->wsvridx,NULL,0);	}
	}
	return true;
}
//------------------------------------------------------------------------
bool CGateWayConnecter::OnstProxyData2UsersCmd(stProxyData2UsersCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam)
{
	if (pcmd->numCheckSum==GW_PACKCHECKSUM)
	{
		stGateWayConnecterUser* tmp=NULL;
		g_logger.debug("服务器广播信息到多个网关会话(%d)!",pcmd->broadcastusers.size);
		stProxyData2UsersCmd::stProxyUserIndex* puserindex=NULL;
		stBaseCmd* pluscmd=(stBaseCmd*)pcmd->getpluscmd();
		int npluscmdlen=pcmd->ncmdlen;
		if (npluscmdlen>=sizeof(stBaseCmd))
		{
			for (int i=0;i<pcmd->broadcastusers.size;i++)
			{
				puserindex=&pcmd->broadcastusers[i];
				tmp=NULL;
				if (m_userlist.get(puserindex->wgindex,puserindex->sidx,tmp))
				{
					if (tmp)
					{		
						bool boret= tmp->svrmsgParse(pluscmd,npluscmdlen,bufferparam);
						return boret;
					}
				}
				if (!tmp){ SendUserClose(puserindex->sidx,-1,puserindex->wsvridx,NULL,0);	}
			}
		}
	}
	return true;
}
//------------------------------------------------------------------------
bool CGateWayConnecter::OnstProxySvrCmd(stProxySvrCmd* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam)
{
	if (pcmd->numCheckSum==GW_PACKCHECKSUM)
	{
		stBaseCmd* pluscmd=(stBaseCmd*)pcmd->pluscmd.getptr();
		int npluscmdlen=pcmd->pluscmd.size;
		if (npluscmdlen>=sizeof(stBaseCmd))
		{
			return svr2GatemsgParse(pluscmd,npluscmdlen,bufferparam);
		}
	}
	return true;
}
//------------------------------------------------------------------------
bool CGateWayConnecter::OnstBuildedProxyDataCmd( stGameSvrBuildMsg* pcmd, unsigned int ncmdlen, stQueueMsgParam* bufferparam )
{
	if( pcmd->numCheckSum == GW_PACKCHECKSUM )
	{
		const char * _DataBuf = pcmd->pluscmd.getptr();
		int npluscmdlen=pcmd->pluscmd.size;
		if (npluscmdlen>=sizeof(stProxyDataCmd))
		{
			return svr2GateBuildedMsgParse(_DataBuf,npluscmdlen,bufferparam);
		}
	}
	return true;
}
//------------------------------------------------------------------------
bool CGateWayConnecter::svr2GateBuildedMsgParse( const char * pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam )
{
	if( !pcmd || ncmdlen < sizeof( stProxyDataCmd ) )
		return false;

	unsigned int PtrIndex = 0;
	while( PtrIndex < ncmdlen )
	{
		stProxyDataCmd* TheProxyDataCmd = ( stProxyDataCmd* )( pcmd + PtrIndex );
		OnstProxyDataCmd( TheProxyDataCmd, TheProxyDataCmd->pluscmd.size, bufferparam );
		PtrIndex += sizeof( stProxyDataCmd ) + TheProxyDataCmd->pluscmd.getarraysize();
	}
	return true;
}
//------------------------------------------------------------------------