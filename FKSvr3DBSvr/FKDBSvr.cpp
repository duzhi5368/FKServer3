/**
*	created:		2013-4-9   15:57
*	filename: 		FKDBSvr
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
#include "FKDBScript.h"
#include <malloc.h>
#include <Ole2.h>
#include "FKDBSvr.h"
#include "res/resource.h"
//------------------------------------------------------------------------
#pragma warning(disable:4239)		//使用了非标准扩展 : “参数” : 从“DBService::RecycleThreadCallBack::stcheckSession”转换到“zHashManagerBase<valueT,e1>::removeValue_Pred_Base &”
//------------------------------------------------------------------------
#define _GAMESVR_CONN_				0
#define _STARTSERVICE_LOG_			"FKDBSvr 开始启动服务..."
//------------------------------------------------------------------------
#define _LOGIN_SVR_CONNETER_			0
#define _CHECKNAME_SVR_CONNETER_		1
#define  _LOG_SVR_CONNETER_				2
//------------------------------------------------------------------------
CLD_IocpClientSocket* GameSvrTcpServices::CreateIocpClient(CMIocpTcpAccepters::CSubAccepter* pAccepter, SOCKET s)
{
	FUNCTION_BEGIN;
	DBService* dbsvr=DBService::instance();
	if (dbsvr->m_boshutdown|| !dbsvr->m_boStartService){
		return NULL;
	}
	switch (pAccepter->gettype())
	{
	case _GAMESVR_CONN_:
		{
			svrinfomapiter it;
			bool bofind = false;	
			char szremoterip[32];
			CLD_Socket::GetRemoteAddress(s,szremoterip,sizeof(szremoterip));
			do{
				AILOCKT(dbsvr->cfg_lock);
				for (it = dbsvr->m_svrlistmap.begin(); it != dbsvr->m_svrlistmap.end(); it++) {
					stServerInfo* psvr = &it->second;
					if (stricmp(psvr->szIp, szremoterip) == 0 && psvr->svr_type==_GAMESVR_TYPE) {
						bofind = true;
						break;
					}
				}
			}while (false);
			if (bofind) {
				CGameSvrSession* pqpsocket = new CGameSvrSession(pAccepter, s);
				AILOCKT(dbsvr->m_gamesvrsession);
				dbsvr->m_gamesvrsession.insert(pqpsocket);
				return (CLD_IocpClientSocket *) pqpsocket;
			} else {
				g_logger.warn("%s : %d 非法的服务器连接...", szremoterip, CLD_Socket::GetRemotePort(s));
			}
		}
		break;
	}
	return NULL;
}
//------------------------------------------------------------------------
void GameSvrTcpServices::OnIocpClientConnect(CLD_Socket* Socket)
{
	FUNCTION_BEGIN;
	CMIocpTcpAccepters::CSubAccepter* pAccepter = (CMIocpTcpAccepters::CSubAccepter*) ((CLD_IocpClientSocket*) Socket)->GetAccepter();
	if (pAccepter){
		g_logger.debug("%s:%d(%s) 连接成功...", Socket->GetRemoteAddress(), Socket->GetRemotePort(), pAccepter->getdis());
	}else{
		g_logger.debug("%s:%d 连接成功...", Socket->GetRemoteAddress(), Socket->GetRemotePort());
	}
}
//------------------------------------------------------------------------
void GameSvrTcpServices::OnClientDisconnect(CLD_Socket* Socket)
{
	FUNCTION_BEGIN;
	CMIocpTcpAccepters::CSubAccepter* pAccepter = (CMIocpTcpAccepters::CSubAccepter*) ((CLD_IocpClientSocket*) Socket)->GetAccepter();
	switch (pAccepter->gettype())
	{
	case _GAMESVR_CONN_:
		{
			//网关断开连接
			DBService* dbsvr=DBService::instance();
			CGameSvrSession* pgamesvr=(CGameSvrSession*)Socket;
			do {
				AILOCKT(dbsvr->m_gamesvrsession);
				GameServerMapIDMap::iterator it,itnext;
				for (it=dbsvr->m_gamesvrsession_mapid.begin(),itnext=it;it!=dbsvr->m_gamesvrsession_mapid.end();it=itnext){
					itnext++;
					if (it->second.ownersession==pgamesvr){
						dbsvr->m_gamesvrsession_mapid.erase(it);
					}
				}
				pgamesvr->m_GateIds.clear();
				dbsvr->m_gamesvrsession.erase(pgamesvr);
			}while(false);
		}
		break;
	}
	g_logger.debug("%s:%d(%s) 连接断开...", Socket->GetRemoteAddress(), Socket->GetRemotePort(), pAccepter->getdis());
}
//------------------------------------------------------------------------
void GameSvrTcpServices::OnClientError(CLD_Socket* Socket, CLD_TErrorEvent ErrEvent, OUT int& nErrCode, char* sErrMsg)
{
	FUNCTION_BEGIN;
	g_logger.debug("%s:%d 连接异常(%d->%s)...", Socket->GetRemoteAddress(), Socket->GetRemotePort(), nErrCode, sErrMsg);
	CMIocpTcpAccepters::CSubAccepter* pAccepter = (CMIocpTcpAccepters::CSubAccepter*) ((CLD_IocpClientSocket*) Socket)->GetAccepter();
	if (nErrCode!=0){
		switch (pAccepter->gettype())
		{
		case _GAMESVR_CONN_:
			{
				((CGameSvrSession*)Socket)->Terminate();
			}
			break;
		}
		nErrCode=0;
	}
}
//------------------------------------------------------------------------
DBService::DBService()
:CWndBase(WND_WIDTH,WND_HEIGHT*2,"DBService_Wnd")
{
	FUNCTION_BEGIN;
	m_timeshutdown=0;
	m_forceclose=false;

	m_niocpworkthreadcount=0;
	m_boStartService = false;
	m_boshutdown=false;
	m_svridx = 0;
	m_szsvrcfgdburl[0]=0;
	m_szSvrName[0]=0;
	m_szZoneName[0]=0;
	m_nZoneid=0;

	m_szmanageip[0]=0;
	m_manageport=5000;
	m_boConnectManage=false;

	m_msgProcessThread=NULL;
	m_LogicProcessThread=NULL;
	m_checknamesvrconnecter=NULL;
	m_logsvrconnecter=NULL;
	m_boopencheckname=true;
	m_iocp.SetRecycleThreadCallBack(RecycleThreadCallBackProxy, this);

	m_Svr2SvrLoginCmd.svr_id=m_svridx;
	m_Svr2SvrLoginCmd.svr_type=m_svrtype;

	static char DBHashCodeCmdBuf[1024 * 4];

	LoadLocalConfig();
	char szTemp[MAX_PATH];
	char szruntime[20];
	timetostr(time(NULL),szruntime,20);
	sprintf_s(szTemp,MAX_PATH,"DS[%s : DBSvr](BuildTime: %s)-(RunTime: %s)",m_szSvrName,__BUILD_DATE_TIME__,szruntime);
	SetTitle(szTemp);
}
//------------------------------------------------------------------------
HMODULE g_AutoCompleteh=0;
//------------------------------------------------------------------------
DBService::~DBService(){
	if (g_AutoCompleteh){
		FreeLibrary(g_AutoCompleteh);
		g_AutoCompleteh=0;
	}
}
//------------------------------------------------------------------------
void DBService::RecycleThreadCallBack()
{
	FUNCTION_BEGIN;
	FUNCTION_WRAPPER(true,NULL);

	time_t curtime = time(NULL);
	if (m_boStartService) {
		static time_t s_timeAction=time(NULL)+2;
		if (curtime > s_timeAction && !m_boshutdown) {
			m_TcpConnters.timeAction();
			s_timeAction = time(NULL) + 4;
		}

		curtime = time(NULL);
		static time_t s_timeClearTimeOutTrans=time(NULL)+2;
		if (curtime > s_timeClearTimeOutTrans){
			DBService* service=DBService::instance();
			do {
				AILOCKT(service->m_checkcreateplayernametransmanage);
				DBService::zBaseCmdTransidManage::iterator it,itnext;
				for (it=service->m_checkcreateplayernametransmanage.begin(),itnext=it;it!=service->m_checkcreateplayernametransmanage.end();it=itnext){
					itnext++;
					if (curtime-it->second.puttime>2*60){
						FreePacketBuffer(it->second.pmsgbuffer);
						service->m_checkcreateplayernametransmanage.getmap().erase(it);
					}
				}
			} while (false);
			s_timeClearTimeOutTrans=time(NULL)+30;
		}
	}
}
//------------------------------------------------------------------------
int DBService::getFaction(int ncountryid){
#ifdef _WX1_PRJ_
	return WX1_getFaction(ncountryid);
#else
	DBService* login=this;
	std::CSyncMap< int,stCountryInfo >::iterator it=login->m_countrtmaps.find(ncountryid);
	if (it!=login->m_countrtmaps.end() && it->second.btisdel==0){
		return it->second.nfaction;
	}else{
		return 0;
	}
#endif
}
//------------------------------------------------------------------------
void DBService::StartService()
{
	FUNCTION_BEGIN;
	g_logger.forceLog( zLogger::zFATAL, "StartService" );

	if (!g_onexceptionbegincallback){ 
		minidump_type=minidump_type | MiniDumpWithPrivateReadWriteMemory | MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithProcessThreadData;
	}
	if (m_boStartService || !LoadLocalConfig()) {
		return;
	}

	g_logger.forceLog(zLogger::zINFO,_STARTSERVICE_LOG_);
#if defined RELEASE_TEST
	g_logger.forceLog(zLogger::zINFO,"RELEASE_TEST");
#endif
	stUrlInfo cfgui(0, m_szsvrcfgdburl/*"msmdb://game:k3434yiuasdh848@127.0.0.1:0/\".\\db\\mydb_svrcfg.mdb\""*/, true);

	g_logger.forceLog( zLogger::zFATAL, "%s" , m_szsvrcfgdburl );

	CSqlClientHandle* cfgsqlc = cfgui.NewSqlClientHandle();
	if (cfgsqlc)
	{
		if (cfgsqlc->setHandle())
		{
			m_iocp.Init(m_niocpworkthreadcount);
			m_iocp.SetIocpCallBack(true, 500 , 800 , 500);
			if (LoadSvrConfig(cfgsqlc)) 
			{
				g_logger.forceLog( zLogger::zFATAL, "已经成功加载svrConfig" );

				LoadSysParam(cfgsqlc);

				m_boStartService = false;

				m_msgProcessThread=CThreadFactory::CreateBindClass(this,&DBService::SimpleMsgProcessThread,(void*)NULL);
				if (m_msgProcessThread)
				{
					m_msgProcessThread->Start(false);
					m_LogicProcessThread=CThreadFactory::CreateBindClass(this,&DBService::LogicProcessThread,(void*)NULL);
					if (m_LogicProcessThread)
					{
						m_LogicProcessThread->Start(false);
						m_boStartService = true;
					}
					else
					{
						g_logger.forceLog( zLogger::zFATAL, "m_LogicProcessThread 失败" );
					}
				}
				else
				{
					g_logger.forceLog( zLogger::zFATAL, "m_msgProcessThread 失败" );
				}

			} else 
			{
				g_logger.forceLog( zLogger::zFATAL, "[LoadSvrConfig] 失败" );
				m_boStartService = true;
				StopService();
				m_boStartService = false;
			}
		}
		else
		{
			g_logger.forceLog( zLogger::zFATAL, "set handle 失败" );
		}
		cfgsqlc->unsetHandle();
		SAFE_DELETE(cfgsqlc);
	}
	else
	{
		g_logger.forceLog( zLogger::zFATAL, "分配sql指针失败" );
	}
	if (m_boStartService){
		g_logger.forceLog(zLogger::zINFO,"启动服务成功...");
		m_TcpConnters.timeAction(true);
	} else {
		g_logger.forceLog(zLogger::zINFO,"启动服务失败123123123123...");
	}
}
//------------------------------------------------------------------------
void DBService::RefSvrRunInfo(){
	FUNCTION_WRAPPER_RESET(true);
	if (!m_boshutdown){
		CThreadMonitor::getme().DebugPrintAllThreadDebufInfo();
	}
}
//------------------------------------------------------------------------
void DBService::ShutDown()
{
	FUNCTION_BEGIN;
	if (m_boshutdown){return;}
	if (!m_boStartService){return;}
	m_boshutdown=true;
	StopService();
	this->Processmsg();
	Sleep(2000);
}
//------------------------------------------------------------------------
void DBService::StopService()
{
	FUNCTION_BEGIN;
	if (!m_boStartService) {
		return;
	}
	g_logger.forceLog(zLogger::zINFO,"开始停止服务...");

	do {
		if (m_msgProcessThread){ m_msgProcessThread->Suspend();}
		m_checknamesvrconnecter=NULL;
		m_logsvrconnecter=NULL;
		if (m_msgProcessThread){ m_msgProcessThread->Resume();}
	}while(false);
	do{
		AILOCKT(m_loginsvrconnter);
		m_loginsvrconnter.clear();
	} while (false);

	do{
		AILOCKT(m_gamesvrsession);
		m_gamesvrsession.clear();
	} while (false);

	m_TcpConnters.closeall();
	m_TcpServices.closeall();
	m_iocp.Uninit();

	if (m_msgProcessThread){
		m_msgProcessThread->Terminate();
		m_msgProcessThread->Waitfor();
		SAFE_DELETE(m_msgProcessThread);
	}

	if (m_LogicProcessThread){
		m_LogicProcessThread->Terminate();
		m_LogicProcessThread->Waitfor();
		SAFE_DELETE(m_LogicProcessThread);
	}

	m_TcpConnters.clear();
	m_TcpServices.clear();

	clearpmap2(m_dbcolmap);

	m_boStartService = false;
	g_logger.forceLog(zLogger::zINFO,"停止服务成功...");
}
//------------------------------------------------------------------------
bool DBService::LoadLocalConfig()
{
	g_logger.forceLog( zLogger::zFATAL, "LoadLocalConfig" );

	FUNCTION_BEGIN;
	CXMLConfigParse config;
	config.InitConfig();
	if (m_szsvrcfgdburl[0]==0){
		config.readstr("svrcfg",m_szsvrcfgdburl,sizeof(m_szsvrcfgdburl)-1,"msmdb://game:k3434yiuasdh848@127.0.0.1:0/\".\\db\\mydb_svrcfg.mdb\"");
		if (m_szsvrcfgdburl[0]==0)
		{
			g_logger.forceLog( zLogger::zFATAL, "m_szsvrcfgdburl return" );
			return false;
		}
	}
	if (m_szSvrName[0]==0){
		config.readstr("svrname",m_szSvrName,sizeof(m_szSvrName)-1,"天与地内部区");
	}
	if (m_szmanageip[0]==0){
		config.readstr("manageip",m_szmanageip,sizeof(m_szmanageip)-1,"127.0.0.1");
	}
	m_manageport=config.readvalue("manageport",(WORD)5000);
	m_boConnectManage=(config.readvalue("openmanage",(BYTE)1)==1);
	m_boAutoStart=(config.readvalue("autostart",(BYTE)0)==1);
	m_niocpworkthreadcount=safe_min(config.readvalue("iocpworks",(int)0),36);

	m_newManMapID = config.readvalue("NewManMap",0);
	m_newManX = config.readvalue("NewManX",0);
	m_newManY = config.readvalue("NewManY",0);

	if (m_svridx==0){
		m_svridx = config.readvalue("svrid",(WORD)0,true) & 0x7fff;
		if (m_svridx==0)
		{
			g_logger.forceLog( zLogger::zFATAL, "m_svridx == 0 return" );
			return false;
		}
	}
	m_Svr2SvrLoginCmd.svr_id=m_svridx;
	return true;
}
//------------------------------------------------------------------------
bool DBService::LoadSvrConfig(CSqlClientHandle* sqlc)
{
	g_logger.forceLog( zLogger::zFATAL, "LoadSvrConfig" );

	FUNCTION_BEGIN;
	if (!sqlc) 
	{
		return false;
	}
	char extip_port[2048] = { 0 };
	char dburl[2048] = { 0 };
	dbCol serverinfo_define[] = { 
		{_DBC_SO_("gametype", DB_WORD, stServerInfo, wgame_type)}, 
		{_DBC_SO_("zoneid", DB_WORD, stServerInfo, wzoneid)}, 
		{_DBC_SO_("zonename", DB_STR, stServerInfo, szZoneName)}, 
		{_DBC_SO_("id", DB_WORD, stServerInfo, svr_id)}, 
		{_DBC_SO_("type", DB_WORD, stServerInfo, svr_type)}, 
		{_DBC_SO_("name", DB_STR, stServerInfo, szName)}, 
		{_DBC_SO_("ip", DB_STR, stServerInfo, szIp)}, 
		{_DBC_SO_("gatewayport", DB_WORD, stServerInfo, GatewayPort)}, 
		{_DBC_SO_("dbport", DB_WORD, stServerInfo, DBPort)}, 
		{_DBC_SO_("gameport", DB_WORD, stServerInfo, GamePort)}, 
		{_DBC_SO_("subsvrport", DB_WORD, stServerInfo, SubsvrPort)}, 
		{_DBC_SA_("extip_port", DB_STR, extip_port)},	//对客户端开放的 IP 端口
		{_DBC_SA_("dburl", DB_STR, dburl)}, 
		{_DBC_MO_NULL_(stServerInfo)}, 
	};
	bool boret = false;
	int ncount = sqlc->getCount(DB_CONFIG_TBL," deleted=0 ");
	if (ncount > 0) 
	{
		ZSTACK_ALLOCA(stServerInfo *, info, (ncount + 1));
		int ncur = sqlc->execSelectSql(vformat("select top 1 * from "DB_CONFIG_TBL" where id=%u and type=%u and deleted=0 ", m_svridx, m_svrtype), serverinfo_define, (unsigned char*) info);
		if (ncur == 1) 
		{
			strcpy_s(m_szLocalIp, sizeof(m_szLocalIp) - 1, info[0].szIp);
			m_gatewayport= info[0].GatewayPort;
			m_dbport = info[0].DBPort;
			m_gameport = info[0].GamePort;
			if (m_TcpServices.put(&m_iocp, m_gameport, "游戏服务器 数据交换端口", _GAMESVR_CONN_, 0, m_szLocalIp) != 0) {
				g_logger.error("游戏服务器 数据交换端口 ( %s : %d ) 监听失败...", m_szLocalIp, m_gameport);
				return false;
			}
			CEasyStrParse parse1, parse2;
			do {
				AILOCKT(cfg_lock);
				parse1.SetParseStr(dburl, "\x9, ;", "''", NULL);
				for (int i = 0; i < parse1.ParamCount(); i++) {
					parse2.SetParseStrEx(parse1[i], "='", "''",NULL);
					if (parse2.ParamCount() == 4) {
						int nmaxdb=atoi(parse2[1]);
						nmaxdb=(nmaxdb==0)?16:nmaxdb;
						nmaxdb=safe_max(nmaxdb,1);

						stUrlInfo ui(atoi(parse2[0]), parse2[3],atoi(parse2[2]) != 0,nmaxdb);
						if (!ui.geturlerror()) {
							if (!m_datadb.putURL(&ui)) {
								g_logger.error("账号 数据库 ( %s ) 连接失败...", parse1[i]);
								g_logger.forceLog( zLogger::zFATAL, "账号 数据库 ( %s ) 连接失败...", parse1[i] );
								return false;
							}
							DWORD hashcode = ui.hashcode;
						} else {
							g_logger.error("账号 数据库 ( %s ) 连接失败...", parse1[i]);
							g_logger.forceLog( zLogger::zFATAL, "账号 数据库 ( %s ) 连接失败...", parse1[i] );
							return false;
						}
					} else 
					{
						g_logger.error("账号 数据库连接字符串 ( %s ) 解析失败...", parse1[i]);
						g_logger.forceLog( zLogger::zFATAL, "账号 数据库连接字符串 ( %s ) 解析失败...", parse1[i] );
						return false;
					}
				}
			}while(false);
			boret = true;
		}
		if (boret) {
			unsigned int idx = 0;
			ncount = sqlc->execSelectSql(vformat("select top %d * from "DB_CONFIG_TBL" where id>0 and id<>%u and deleted=0 ", ncount, m_svridx), serverinfo_define, (unsigned char *) info);
			if (ncount > 0) {
				do{
					AILOCKT(cfg_lock);
					m_svrlistmap.clear();
				} while (false);
				if (boret) {
					for (int i = 0; i < ncount; i++) {
						if (info[i].svr_type == _LOGINSVR_TYPE || info[i].svr_type == _GAMESVR_TYPE) {
							do{
								AILOCKT(cfg_lock);
								m_svrlistmap[info[i].svr_marking] = info[i];
							} while (false);
						}
						if ( info[i].svr_type == _LOGSVR_TYPE && info[i].SubsvrPort > 0 ){
							m_logsvrconnecter = new CLogSvrConnecter(&m_iocp, &info[i]);
							if (m_logsvrconnecter) {
								m_TcpConnters.put(m_logsvrconnecter,info[i].szIp,info[i].SubsvrPort, "日志服务器连接", _LOG_SVR_CONNETER_, idx);
								idx++;
							}else{
								return false;
							}
						}
						if (info[i].svr_type == _LOGINSVR_TYPE && info[i].DBPort > 0) {
							CLoginSvrDBConnecter* pconnecter = new CLoginSvrDBConnecter(&m_iocp, &info[i]);
							if (pconnecter) {
								do{
									AILOCKT(m_loginsvrconnter);
									m_loginsvrconnter.insert(pconnecter);
								} while (false);
								m_TcpConnters.put(pconnecter,info[i].szIp,info[i].DBPort, "帐号服务器连接", _LOGIN_SVR_CONNETER_, idx);
								idx++;
							}
						} else if ( info[i].svr_type == _CHECKNAMESVR_TYPE && info[i].SubsvrPort > 0 && m_checknamesvrconnecter==NULL && m_boopencheckname){
							m_checknamesvrconnecter = new CCheckNameSvrConnecter(&m_iocp, &info[i]);
							if (m_checknamesvrconnecter) {
								m_TcpConnters.put(m_checknamesvrconnecter,info[i].szIp,info[i].SubsvrPort, "全局名字重复性检查服务器连接", _CHECKNAME_SVR_CONNETER_, idx);
								idx++;
							}else{
								return false;
							}
						}
					}
				}
			}
		}
	}

	g_logger.forceLog( zLogger::zFATAL, "LoadSvrConfig END ret = %d" , boret );
	return boret;
}
//------------------------------------------------------------------------
bool DBService::LoadSysParam(CSqlClientHandle* sqlc)
{
	FUNCTION_BEGIN;
	if (!sqlc) {
		return false;
	}

	m_Svr2SvrLoginCmd.wgame_type=0;
	m_Svr2SvrLoginCmd.wzoneid=m_nZoneid;
	strcpy_s(m_Svr2SvrLoginCmd.szZoneName,sizeof(m_Svr2SvrLoginCmd.szZoneName)-1,m_szZoneName);

	return true;
}
//------------------------------------------------------------------------
bool DBService::sendCmd2LoginSvrByAccount(const char* szAccount,void* pbuf, unsigned int nsize, int zliblevel ){
	CLoginSvrDBConnecter* loginsvr=NULL;
	do{
		AILOCKT(m_loginsvrconnter);
		LoginServerHashCodeMap::iterator it=m_loginsvrconnter_hashcode.find( str_dbhashcode(szAccount,128,m_accountdbcount,m_accounttblcount1db,m_accountalltblcount) );
		if (it!=m_loginsvrconnter_hashcode.end()){
			loginsvr=it->second;
			if (loginsvr && loginsvr->IsConnected() && !loginsvr->isTerminate()){
				return loginsvr->sendcmd(pbuf,nsize,zliblevel);
			}
		}
	} while (false);
	return false;
}
//------------------------------------------------------------------------
CLoginSvrDBConnecter* DBService::getLoginsvrByAccount(const char* szAccount)
{
	CLoginSvrDBConnecter* loginsvr=NULL;
	do{
		AILOCKT(m_loginsvrconnter);
		LoginServerHashCodeMap::iterator it=m_loginsvrconnter_hashcode.find( str_dbhashcode(szAccount,128,m_accountdbcount,m_accounttblcount1db,m_accountalltblcount) );
		if (it!=m_loginsvrconnter_hashcode.end()){
			loginsvr=it->second;
			if (loginsvr && loginsvr->IsConnected() && !loginsvr->isTerminate()){
				return loginsvr;
			}
		}
	} while (false);
	return NULL;
}
//------------------------------------------------------------------------
bool DBService::OnInit()
{
	LoadLocalConfig();
	if (m_boAutoStart){
		OnStartService();
	}
	return true;
}
//------------------------------------------------------------------------
void DBService::OnIdle()
{
	static time_t lastshow=time(NULL);
	if (time(NULL)>lastshow){
		SetStatus(0,"type: %d(id: %d) %d:%s",m_svrtype,m_svridx,m_nZoneid,m_szZoneName);
		SetStatus(1,"%u/%u:%u:%u(%u:%u:%u)",m_iocp.GetClientCount(),m_iocp.GetAccepterCount(),
			m_iocp.GetConnecterCount(),m_iocp.GetWaitRecycleCount(),	0,0,0);
		SetStatus(2,"%u: %s",::GetCurrentProcessId(),CThreadMonitor::getme().m_szThreadFlag);

		static time_t static_clear_tick=time(NULL)+60*5;
		if (time(NULL)>static_clear_tick){
			static_clear_tick=time(NULL)+60*5;
		}
		if (time(NULL)>m_timeshutdown && m_timeshutdown!=0){
			g_logger.forceLog(zLogger::zINFO,"服务器执行定时关闭任务： %s",timetostr(m_timeshutdown));
			m_timeshutdown=0;
			m_forceclose=true;
			Close();
		}
		lastshow=lastshow+1;
	}
	Sleep(50);
}
//------------------------------------------------------------------------
void DBService::OnQueryClose(bool& boClose)
{
	if (m_boStartService){boClose=false;}
	else {return;}
	if (m_boshutdown){return;}
	this->OnStopService();
}
//------------------------------------------------------------------------
void DBService::OnUninit()
{

}
//------------------------------------------------------------------------
void DBService::OnStartService(){
	StartService();
	if (m_boStartService){EnableCtrl( false );}
}
//------------------------------------------------------------------------
void DBService::OnStopService(){
	if (m_forceclose || (MessageBox(0,"你确定要关闭游戏 数据库服务器 么?","警告",MB_OKCANCEL | MB_DEFBUTTON2)==IDOK)){
		if (!m_forceclose){g_logger.forceLog(zLogger::zINFO,"服务器执行手动关闭任务");}
		ShutDown();
		if (!m_boStartService){EnableCtrl( true );}
		this->Close();
	}
}
//------------------------------------------------------------------------
void DBService::OnConfiguration()
{
	SetLog(0,"重新加载配置文件...");
}
//------------------------------------------------------------------------
long DBService::OnTimer( int nTimerID ){
	return 0;
}
//------------------------------------------------------------------------
fnSaveGMOrder g_SaveGMOrder=NULL;
//------------------------------------------------------------------------
bool DBService::OnCommand( char* szCmd )
{
	g_SaveGMOrder(szCmd);
	g_logger.forceLog(zLogger::zINFO,"服务器管理员执行GM命令 %s",szCmd);
	return true;
}
//------------------------------------------------------------------------
long DBService::OnCommand( int nCmdID )
{
	switch ( nCmdID )
	{
	case IDM_STARTSERVICE:	OnStartService();	break;
	case IDM_STOPSERVICE:	OnStopService();	break;
	case IDM_CONFIGURATION:	OnConfiguration();	break;
	case IDM_CLEARLOG:		OnClearLog();		break;
	case IDM_DEBUGINFO:		RefSvrRunInfo();	break;
	case IDM_EXIT:			OnExit();			break;
	}
	return 0;
}
//------------------------------------------------------------------------
bool DBService::CreateToolbar()
{
	TBBUTTON tbBtns[] = 
	{
		{0, IDM_STARTSERVICE, TBSTATE_ENABLED, TBSTYLE_BUTTON},
		{1, IDM_STOPSERVICE,  TBSTATE_ENABLED, TBSTYLE_BUTTON},
	};

	int nBtnCnt = sizeof( tbBtns ) / sizeof( tbBtns[0] );

	m_hWndToolbar = CreateToolbarEx( m_hWnd, WS_CHILD | WS_VISIBLE | WS_BORDER,
		IDC_TOOLBAR, nBtnCnt, m_hInstance, IDB_TOOLBAR,
		(LPCTBBUTTON) &tbBtns, nBtnCnt, 16, 16, 16, 16, sizeof( TBBUTTON ) );


	m_hCmdEdit = CreateWindow( WC_EDIT, "",WS_CHILD| WS_VISIBLE | ES_WANTRETURN,
		nBtnCnt* (16+8)+16, 3, m_nwidth-(nBtnCnt* (16+8)+16), 19, m_hWndToolbar, 0, m_hInstance, this );

	INIT_AUTOCOMPLETE(m_hCmdEdit,"AutoComplete.dll","InitAutoComplete","SaveGMOrder",".\\cmdlist\\dbsvrcmd.txt",g_SaveGMOrder,g_AutoCompleteh);
	SetWindowLong( m_hCmdEdit, GWL_USERDATA,(long)this);
	m_EditMsgProc=(WNDPROC)SetWindowLong (m_hCmdEdit, GWL_WNDPROC, (long)WinProc);
	return m_hWndToolbar ? true : false;
}
//------------------------------------------------------------------------
bool DBService::Init( HINSTANCE hInstance )
{
	m_hInstance = hInstance;

	InitCommonControls();

	if ( !CreateWnd() || !CreateToolbar() || !CreateList() || !CreateStatus() )
		return false;

	int   pos[   4   ]={   300,   480,  620,  -1   };     //   100,   200,   300   为间隔   
	::SendMessage(   m_hWndStatus,   SB_SETPARTS, (WPARAM)4, (LPARAM)pos   );   
	EnableCtrl( true );
	ShowWindow( m_hWnd, SW_SHOWDEFAULT );

	if ( !OnInit() )
		return false;
	return true;
}
//------------------------------------------------------------------------
void DBService::OnClearLog(){
	__super::OnClearLog();
}

