/**
*	created:		2013-4-9   15:18
*	filename: 		FKDBSvr
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "FKGameSvrSession.h"
#include "FKLoginSvrConnecter.h"
#include "FKLogSvrConnecter.h"
#include "FKCheckNameSvrConnecter.h"
//------------------------------------------------------------------------
class GameSvrTcpServices : public CMIocpTcpAccepters 
{
public:
	GameSvrTcpServices()
		: CMIocpTcpAccepters("")
	{
	};
	virtual CLD_IocpClientSocket* CreateIocpClient(CMIocpTcpAccepters::CSubAccepter* pAccepter, SOCKET s);
	virtual void OnIocpClientConnect(CLD_Socket* Socket);
	virtual void OnClientDisconnect(CLD_Socket* Socket);
	virtual void OnClientError(CLD_Socket* Socket, CLD_TErrorEvent ErrEvent, OUT int& nErrCode, char* sErrMsg);
};
//------------------------------------------------------------------------
class DBService : public Singleton< DBService>,public CWndBase 
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
	CLD_IocpHandle m_iocp;						//完成端口对象
	GameSvrTcpServices m_TcpServices;			//tcp服务器管理器
	CMIocpTcpConnters m_TcpConnters;
	bool m_boStartService;

	static const WORD m_svrtype=_DBSVR_TYPE;

	WORD m_svridx;
	char m_szsvrcfgdburl[MAX_PATH];
	char m_szSvrName[MAX_PATH];
	bool m_boAutoStart;
	int m_niocpworkthreadcount;
	char m_szmanageip[_MAX_IP_LEN_];			//游戏管理服务器IP
	WORD m_manageport;							//游戏管理服务器端口
	bool m_boConnectManage;						//是否连接游戏管理服务器
	char m_szLocalIp[_MAX_IP_LEN_];
	char m_szZoneName[_MAX_NAME_LEN_];
	int  m_nZoneid;
	WORD m_gatewayport;
	WORD m_dbport;
	WORD m_gameport;
	bool m_boshutdown;
	time_t m_timeshutdown;
	bool m_forceclose;
	int m_newManMapID;
	int m_newManX;
	int m_newManY;
public:
	CIntLock cfg_lock;	
	svrinfomap m_svrlistmap;				
	typedef std::vector< dbColProxy >	vecloadcol;

	struct stDbCols{
		dbCol* lastbindata;
		DWORD dwvertypevalue;
		int datasize;
		int bindataoffset;
		vecloadcol cols;
	};
	typedef std::CSyncMap< DWORD,stDbCols* >	dbcolmap;
	dbcolmap	m_dbcolmap;
	struct stBaseCmdTrans{
		stQueueMsg* pmsgbuffer;
		stBaseCmd* pcmd;
		time_t puttime;
		CLoginSvrDBConnecter* loginsvr;
		stBaseCmdTrans(stQueueMsg* pbuffer=NULL,stBaseCmd* p=NULL,CLoginSvrDBConnecter* plogin=NULL):pmsgbuffer(pbuffer),pcmd(p),loginsvr(plogin){	puttime=time(NULL); };
	};
	typedef zTransidManage<DWORD,stBaseCmdTrans> zBaseCmdTransidManage;
	zBaseCmdTransidManage  m_checkcreateplayernametransmanage;

	stSvr2SvrLoginCmd m_Svr2SvrLoginCmd;
	int m_accountdbcount;
	int m_accounttblcount1db;
	int m_accountalltblcount;

	HashDBPool m_datadb;

	CSyncSet< CGameSvrSession* > m_gamesvrsession;
	GameServerMapIDMap m_gamesvrsession_mapid;
	CSyncMap< int,stMapSubLineInfo > m_mapsublineset;
	std::vector<stIpInfoState> m_gamesvrips ;

	CSyncSet< CLoginSvrDBConnecter* > m_loginsvrconnter;
	LoginServerHashCodeMap m_loginsvrconnter_hashcode;
	CCheckNameSvrConnecter* m_checknamesvrconnecter;
	CLogSvrConnecter*	m_logsvrconnecter;	
	bool m_boopencheckname;

	std::CSyncMap< int,stCountryInfo > m_countrtmaps;
public:
	DBService();
	~DBService();

	int getFaction(int ncountryid);

	void StartService();
	void RefSvrRunInfo();
	void ShutDown();
	void StopService();

	static void RecycleThreadCallBackProxy(void* pobj)
	{
		((DBService *) pobj)->RecycleThreadCallBack();
	}
	virtual void RecycleThreadCallBack();

	bool LoadLocalConfig();
	bool LoadSvrConfig(CSqlClientHandle* sqlc);
	bool LoadSysParam(CSqlClientHandle* sqlc);

	bool sendCmd2LoginSvrByAccount(const char* szAccount,void* pbuf, unsigned int nsize, int zliblevel = Z_DEFAULT_COMPRESSION );
	CLoginSvrDBConnecter* getLoginsvrByAccount(const char* szAccount);

	CLD_ThreadBase* m_msgProcessThread;
	CLD_ThreadBase* m_LogicProcessThread;

	unsigned int __stdcall SimpleMsgProcessThread(CLD_ThreadBase* pthread,void* param);
	unsigned int __stdcall LogicProcessThread(CLD_ThreadBase* pthread,void* param);

	bool  doGmOperate(stBaseCmd* pcmd,CLoginSvrDBConnecter* pThis,unsigned int ncmdlen);
};
//------------------------------------------------------------------------