/**
*	created:		2013-4-9   17:34
*	filename: 		FKGameGatewayConneter
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKGameGatewayConneter.h"
#include "FKGameGatewaySvr.h"
//------------------------------------------------------------------------
IMP_OP_NEW(CGameGateWayUser);
//------------------------------------------------------------------------
extern bool s_usePerformance;
//------------------------------------------------------------------------
bool CGameGateWayUser::svrmsgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	extern bool g_boshowproxylog;
	if (g_boshowproxylog){
		char szbuf[512]={0};
		AbsMem2Hex((char *)pcmd,ncmdlen,32,4,4,szbuf,sizeof(szbuf)-1);
		g_logger.debug("服务器会话(%d)转发(%d:%d ->%d )到网关会话(%d-%d) data: %s",Svridx(),pcmd->cmd,pcmd->subcmd,ncmdlen,Gateidx(),ClientSocketIdx(),szbuf);
	}
	if (!m_Owner){	return true; }
	switch (pcmd->value)
	{
	default:
		{
			if (pcmd->value==stCheckSpeedCmd::_value){
				stCheckSpeedCmd* pdstcmd=(stCheckSpeedCmd*)pcmd;
				pdstcmd->setLog(GameGateService::getMe().m_Svr2SvrLoginCmd.svr_id_type_value);
				g_logger.forceLog(zLogger::zFORCE, "(%d)测速信号_svr %d:%s : r:%d(l:%d)",pdstcmd->dwProxyCount, pdstcmd->dwCheckIndex,pdstcmd->szCheckStr,pdstcmd->dwLocalTick,::GetTickCount() );
			}

			if( s_usePerformance )
			{
				m_Owner->addcmd( pcmd, ncmdlen );
			}
			else
			{
				m_Owner->sendcmd(pcmd,ncmdlen);
			}
		}
		break;
	}
	return true;
}
//------------------------------------------------------------------------
bool CGameGateWayUser::OnsvrKickUser(){
	AILOCKT(GameGateService::instance()->m_gatewayconnter);
	if (m_Owner){ m_Owner->Terminate(__FF_LINE__);	}
	if (m_szPlayerNmae[0]!=0){
		g_logger.debug("玩家 %s 角色 %s 被服务器踢下线",m_szAccount,m_szPlayerNmae);
	}
	return true;
}
//------------------------------------------------------------------------
void CGameGateWayConnecter::pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	switch (pcmd->value)
	{
	case stProxyDataCmd::_value:
		{
			stBaseCmd* pluscmd=(stBaseCmd*)((stProxyDataCmd*)pcmd)->pluscmd.getptr();
			int npluscmdlen=((stProxyDataCmd*)pcmd)->pluscmd.size;
			if (npluscmdlen>=sizeof(stBaseCmd)){
				if (pluscmd->value==stCheckSpeedCmd::_value){
					stCheckSpeedCmd* pdstcmd=(stCheckSpeedCmd*)pluscmd;
					pdstcmd->setLog(GameGateService::getMe().m_Svr2SvrLoginCmd.svr_id_type_value);
					g_logger.forceLog(zLogger::zFORCE, "(%d)测速信号_recv %d:%s : r:%d(l:%d)",pdstcmd->dwProxyCount,pdstcmd->dwCheckIndex,pdstcmd->szCheckStr,pdstcmd->dwLocalTick,::GetTickCount() );
				}	
			}
		}
	}	
	__super::pushMsgQueue(ppacket,pcmd, ncmdlen,bufferparam);
}
//------------------------------------------------------------------------
void CGameGateWayConnecter::OnIocpConnect(){
	__super::OnIocpConnect();

	GameGateService* gateway=GameGateService::instance();
	gateway->m_Svr2SvrLoginCmd.m_now=time(NULL);
	SendSvrCmd(&gateway->m_Svr2SvrLoginCmd,sizeof(gateway->m_Svr2SvrLoginCmd));
}
//------------------------------------------------------------------------
void CGameGateWayConnecter::OnDisconnect(){
	__super::OnDisconnect();
	do {
		std::vector< CGameGateWayUser* > g_removed(128);
		g_removed.clear();
		CGatewayConnManage::gwc_iter it;
		CGameGateWayUser* pUser=NULL;
		do {
			AILOCKT(m_userlist);
			for (it=m_userlist.begin();it!=m_userlist.end();it++){
				pUser=(CGameGateWayUser*)it->obj;
				if (pUser){
					g_removed.push_back(pUser);
				}
			}
		} while (false);
		if (g_removed.size()>0){
			for (int i=0;(size_t)i<g_removed.size();i++){
				pUser=g_removed[i];
				do {
					AILOCKT(GameGateService::getMe().m_gatewayconnter);
					if (pUser->m_Owner){
						pUser->m_Owner->Terminate(__FF_LINE__);
					}
				} while (false);
			}
		}
	} while (false);

	do {
		GameGateService* gateway=GameGateService::instance();
		AILOCKT(gateway->m_gatewayconnter);
		GameServerMapIDMap::iterator it,itnext;
		for (it=gateway->m_gatewayconnter_mapid.begin(),itnext=it;it!=gateway->m_gatewayconnter_mapid.end();it=itnext){
			itnext++;
			if (it->second.ownerconnecter==this){
				gateway->m_gatewayconnter_mapid.erase(it);
			}
		}
	} while (false);
	m_bovalid = false;
}
//------------------------------------------------------------------------
bool CGameGateWayConnecter::isvalid(){
	return m_bovalid;
}
//------------------------------------------------------------------------
time_t CGameGateWayConnecter::valid_timeout(){
	return 30;
}
//------------------------------------------------------------------------
bool CGameGateWayConnecter::svr2GatemsgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	switch (pcmd->value)
	{
	case stSvr2SvrLoginCmd::_value:
		{
			return OnstSvr2SvrLoginCmd((stSvr2SvrLoginCmd*)pcmd,ncmdlen,bufferparam);
		}
		break;
	}
	return true;
}
//------------------------------------------------------------------------
bool CGameGateWayConnecter::OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	FUNCTION_BEGIN;
	if (svr_id_type_value != pcmd->svr_id_type_value) {
		Terminate();
		g_logger.error("连接的服务器(%u:%u <> %u:%u)与配置文件记录不一致...", svr_id, svr_type, pcmd->svr_id, pcmd->svr_type);
		m_bovalid = false;
	} else {
		time_t difftime=time(NULL)-pcmd->m_now;
		if ( difftime>(60*15) ){
			g_logger.forceLog(zLogger::zERROR ,"服务器 %d(%d) [%s:%d] 系统时间异常( %s <> %s ),请校正所有服务器时间后重新启动服务!",
				svr_id,svr_type,GetRemoteAddress(),GetRemotePort(),timetostr(time(NULL)) ,timetostr(pcmd->m_now));
		}else if(difftime>10){
			g_logger.forceLog(zLogger::zDEBUG ,"服务器 %d(%d) [%s:%d] 系统时间差为 %d 秒( %s <> %s )!",svr_id,svr_type,GetRemoteAddress(),GetRemotePort(),difftime,timetostr(time(NULL)) ,timetostr(pcmd->m_now));
		}
		do {
			GameGateService* gateway=GameGateService::instance();
			gateway->RefStateCmd();
			AILOCKT(gateway->cfg_lock);
		} while (false);		
		m_bovalid = true;
		g_logger.debug("游戏服务器 %d(%d) 登陆校验成功!",svr_id,svr_type);
	}
	return true;
}
//------------------------------------------------------------------------