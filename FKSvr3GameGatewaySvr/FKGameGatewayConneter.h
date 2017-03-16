/**
*	created:		2013-4-9   17:07
*	filename: 		FKGameGatewayConneter
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "FKGameGatewayClientSession.h"
#include "../FKSvr3Common/Include/BaseProtocol/BaseServerCmd.h"
//------------------------------------------------------------------------
class CUserSession;
//------------------------------------------------------------------------
class CGameGateWayUser:public stGateWayConnecterUser
{
public:
	CUserSession* m_Owner;
	char m_szAccount[_MAX_ACCOUT_LEN_];
	char m_szPlayerNmae[_MAX_ACCOUT_LEN_];

	CGameGateWayUser(CUserSession* Owner):m_Owner(Owner){
		m_szAccount[0]=0;
		m_szPlayerNmae[0]=0;
	};
	virtual bool svrmsgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	virtual bool OnsvrKickUser();
	DEC_OP_NEW(CGameGateWayUser);
};
//------------------------------------------------------------------------
class CGameGateWayConnecter:public CGateWayConnecter
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
	CGameGateWayConnecter(CLD_IocpHandle* Owner,stServerInfo* psvrinfo):CGateWayConnecter(Owner),svr_id_type_value(0),m_bovalid(false){
		if (psvrinfo){
			svr_id_type_value=psvrinfo->svr_id_type_value;
			m_svrinfo=*psvrinfo;
			m_dwMaxRecvPacketLen=_MAX_SEND_PACKET_SIZE_;
		}
	};
	void pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	virtual bool svr2GatemsgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
	bool OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
};
//------------------------------------------------------------------------
struct stMapConnterInfo
{
	CGameGateWayConnecter* ownerconnecter;
	stMapInfo mapinfo;
};
//------------------------------------------------------------------------
typedef std::map< DWORD,stMapConnterInfo > GameServerMapIDMap;
typedef CSyncMap< DWORD,CEncrypt > loginsvrencryptmap;
//------------------------------------------------------------------------