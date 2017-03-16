/**
*	created:		2013-4-7   23:46
*	filename: 		FKSubsvrSession
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKSubsvrSession.h"
#include "FKCheckNameSvr.h"
//------------------------------------------------------------------------
CSubSvrSession::CSubSvrSession(CLD_IocpBaseAccepter* Owner, SOCKET s):CLoopbufIocpClientSocketTask(Owner,s){
	FUNCTION_BEGIN;
	m_bovalid = false;
}
//------------------------------------------------------------------------
bool CSubSvrSession::isvalid(){
	return m_bovalid;
}
//------------------------------------------------------------------------
time_t CSubSvrSession::valid_timeout(){
	return 30;
}
//------------------------------------------------------------------------
bool CSubSvrSession::msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam)
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
bool CSubSvrSession::OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	FUNCTION_BEGIN;
	CCheckNameService * checknamesvr=CCheckNameService::instance();
	if (pcmd->dwCheckCode == _PRE_CHECK_CODE_ && (pcmd->svr_type==_GAMESVR_TYPE || pcmd->svr_type==_DBSVR_TYPE || pcmd->svr_type==_LOGINSVR_TYPE || pcmd->svr_type==_SUPERGAMESVR_TYPE) && ncmdlen>=sizeof(stSvr2SvrLoginCmd)) 
	{
		do{
			bool boAllowServer = false; 
			do{
				AILOCKT(checknamesvr->cfg_lock);
				svrinfomapiter it=checknamesvr->m_svrlistmap.find(pcmd->svr_marking);
				if(it != checknamesvr->m_svrlistmap.end()){
					boAllowServer=true;
					m_svrinfo=it->second;
				}
			}while (false);

			if (boAllowServer) 
			{
				bool bofindthis=false;
				svr_id_type_value = pcmd->svr_id_type_value; 
				m_svrinfo.wgame_type=pcmd->wgame_type;
				m_svrinfo.wzoneid=pcmd->wzoneid;
				strcpy_s(m_svrinfo.szZoneName,sizeof(m_svrinfo.szZoneName)-1,pcmd->szZoneName);
				strcpy_s(m_szZoneName,sizeof(m_szZoneName)-1,pcmd->szZoneName);

				do{
					AILOCKT(*GetAccepter()->GetClientList());

					DEF1_FUNCTOR_BEGIN(find_id, CLD_ClientSocket* , bool)
					{
						if ( m_this != _p1 && (m_this->m_svrinfo.svr_marking == ((CSubSvrSession *) (_p1))->m_svrinfo.svr_marking) ){
							return true;
						}else if (m_this==_p1 && ((CSubSvrSession *) (_p1))->m_bovalid){
							m_bofindthis=true;
						}
						return false;
					}

					CSubSvrSession *	m_this;
					bool m_bofindthis;
					DEF_FUNCTOR_END;

					find_id::_IF find_byidtype;
					find_byidtype.m_this=this;
					find_byidtype.m_bofindthis=false;
					CLD_Accepter::iterator it = find_if(GetAccepter()->begin(), GetAccepter()->end(), find_byidtype);
					if (it != GetAccepter()->end()) 
					{
						g_logger.error("发现重复的服务器连接 id=%d  type=%d", pcmd->svr_id, pcmd->svr_type);
						svr_id_type_value = 0;
						Terminate();
						return true;
					}

					bofindthis = find_byidtype.m_bofindthis;
				}while(false);

				if (!bofindthis)
				{
					time_t difftime=time(NULL)-pcmd->m_now;
					if ( difftime>(60*15) )
					{
						g_logger.forceLog(zLogger::zERROR ,"服务器 %d(%d) [%s:%d] 系统时间异常( %s <> %s ),请校正所有服务器时间后重新启动服务!",
							svr_id,svr_type,GetRemoteAddress(),GetRemotePort(),timetostr(time(NULL)) ,timetostr(pcmd->m_now));
						Terminate();
						return true;
					}else if(difftime>10)
					{
						g_logger.forceLog(zLogger::zDEBUG ,"服务器 %d(%d) [%s:%d] 系统时间差为 %d 秒( %s <> %s )!",svr_id,svr_type,GetRemoteAddress(),GetRemotePort(),difftime,timetostr(time(NULL)) ,timetostr(pcmd->m_now));
					}

					checknamesvr->m_Svr2SvrLoginCmd.m_now=time(NULL);
					sendcmd((char *) &checknamesvr->m_Svr2SvrLoginCmd,sizeof(checknamesvr->m_Svr2SvrLoginCmd));

					do{
						CCheckNameService* checknamesvr=CCheckNameService::instance();
						AILOCKT(checknamesvr->cfg_lock);
					}while(false);

					g_logger.debug("服务器 %d(%d)[%d:%d:%s] 连接校验成功!",svr_id,svr_type,pcmd->wgame_type,pcmd->wzoneid,pcmd->szZoneName );
				}
				m_bovalid = true;
			} else 
			{
				Terminate();
			}
		}while(false);

	} else 
	{
		Terminate();
	}
	return true;
}
//------------------------------------------------------------------------