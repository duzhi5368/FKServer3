/**
*	created:		2013-4-7   23:25
*	filename: 		BaseServerCmd
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../Network/FKPacket.h"
#include "../FKGlobal.h"
//------------------------------------------------------------------------
#pragma pack(push,1)
//------------------------------------------------------------------------
#define CMD_SERVER2OTHERSVR		1
DEFINEUSERCMD1(stServer2OtherSvrCmd, CMD_SERVER2OTHERSVR);
//------------------------------------------------------------------------
//服务器 登陆 其他 某个服务器 的消息基本结构
#define SUBCMD_OTHERSVELOGIN2LOGINSVR		1
struct stSvr2SvrLoginCmd : public stServer2OtherSvrCmd< SUBCMD_OTHERSVELOGIN2LOGINSVR >
{
	DWORD dwCheckCode;
	union
	{
		struct
		{
			union
			{
				struct
				{
					WORD wgame_type;
					WORD wzoneid;
				};
				DWORD game_type_zoneid_value;
			};

			union
			{
				struct
				{
					WORD svr_id;
					WORD svr_type;
				};
				DWORD svr_id_type_value;
			};
		};
		__int64 svr_marking;
	};
	time_t m_now;
	char szZoneName[_MAX_NAME_LEN_];

	stSvr2SvrLoginCmd()
	{
		svr_marking = 0;
		dwCheckCode = _PRE_CHECK_CODE_;
		szZoneName[0] = 0;
	}
};
//------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------