/**
*	created:		2013-4-9   16:20
*	filename: 		FKLogSvrConnecter
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
class CLogSvrConnecter:public CLoopbufIocpConnecterTask
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
public:
	virtual void OnIocpConnect();
	virtual void OnDisconnect();
	bool isvalid();
	virtual time_t valid_timeout();
public:
	CLogSvrConnecter(CLD_IocpHandle* Owner,stServerInfo* psvrinfo):CLoopbufIocpConnecterTask(Owner),svr_id_type_value(0),m_bovalid(false){
		if (psvrinfo){
			svr_id_type_value=psvrinfo->svr_id_type_value;
			m_svrinfo=*psvrinfo;
		}
	};

	virtual bool msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	bool OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
};
//------------------------------------------------------------------------