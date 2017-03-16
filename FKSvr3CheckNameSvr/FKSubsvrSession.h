/**
*	created:		2013-4-7   23:28
*	filename: 		FKSubsvrSession
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "../FKSvr3Common/Include/BaseProtocol/BaseServerCmd.h"
//------------------------------------------------------------------------
class CSubSvrSession:public CLoopbufIocpClientSocketTask
{
public:
	union {
		struct {
			WORD svr_id;
			WORD svr_type;
		};
		DWORD svr_id_type_value;
	};
	stServerInfo m_svrinfo;
	bool m_bovalid;
	char m_szZoneName[_MAX_NAME_LEN_];
public:
	CSubSvrSession(CLD_IocpBaseAccepter* Owner, SOCKET s);
public:
	bool isvalid();
	time_t valid_timeout();
	virtual bool msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	bool OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
};
//------------------------------------------------------------------------
struct stMapSessionInfo
{
	CSubSvrSession* ownersession;
	stMapInfo mapinfo;
};
//------------------------------------------------------------------------
typedef std::map< DWORD,stMapSessionInfo > GameServerMapIDMap;
//------------------------------------------------------------------------