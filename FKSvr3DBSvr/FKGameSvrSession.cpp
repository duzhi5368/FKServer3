/**
*	created:		2013-4-9   15:48
*	filename: 		FKGameSvrSession
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKGameSvrSession.h"
#include "FKDBSvr.h"
//------------------------------------------------------------------------
CGameSvrSession::CGameSvrSession(CLD_IocpBaseAccepter* Owner, SOCKET s):CLoopbufIocpClientSocketTask(Owner,s){
	FUNCTION_BEGIN;
	m_bovalid = false;
	m_nPlayerCount=0;
	m_dwMaxRecvPacketLen=(_MAX_SEND_PACKET_SIZE_-1024);
}
//------------------------------------------------------------------------
bool CGameSvrSession::isvalid(){
	return m_bovalid;
}
//------------------------------------------------------------------------
time_t CGameSvrSession::valid_timeout(){
	return 30;
}
//------------------------------------------------------------------------
bool ipinfousercount_cmp(const stIpInfoState& m1,const stIpInfoState& m2){
	return m1.ncount<m2.ncount;	//升序
}
//------------------------------------------------------------------------
bool CGameSvrSession::GetALoginIpInfo(in_addr& ip,WORD& port,BYTE ip_type){
	DBService* login=DBService::instance();
	do{
		std::vector<stIpInfoState>& gamesvrips=login->m_gamesvrips ;
		AILOCKT(login->m_gamesvrsession);

		if (gamesvrips.size()>=2){
			sort(gamesvrips.begin(),gamesvrips.end(),ipinfousercount_cmp);
		}
		int nmin=-1;
		int ntype_min=-1;
		for (size_t i=0;i<gamesvrips.size();i++){
			if ( gamesvrips[i].state==0 && checkGate(&gamesvrips[i]) ){
				if (nmin<0){ nmin=i;};
				if (gamesvrips[i].type==ip_type){
					ntype_min=i;
					break;
				}
			}
		}
		if (ntype_min<0){ ntype_min=nmin;	}
		if (ntype_min>=0 && (size_t)ntype_min<gamesvrips.size()){
			ip=gamesvrips[ntype_min].ip;
			port=gamesvrips[ntype_min].port;
			gamesvrips[ntype_min].ncount++;
			return true;
		}
	} while (false);
	return false;
}
//------------------------------------------------------------------------
bool CGameSvrSession::msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam)
{
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
bool CGameSvrSession::OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	FUNCTION_BEGIN;
	DBService * dbsvr=DBService::instance();
	if (pcmd->dwCheckCode == _PRE_CHECK_CODE_ && pcmd->svr_type==_GAMESVR_TYPE && ncmdlen>=sizeof(stSvr2SvrLoginCmd)) {
		do{
			bool boAllowServer = false; 
			do{
				AILOCKT(dbsvr->cfg_lock);
				svrinfomapiter it=dbsvr->m_svrlistmap.find(pcmd->svr_marking);
				if(it != dbsvr->m_svrlistmap.end()){
					boAllowServer=true;
					m_ServerInfo=it->second;
				}
			}while (false);
			if (boAllowServer) {
				bool bofindthis=false;
				{
					svr_id_type_value = pcmd->svr_id_type_value; 
					m_ServerInfo.wgame_type=pcmd->wgame_type;
					m_ServerInfo.wzoneid=pcmd->wzoneid;
					strcpy_s(m_ServerInfo.szZoneName,sizeof(m_ServerInfo.szZoneName)-1,pcmd->szZoneName);
					strcpy_s(m_szZoneName,sizeof(m_szZoneName)-1,pcmd->szZoneName);
					do{
						AILOCKT(*GetAccepter()->GetClientList());
						DEF1_FUNCTOR_BEGIN(find_id, CLD_ClientSocket* , bool)
						{
							if (m_this != _p1 && (m_this->svr_id_type_value == ((CGameSvrSession *) (_p1))->svr_id_type_value)){
								return true;
							}else if (m_this==_p1 && ((CGameSvrSession *) (_p1))->m_bovalid){
								m_bofindthis=true;
							}
							return false;
						}
						CGameSvrSession *	m_this;
						bool m_bofindthis;
						DEF_FUNCTOR_END;

						find_id::_IF find_byidtype;
						find_byidtype.m_this=this;
						find_byidtype.m_bofindthis=false;
						CLD_Accepter::iterator it = find_if(GetAccepter()->begin(), GetAccepter()->end(), find_byidtype);
						if (it != GetAccepter()->end()) {
							g_logger.error("发现重复的服务器连接 id=%d  type=%d", pcmd->svr_id, pcmd->svr_type);
							svr_id_type_value = 0;
							Terminate();
							return true;
						}
						bofindthis=find_byidtype.m_bofindthis;
					}while(false);
				}
				if (!bofindthis){
					time_t difftime=time(NULL)-pcmd->m_now;
					if ( difftime>(60*15) ){
						g_logger.forceLog(zLogger::zERROR ,"服务器 %d(%d) [%s:%d] 系统时间异常( %s <> %s ),请校正所有服务器时间后重新启动服务!",
							svr_id,svr_type,GetRemoteAddress(),GetRemotePort(),timetostr(time(NULL)) ,timetostr(pcmd->m_now));
						Terminate();
						return true;
					}else if(difftime>10){
						g_logger.forceLog(zLogger::zDEBUG ,"服务器 %d(%d) [%s:%d] 系统时间差为 %d 秒( %s <> %s )!",svr_id,svr_type,GetRemoteAddress(),GetRemotePort(),difftime,timetostr(time(NULL)) ,timetostr(pcmd->m_now));
					}
					dbsvr->m_Svr2SvrLoginCmd.m_now=time(NULL);
					sendcmd((char *) &dbsvr->m_Svr2SvrLoginCmd,sizeof(dbsvr->m_Svr2SvrLoginCmd));
					do{
						DBService* dbsvr=DBService::instance();
						AILOCKT(dbsvr->cfg_lock);
					}while(false);
					g_logger.debug("游戏服务器 %d(%d)[%d:%d:%s] 连接校验成功!",svr_id,svr_type,pcmd->wgame_type,pcmd->wzoneid,pcmd->szZoneName );
				}
				m_bovalid = true;
			} else {
				Terminate();
			}
		}while(false);
	} else {
		Terminate();
	}
	return true;
}
//------------------------------------------------------------------------