void DBService::EnableCtrl( bool bEnableStart )
{
	HMENU hMenu = GetSubMenu( GetMenu( m_hWnd ), 0 );

	if ( bEnableStart ){
		EnableMenuItem( hMenu, IDM_STARTSERVICE,	MF_ENABLED | MF_BYCOMMAND );
		EnableMenuItem( hMenu, IDM_STOPSERVICE,		MF_GRAYED  | MF_BYCOMMAND );
		EnableMenuItem( hMenu, IDM_CONFIGURATION,	MF_ENABLED | MF_BYCOMMAND );

		SendMessage( m_hWndToolbar, TB_SETSTATE, IDM_STARTSERVICE, 
			(LPARAM) MAKELONG( TBSTATE_ENABLED, 0 ) );
		SendMessage( m_hWndToolbar, TB_SETSTATE, IDM_STOPSERVICE,
			(LPARAM) MAKELONG( TBSTATE_INDETERMINATE, 0 ) );
	}else{
		EnableMenuItem( hMenu, IDM_STARTSERVICE,	MF_GRAYED  | MF_BYCOMMAND );
		EnableMenuItem( hMenu, IDM_STOPSERVICE,		MF_ENABLED | MF_BYCOMMAND );
		EnableMenuItem( hMenu, IDM_CONFIGURATION,	MF_GRAYED  | MF_BYCOMMAND );

		SendMessage( m_hWndToolbar, TB_SETSTATE, IDM_STARTSERVICE, 
			(LPARAM) MAKELONG( TBSTATE_INDETERMINATE, 0 ) );
		SendMessage( m_hWndToolbar, TB_SETSTATE, IDM_STOPSERVICE,
			(LPARAM) MAKELONG( TBSTATE_ENABLED, 0 ) );
	}
}
//------------------------------------------------------------------------
static CIntLock msglock;
//------------------------------------------------------------------------
void __stdcall ShowLogFunc(zLogger::zLevel& level,const char* logtime,const char* msg){	
	if (msg) {
		DBService::getMe().SetLog(level.showcolor,msg);
	}
}
//------------------------------------------------------------------------
void OnCreateInstance(CWndBase *&pWnd)
{
	OleInitialize(NULL);
	pWnd=DBService::instance();

	CXMLConfigParse config;
	config.InitConfig();
	int nshowlvl=config.readvalue("showlvl",(int)5);
	int nloglvl=config.readvalue("writelvl",(int)3);
	char szlogpath[MAX_PATH];
	config.readstr("logpath",szlogpath,sizeof(szlogpath)-1,vformat(".\\log\\dbsvr%d_log\\",DBService::getMe().m_Svr2SvrLoginCmd.svr_id),true);

	g_logger.setLevel(nloglvl, nshowlvl);
	g_logger.SetLocalFileBasePath(szlogpath);
	g_logger.SetShowLogFunc(ShowLogFunc);
}
//------------------------------------------------------------------------
int OnDestroyInstance( CWndBase *pWnd ){
	if ( pWnd ){
		DBService::delMe();
		OleUninitialize();
		pWnd = NULL;
	}
	return 0;
}
//-----------------------------------------------------------------------