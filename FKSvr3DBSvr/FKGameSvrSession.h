/**
*	created:		2013-4-9   15:43
*	filename: 		FKGameSvrSession
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "FKLoginSvrConnecter.h"
//------------------------------------------------------------------------
class CGameSvrSession:public CLoopbufIocpClientSocketTask
{
public:
	union {
		struct {
			WORD svr_id;
			WORD svr_type;
		};
		DWORD svr_id_type_value;
	};
public:
	stServerInfo m_ServerInfo;
	bool m_bovalid;
	int m_nPlayerCount;
	std::vector<stIpInfoState>	m_GateIds;
	char m_szZoneName[_MAX_NAME_LEN_];
public:
	bool checkGate(stIpInfoState* pipinfo){
		for (int j=0;j<(int)m_GateIds.size();j++){
			if (m_GateIds[j].ipinfo_id==pipinfo->ipinfo_id
				&& m_GateIds[j].type==pipinfo->type
				&& m_GateIds[j].ip.s_addr==pipinfo->ip.s_addr ){
					return true;
			}
		}
		return false;
	}
public:
	CGameSvrSession(CLD_IocpBaseAccepter* Owner, SOCKET s);

	bool isvalid();
	time_t valid_timeout();
	bool GetALoginIpInfo(in_addr& ip,WORD& port,BYTE ip_type);
	virtual bool msgParse(stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);

	bool OnstSvr2SvrLoginCmd(stSvr2SvrLoginCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam);
};
//------------------------------------------------------------------------
struct stMapSessionInfo
{
	CGameSvrSession* ownersession;
	stMapInfo mapinfo;
};
//------------------------------------------------------------------------
typedef std::map< DWORD,stMapSessionInfo > GameServerMapIDMap;
//------------------------------------------------------------------------