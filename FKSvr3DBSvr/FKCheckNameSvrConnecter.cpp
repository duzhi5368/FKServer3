/**
*	created:		2013-4-8   16:34
*	filename: 		FKCheckNameSvrConnecter
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKCheckNameSvrConnecter.h"
#include "FKDBSvr.h"
//------------------------------------------------------------------------
void CCheckNameSvrConnecter::OnIocpConnect(){
	__super::OnIocpConnect();
	DBService* service=DBService::instance();

	service->m_Svr2SvrLoginCmd.m_now=time(NULL);
	sendcmd(&service->m_Svr2SvrLoginCmd,sizeof(service->m_Svr2SvrLoginCmd));
}
//------------------------------------------------------------------------
void CCheckNameSvrConnecter::OnDisconnect(){
	__super::OnDisconnect();

	DBService* service=DBService::instance();
	do {
		AILOCKT(service->m_checkcreateplayernametransmanage);
		DBService::zBaseCmdTransidManage::iterator it;
		for (it=service->m_checkcreateplayernametransmanage.begin();it!=service->m_checkcreateplayernametransmanage.end();it++){
			FreePacketBuffer(it->second.pmsgbuffer);
		}
		service->m_checkcreateplayernametransmanage.clear();
	} while (false);

	m_bovalid = false;
}
//------------------------------------------------------------------------
bool CCheckNameSvrConnecter::isvalid(){
	return m_bovalid;
}
//------------------------------------------------------------------------
time_t CCheckNameSvrConnecter::valid_timeout(){
	return 30;
}
//------------------------------------------------------------------------
bool CCheckNameSvrConnecter::msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
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
bool CCheckNameSvrConnecter::OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
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
		m_bovalid = true;
		g_logger.debug("名字重复性检查服务器 %d(%d) 登陆校验成功!",svr_id,svr_type);
	}
	return true;
}
//------------------------------------------------------------------------