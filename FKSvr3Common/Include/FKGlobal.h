/**
*	created:		2013-4-7   21:55
*	filename: 		FKGlobal
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <time.h>
#include <Windows.h>
#include <malloc.h>
#include "Network/FKAsynSocket.h"
#include "Endec/FKEncDec.h"
#include "Endec/FKCrc32.h"
#include "FKStringEx.h"
#include "RTTI/FKReflect.h"
#include "FKXmlParser.h"
#include "Network/FKPacket.h"
#include "Endec/FKBase64.h"
#include "DataBase/FKDBConnPool.h"
//------------------------------------------------------------------------
#define		_LOGINSVR_TYPE				100		//帐号服务器
#define		_DBSVR_TYPE					200		//数据库服务器
#define		_GAMESVR_TYPE				300		//游戏管理服务器(加载个子游戏模块,为子游戏模块提供通信服务)
#define		_SUPERGAMESVR_TYPE			400		//游戏服务器(管理全区数据)
#define		_CHECKNAMESVR_TYPE			500		//名字重复检查服务器(跨区)
#define		_GLOBAL_GAMESVR_TYPE		600		//竞技场游戏服务器(跨区)
#define		_LOGSVR_TYPE				700		//日志服务器
#define		_GMSERVERMANAGE_TYPE		800		//游戏管理服务器(单机)
#define		_GLOBAL_LOGSVR_TYPE			900		//全局日志服务器(跨区)
#define		_RMBSVR_TYPE				1000	//充值服务器(管理一个区，只连接LOGIN)
#define		_LOGINSVR_GATEWAY_TYPE		1100	//登陆网关
#define		_GAMESVR_GATEWAY_TYPE		1300	//游戏网关
//------------------------------------------------------------------------
#define		_MAX_ACCOUT_LEN_			48		//最大账号长度
#define		_MAX_NAME_LEN_				48		//最大名字长度
#define		_MAX_PASS_LEN_				48		//最大密码长度
#define		_MAX_IP_LEN_				24		//最大IP长度
#define		_MAX_RMB_LEN_				24		//最大订单长度
//------------------------------------------------------------------------
#define		_PRE_CHECK_CODE_				0x55884433					//连接 帐号服务器 第一个包的校验码
#define		DB_CONFIG_TBL					"mydb_svrcfg_tbl"			//所有合法服务器列表
#define		DB_CHKSERVERPARAM_TBL			"mydb_chksvrparam_tbl"		//服务器的保存参数的表
#define 	_FLASH_AUTHORITY_IP_			"0.0.0.0"
#define 	_FLASH_AUTHORITY_PORT_			843
#define 	_FLASH_AUTHORITY_NAME_			"flashauthority_549sdfjuaosud5trs"
#define 	_FLASH_AUTHORITY_RET_			"<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>\0"

//------------------------------------------------------------------------
struct stServerInfo
{
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
	char szZoneName[_MAX_NAME_LEN_];
	char szName[_MAX_NAME_LEN_];
	char szIp[_MAX_IP_LEN_];			//服务器内部数据交换使用的IP
	WORD GatewayPort;					//网关端口
	WORD DBPort;						//数据库服务器连接的端口
	WORD GamePort;						//游戏服务器连接的端口
	WORD SubsvrPort;

	stServerInfo()
	{
		ZEROOBJ(this);
	};
};
//------------------------------------------------------------------------
typedef std::map< __int64, stServerInfo> svrinfomap;
typedef std::map< __int64, stServerInfo>::value_type svrinfomaptype;
typedef std::map< __int64, stServerInfo>::iterator svrinfomapiter;
//------------------------------------------------------------------------
struct stMapInfo
{
	union
	{
		struct
		{
			WORD wmapid;
			BYTE wmapcountryid;
			BYTE btmapsublineid;
		};
		DWORD dwmapid;
	};
	WORD w;
	WORD h;

	stMapInfo()
	{
		memset(this,0,sizeof(*this));
	}
};
//------------------------------------------------------------------------
struct stIpInfoState
{
	in_addr ip;
	union
	{
		struct
		{
			WORD svr_id;
			WORD port;
		};
		DWORD ipinfo_id;
	};
	WORD ncount;

	BYTE type;
	BYTE state;			//0 正常  -1 关闭

	stIpInfoState()
	{
		ipinfo_id = 0;
		ncount = 0;
		state = 0;
		type = 0;
	}
};
//------------------------------------------------------------------------
/*[服务器地图分线]*/
struct stMapSubLineInfo
{
	BYTE btmapsublineid;
	int nmapcount;
	int nonlineusercount;

