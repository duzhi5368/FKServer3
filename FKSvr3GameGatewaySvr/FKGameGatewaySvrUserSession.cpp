/**
*	created:		2013-4-9   20:40
*	filename: 		FKGameGatewaySvrUserSession
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKGameGatewaySvrUserSession.h"
#include "FKGameGatewaySvr.h"
#include <fstream>
//------------------------------------------------------------------------
IMP_OP_NEW(CUserSession);
//------------------------------------------------------------------------
CUserSession::CUserSession(CLD_IocpBaseAccepter* Owner, SOCKET s):CLoopbufIocpClientSocketTask(Owner,s)
{
	FUNCTION_BEGIN;

	m_EncodeSetter.setEncMethod(_DEFAULT_ENCODE_TYPE);
	m_EncodeSetter.set_key_rc5((const unsigned char *) &connect_def_key_16_byte, sizeof(connect_def_key_16_byte), RC5_8_ROUNDS);
	m_GateWayUser=new CGameGateWayUser(this);
	m_pUpperEncodeSetter = &m_EncodeSetter;
	m_bConnectValid = false;
	m_SuccLogined=false;
	m_tryLogin=false;
	m_ipType=0;
	m_Activatetime=time(NULL);
	m_szAccount[0]=0;
	m_szPlayerNmae[0]=0;
	m_btGmLevel=0; 
	m_dwLastPostcmdsTick=GetTickCount()+60+_random(40);;
}
//------------------------------------------------------------------------
CUserSession::~CUserSession()
{
	do {
		GameGateService* gateway=GameGateService::instance();
		CGameGateWayUser* pGateWayUser=m_GateWayUser;
		m_GateWayUser=NULL;
		AILOCKT(gateway->m_waitdelgateuser);
		gateway->m_waitdelgateuser.insert(pGateWayUser);
	} while (false);
}
//------------------------------------------------------------------------
bool CUserSession::isvalid()
{
	return m_bConnectValid;
}
//------------------------------------------------------------------------
time_t CUserSession::valid_timeout()
{
	return 20;
}
//------------------------------------------------------------------------
void CUserSession::Terminate(const char* ffline,const TerminateMethod method)
{
	if (terminate==terminate_no && method!=terminate_client_active && IsConnected())
	{
		g_logger.debug("玩家 %s 角色 %s 被网关踢下线",m_szAccount,m_szPlayerNmae);
	}
	__super::Terminate();
}
//------------------------------------------------------------------------
void CUserSession::RecycleThreadCallBack()
{
	if (!m_SuccLogined)
	{
		if ( (time(NULL) - m_conntime)>60*2 || (m_tryLogin==false && (time(NULL) - m_conntime)>20 ) )
		{
			this->Terminate(__FF_LINE__);
		}
	}
	__super::RecycleThreadCallBack();
}
//------------------------------------------------------------------------
void CUserSession::pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam)
{
	GameGateService* gamegate=GameGateService::instance();
	bufferparam->bofreebuffer=true;
	AILOCKT(gamegate->m_clintsession);
	msgParse(pcmd,ncmdlen,bufferparam);
	if (bufferparam->bofreebuffer)
	{
		FreePacketBuffer(bufferparam->pQueueMsgBuffer);
	}
}
//------------------------------------------------------------------------
bool CUserSession::msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam)
{
	switch(pcmd->value)
	{
	default:
		{
			if (pcmd->value==stCheckSpeedCmd::_value)
			{
				stCheckSpeedCmd* pdstcmd=(stCheckSpeedCmd*)pcmd;
				pdstcmd->setLog(GameGateService::getMe().m_Svr2SvrLoginCmd.svr_id_type_value);
				g_logger.forceLog(zLogger::zFORCE, "(%d)测速信号_client %d:%s : r:%d(l:%d)",pdstcmd->dwProxyCount, pdstcmd->dwCheckIndex,pdstcmd->szCheckStr,pdstcmd->dwLocalTick,::GetTickCount() );
			}
			if (m_SuccLogined && m_GateWayUser)
			{
				m_GateWayUser->SendProxyCmd(pcmd,ncmdlen);
			}else
			{
				Terminate(__FF_LINE__);
			}
		}
		break;
	}
	return false;
}
//------------------------------------------------------------------------