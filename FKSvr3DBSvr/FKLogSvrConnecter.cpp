/**
*	created:		2013-4-9   16:22
*	filename: 		FKLogSvrConnecter
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKLogSvrConnecter.h"
#include "FKDBSvr.h"
//------------------------------------------------------------------------
void CLogSvrConnecter::OnIocpConnect(){
	FUNCTION_BEGIN;
	__super::OnIocpConnect();
	DBService* service=DBService::instance();

	service->m_Svr2SvrLoginCmd.m_now=time(NULL);
	sendcmd(&service->m_Svr2SvrLoginCmd,sizeof(service->m_Svr2SvrLoginCmd));
}
//------------------------------------------------------------------------
void CLogSvrConnecter::OnDisconnect(){
	FUNCTION_BEGIN;
	__super::OnDisconnect();

	m_bovalid = false;
}
//------------------------------------------------------------------------
bool CLogSvrConnecter::isvalid(){
	FUNCTION_BEGIN;
	return m_bovalid;
}
//------------------------------------------------------------------------
time_t CLogSvrConnecter::valid_timeout(){
	FUNCTION_BEGIN;
	return 30;
}
//------------------------------------------------------------------------
bool CLogSvrConnecter::msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	FUNCTION_BEGIN;
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
bool CLogSvrConnecter::OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
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
		g_logger.debug("日志服务器 %d(%d) 登陆校验成功!",svr_id,svr_type);
	}
	return true;
}
//------------------------------------------------------------------------