	stMapSubLineInfo()
	{
		ZEROOBJ(this);
	}
};
//------------------------------------------------------------------------
struct stCountryInfo
{
	int nCountryId;
	char szCountry[_MAX_NAME_LEN_];
	char szCountryHomeMap[_MAX_NAME_LEN_];
	DWORD dwCountryHomeMapId;
	DWORD countryhomex;
	DWORD countryhomey;
	BYTE btisdel;
	int nonlineusercount;
	int nusercount;
	int nfaction;

	RTTI_DESCRIBE_STRUCT(stCountryInfo, (
		RTTI_FIELD_N(nCountryId, 0, countryid)
		, RTTI_ARRAY_N(szCountry, 0, countryname)
		, RTTI_ARRAY_N(szCountryHomeMap, 0, countryhomemap)
		, RTTI_FIELD_N(countryhomex, 0, countryhomex)
		, RTTI_FIELD_N(countryhomey, 0, countryhomey)
		, RTTI_FIELD_N(btisdel, 0, deleted)
		));

	stCountryInfo()
	{
		ZEROOBJ(this);
	}
};
//------------------------------------------------------------------------
struct stSvrTmpId
{
	union
	{
		struct
		{
			WORD loginsvr_id;		//把不同的 帐号服务器 产生的临时ID区分开 防止产生相同的ID
			WORD tmp;				//保留 //svr_type
			DWORD dwtmpid;
		};
		unsigned __int64 IDValue;
	};
	__inline DWORD getsvridtype(WORD wtype = _LOGINSVR_TYPE)
	{
		return MAKELONG(loginsvr_id , wtype);
	}
};
//------------------------------------------------------------------------
struct stLoginTokenData
{
public:
	time_t createtime;
	time_t updatatime;
	DWORD tokencheck;
	in_addr ip;						//登陆IP
	stSvrTmpId loginsvr_tmpid;
public:
	stLoginTokenData()
	{
		createtime = time(NULL);
	}
	void encode(CEncrypt* enc, DWORD tock)
	{
		updatatime = time(NULL);

		for(int i = 0; i <= (sizeof(*this) - sizeof(tock)); i++)
		{
			*((DWORD*)(&((char*)this)[i])) = *((DWORD*)(&((char*)this)[i])) ^ tock;
		}

		if(enc)
		{
			enc->encdec(this, sizeof(*this), true);
		}
	}
	bool decode(CEncrypt* enc, DWORD tock)
	{
		if(enc)
		{
			enc->encdec(this, sizeof(*this), false);
		}

		for(int i = (sizeof(*this) - sizeof(tock)); i >= 0; i--)
		{
			*((DWORD*)(&((char*)this)[i])) = *((DWORD*)(&((char*)this)[i])) ^ tock;
		}

		return (loginsvr_tmpid.IDValue != 0 && ip.s_addr != INADDR_NONE);
	}
	bool isvalid(DWORD tock, DWORD svr_id, in_addr cip)
	{
		return ((tokencheck == tock) && loginsvr_tmpid.getsvridtype(_LOGINSVR_TYPE) == svr_id && (ip.s_addr == cip.s_addr || ip.s_addr == 0x0100007f /*127.0.0.1*/  || ip.s_addr == 0 /*0.0.0.0*/));
	}
	void init(DWORD tock, stSvrTmpId in_tmpid, in_addr cip)
	{
		ip.s_addr = cip.s_addr;
		loginsvr_tmpid = in_tmpid;
		tokencheck = tock;
	}
};
//------------------------------------------------------------------------
typedef		stLoginTokenData 	stLoginToken;
STATIC_ASSERTMN2(sizeof(stLoginToken) == ROUNDNUM2(sizeof(stLoginToken), 8), sizeof(stLoginToken), ROUNDNUM2(sizeof(stLoginToken), 8), stlogintoken_not_alig_power8);
//------------------------------------------------------------------------
typedef bool(__stdcall *fnInitAutoComplete)(HANDLE, char*);
typedef bool(__stdcall *fnSaveGMOrder)(char*);
//------------------------------------------------------------------------
#define INIT_AUTOCOMPLETE(h,dllname,p1,p2,cmdlist,saveproc,hdll)\
	if ( h && IsDebuggerPresent()!=TRUE )\
{\
	(hdll)=::LoadLibrary(dllname);\
	if (hdll)\
{\
	fnInitAutoComplete pInitAutoComplete=(fnInitAutoComplete)::GetProcAddress((hdll),p1);\
	if (pInitAutoComplete)\
{\
	pInitAutoComplete(h,cmdlist);\
}\
	(saveproc)=NULL;\
	(saveproc)=(fnSaveGMOrder)::GetProcAddress((hdll),p2);\
}\
}
//------------------------------------------------------------------------