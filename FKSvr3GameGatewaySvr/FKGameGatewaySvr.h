/**
*	created:		2013-4-9   17:05
*	filename: 		FKGameGatewaySvr
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "FKGameGatewayConneter.h"
#include "FKGameGatewaySvrUserSession.h"
//------------------------------------------------------------------------
class GateTcpServices : public CMIocpTcpAccepters 
{
public:
	class CGameSubAccepter:public CMIocpTcpAccepters::CSubAccepter
	{
	public:
		stIpInfoState m_ipinfo;

		CGameSubAccepter(CLD_IocpHandle* iocphandle,CMIocpTcpAccepters* pService=NULL,const std::string &name="",int type=0,int idx=0)
			:CSubAccepter(iocphandle,pService,name,type,idx){};
	};

	GateTcpServices()
		: CMIocpTcpAccepters("")
	{
	};
	virtual CLD_IocpClientSocket* CreateIocpClient(CMIocpTcpAccepters::CSubAccepter* pAccepter, SOCKET s);
	virtual void OnIocpClientConnect(CLD_Socket* Socket);
	virtual void OnClientDisconnect(CLD_Socket* Socket);
	virtual void OnClientError(CLD_Socket* Socket, CLD_TErrorEvent ErrEvent, OUT int& nErrCode, char* sErrMsg);
	virtual CSubAccepter* CreateAccepter(CLD_IocpHandle* iocphandle,CMIocpTcpAccepters* pService=NULL,const std::string &name="",int type=0,int idx=0){
		return (CSubAccepter*)new CGameSubAccepter(iocphandle,pService,name,type,idx);
	}
};
//------------------------------------------------------------------------
class GameGateService : public Singleton< GameGateService>,public CWndBase 
{
public:
	void RunStep();
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
	CLD_IocpHandle m_iocp;				//完成端口对象
	GateTcpServices m_TcpServices;		//tcp服务器管理器
	CMIocpTcpConnters m_TcpConnters;
	bool m_boStartService;

	static const WORD m_svrtype=_GAMESVR_GATEWAY_TYPE;

	WORD m_svridx;
	char m_szsvrcfgdburl[MAX_PATH];
	char m_szSvrName[MAX_PATH];
	bool m_boAutoStart;
	int m_niocpworkthreadcount;
	char m_szmanageip[_MAX_IP_LEN_];				//游戏管理服务器IP
	WORD m_manageport;								//游戏管理服务器端口
	bool m_boConnectManage;							//是否连接游戏管理服务器
	char m_szLocalIp[_MAX_IP_LEN_];
	char m_szZoneName[_MAX_NAME_LEN_];
	int  m_nZoneid;
	WORD m_wgatewayowner;
	bool m_boshutdown;
	time_t m_timeshutdown;
	bool m_forceclose;
	bool tcpstate_changed;
public:
	DWORD m_lastRunTick;
	CIntLock cfg_lock;	
	svrinfomap m_svrlistmap;	
	stSvr2SvrLoginCmd m_Svr2SvrLoginCmd;
	loginsvrencryptmap m_loginsvrenc;
	CSyncSet< CUserSession* > m_clintsession;
	CSyncMap< DWORD,CGameGateWayConnecter* > m_gatewayconnter;
	GameServerMapIDMap m_gatewayconnter_mapid;
	CSyncSet< CGameGateWayUser* >  m_waitdelgateuser;					
public:
	GameGateService();
	~GameGateService();

	void StartService();
	void RefSvrRunInfo();
	void ShutDown();
	void StopService();

	static void RecycleThreadCallBackProxy(void* pobj)
	{
		((GameGateService *) pobj)->RecycleThreadCallBack();
	}
	virtual void RecycleThreadCallBack();

	bool LoadLocalConfig();
	bool LoadSvrConfig(CSqlClientHandle* sqlc);
	bool LoadSysParam(CSqlClientHandle* sqlc);

	int m_accdbcount;
	int m_acctblcount1db;
	int m_accalltblcount;

	int m_datadbcount;
	int m_datatblcount1db;
	int m_dataalltblcount;

	bool decodetoken(DWORD loginsvr_id,stLoginToken* pSrcToken,stLoginToken* pDstToken,DWORD tock);

	CLD_ThreadBase* m_msgProcessThread;
	CLD_ThreadBase* m_LogicProcessThread;

	unsigned int __stdcall SimpleMsgProcessThread(CLD_ThreadBase* pthread,void* param);
	unsigned int __stdcall LogicProcessThread(CLD_ThreadBase* pthread,void* param);

	unsigned int __stdcall	LogicProcessThread_Try_S1();
	unsigned int __stdcall	LogicProcessThread_Try_S2();
	unsigned int __stdcall	LogicProcessThread_Try_S3();

	void RefStateCmd();
	bool Send2GameSvrs(void* pcmd,int ncmdlen);
};
//------------------------------------------------------------------------