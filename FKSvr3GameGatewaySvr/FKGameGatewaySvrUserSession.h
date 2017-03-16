/**
*	created:		2013-4-9   20:27
*	filename: 		FKGameGatewaySvrUserSession
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "FKGameGatewayConneter.h"
//------------------------------------------------------------------------
//默认加密类型
#define		_DEFAULT_ENCODE_TYPE		CEncrypt::ENCDEC_NONE
//------------------------------------------------------------------------
class CUserSession:public CLoopbufIocpClientSocketTask
{
public:
	CEncrypt m_EncodeSetter;
	bool m_bConnectValid;
	bool m_SuccLogined;
	bool m_tryLogin;
	BYTE m_ipType;
	time_t m_Activatetime;
	BYTE m_btGmLevel;
	CGameGateWayUser* m_GateWayUser;

	char m_szAccount[_MAX_ACCOUT_LEN_];
	char m_szPlayerNmae[_MAX_ACCOUT_LEN_];
	DWORD m_dwLastPostcmdsTick;

	CUserSession(CLD_IocpBaseAccepter* Owner, SOCKET s);
	~CUserSession();

	bool isvalid();
	time_t valid_timeout();
	void RecycleThreadCallBack();

	virtual void Terminate(const char* ffline,const TerminateMethod method = terminate_server_active);

	virtual void pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	virtual bool msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	DEC_OP_NEW(CUserSession);
};
//------------------------------------------------------------------------