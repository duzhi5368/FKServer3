/**
*	created:		2013-4-6   22:47
*	filename: 		FKCheckNameSvr
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "../FKSvr3Common/Include/BaseProtocol/BaseServerCmd.h"
#include "FKSubsvrSession.h"
//------------------------------------------------------------------------
class CCheckNameTcpServices : public CMIocpTcpAccepters 
{
public:
	CCheckNameTcpServices()
		: CMIocpTcpAccepters("")
	{
	};
	virtual CLD_IocpClientSocket* CreateIocpClient(CMIocpTcpAccepters::CSubAccepter* pAccepter, SOCKET s);
	virtual void OnIocpClientConnect(CLD_Socket* Socket);
	virtual void OnClientDisconnect(CLD_Socket* Socket);
	virtual void OnClientError(CLD_Socket* Socket, CLD_TErrorEvent ErrEvent, OUT int& nErrCode, char* sErrMsg);
};

//------------------------------------------------------------------------
#define _SVR_PARAM_DBHASHCODE_			0
//------------------------------------------------------------------------
#ifdef _WX1_PRJ_
#define  _MAX_ONLYID_		0xFFFFFFFE
#else
#define  _MAX_ONLYID_		0xFFFFFFFE
#endif
#define  _MIN_ONLYID_		40000
//------------------------------------------------------------------------

#define MAKEBIGINT(h,l)	   ( ( ( (ULONGLONG)(h) ) << 32 ) | (l) )

#ifdef _WX1_PRJ_
#define MAKE_ONLYID(id,tid)					(id)
#else
#define MAKE_ONLYID(id,tid)					( (((id) & 0x3FFFFFF)<<6) | ((tid) & 0x3F) ) 
#define MAKE_PLAYER_ONLYID( PlatFormID, ZoneID, UserID )	MAKEBIGINT( ( ( ( PlatFormID ) << 8 ) | ZoneID ), UserID )
#endif // _WX1_PRJ_
//------------------------------------------------------------------------

class CCheckNameService : public Singleton< CCheckNameService>,public CWndBase 
{
public:
	void EnableCtrl( bool bEnableStart );
	bool CreateToolbar();

	virtual void OnStartService();
	virtual void OnStopService();
	virtual void OnConfiguration();

	virtual void OnQueryClose(bool& boClose);
	virtual bool OnInit();
	virtual void OnUninit();
	virtual long OnTimer( int nTimerID );
	virtual long OnCommand( int nCmdID );
	virtual bool OnCommand( char* szCmd );
	virtual bool Init( HINSTANCE hInstance );
	virtual void OnClearLog();
	virtual void OnIdle();
public:
	static const WORD m_svrtype=_CHECKNAMESVR_TYPE;
public:
	CLD_IocpHandle m_iocp;							// 完成端口对象
	CCheckNameTcpServices m_TcpServices;			// tcp服务器管理器
	CMIocpTcpConnters m_TcpConnters;
	bool m_boStartService;
	WORD m_svridx;
	char m_szsvrcfgdburl[MAX_PATH];
	char m_szgamesvrparamdburl[MAX_PATH];			//游戏服务器数据库地址
	char m_szSvrName[MAX_PATH];
	bool m_boAutoStart;
	int m_niocpworkthreadcount;
	char m_szmanageip[_MAX_IP_LEN_];				//游戏管理服务器IP
	WORD m_manageport;								//游戏管理服务器端口
	bool m_boConnectManage;							//是否连接游戏管理服务器
	char m_szLocalIp[_MAX_IP_LEN_];
	char m_szZoneName[_MAX_NAME_LEN_];
	int  m_nZoneid;
	WORD m_subsvrport;
	int  m_nTradeid;
	bool m_boshutdown;
	time_t m_timeshutdown;
	bool m_forceclose;
	CIntLock cfg_lock;	
	svrinfomap m_svrlistmap;				
	int m_dwUserOnlyId;
	int m_dwAccountOnlyId;
	int m_dwStrOnlyId;
	stSvr2SvrLoginCmd m_Svr2SvrLoginCmd;
	HashDBPool m_datadb;
	HashDBPool m_svrdatadb;
	CSyncSet< CSubSvrSession* > m_gamesvrsession;
	typedef LimitStrCaseHash<void*> checkstrmap;
	checkstrmap	m_accountmap;
	checkstrmap	m_namemap;
	LimitStrCaseHash<checkstrmap*> m_checkstrmap;
public:
	CCheckNameService();
	~CCheckNameService();

	void StartService();
	void RefSvrRunInfo();
	void ShutDown();
	void StopService();

	static void RecycleThreadCallBackProxy(void* pobj)
	{
		((CCheckNameService *) pobj)->RecycleThreadCallBack();
	}
	virtual void RecycleThreadCallBack();

	bool LoadLocalConfig();
	bool LoadSvrConfig(CSqlClientHandle* sqlc);
	bool LoadSysParam(CSqlClientHandle* sqlc);

	bool LoadServerParam();
	bool SaveServerParam(int naddtime,int nadditemid);

	CLD_ThreadBase* m_msgProcessThread;
	CLD_ThreadBase* m_LogicProcessThread;

	unsigned int __stdcall SimpleMsgProcessThread(CLD_ThreadBase* pthread,void* param);
	unsigned int __stdcall LogicProcessThread(CLD_ThreadBase* pthread,void* param);
};
//------------------------------------------------------------------------