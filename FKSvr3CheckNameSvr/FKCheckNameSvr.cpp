/**
*	created:		2013-4-8   13:12
*	filename: 		FKCheckNameSvr
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKCheckNameSvr.h"
#include "../FKSvr3Common/FKCommonInclude.h"
#include <malloc.h>
#include <Ole2.h>
#include <stdio.h>
#include "Res/resource.h"
//------------------------------------------------------------------------
#pragma warning(disable:4239)		//使用了非标准扩展 : “参数” : 从“CCheckNameService::RecycleThreadCallBack::stcheckSession”转换到“zHashManagerBase<valueT,e1>::removeValue_Pred_Base &”
//------------------------------------------------------------------------
#define _SUBSVR_CONN_				0
#define _STARTSERVICE_LOG_			"FKCheckNameSvr 开始启动服务..."
//------------------------------------------------------------------------
CLD_IocpClientSocket* CCheckNameTcpServices::CreateIocpClient(CMIocpTcpAccepters::CSubAccepter* pAccepter, SOCKET s)
{
	FUNCTION_BEGIN;
	CCheckNameService* checknamesvr=CCheckNameService::instance();
	if (checknamesvr->m_boshutdown|| !checknamesvr->m_boStartService){
		return NULL;
	}
	switch (pAccepter->gettype())
	{
	case _SUBSVR_CONN_:
		{
			svrinfomapiter it;
			bool bofind = false;	
			char szremoterip[32];
			CLD_Socket::GetRemoteAddress(s,szremoterip,sizeof(szremoterip));
			do{
				AILOCKT(checknamesvr->cfg_lock);
				for (it = checknamesvr->m_svrlistmap.begin(); it != checknamesvr->m_svrlistmap.end(); it++) {
					stServerInfo* psvr = &it->second;
					if ( stricmp(psvr->szIp, szremoterip) == 0 && (psvr->svr_type==_SUPERGAMESVR_TYPE || psvr->svr_type==_GAMESVR_TYPE || psvr->svr_type==_DBSVR_TYPE || psvr->svr_type==_LOGINSVR_TYPE) ) {
						bofind = true;
						break;
					}
				}
			}while (false);
			if (bofind) {
				CSubSvrSession* pqpsocket = new CSubSvrSession(pAccepter, s);
				AILOCKT(checknamesvr->m_gamesvrsession);
				checknamesvr->m_gamesvrsession.insert(pqpsocket);
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
void CCheckNameTcpServices::OnIocpClientConnect(CLD_Socket* Socket)
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
void CCheckNameTcpServices::OnClientDisconnect(CLD_Socket* Socket)
{
	FUNCTION_BEGIN;
	CMIocpTcpAccepters::CSubAccepter* pAccepter = (CMIocpTcpAccepters::CSubAccepter*) ((CLD_IocpClientSocket*) Socket)->GetAccepter();
	switch (pAccepter->gettype())
	{
	case _SUBSVR_CONN_:
		{
			CCheckNameService* checknamesvr=CCheckNameService::instance();
			CSubSvrSession* pgamesvr=(CSubSvrSession*)Socket;
			do {
				AILOCKT(checknamesvr->m_gamesvrsession);
				checknamesvr->m_gamesvrsession.erase(pgamesvr);
			}while(false);
		}
		break;
	}
	g_logger.debug("%s:%d(%s) 连接断开...", Socket->GetRemoteAddress(), Socket->GetRemotePort(), pAccepter->getdis());
}
//------------------------------------------------------------------------
void CCheckNameTcpServices::OnClientError(CLD_Socket* Socket, CLD_TErrorEvent ErrEvent, OUT int& nErrCode, char* sErrMsg)
{
	FUNCTION_BEGIN;
	g_logger.debug("%s:%d 连接异常(%d->%s)...", Socket->GetRemoteAddress(), Socket->GetRemotePort(), nErrCode, sErrMsg);
	CMIocpTcpAccepters::CSubAccepter* pAccepter = (CMIocpTcpAccepters::CSubAccepter*) ((CLD_IocpClientSocket*) Socket)->GetAccepter();
	if (nErrCode!=0){
		switch (pAccepter->gettype())
		{
		case _SUBSVR_CONN_:
			{
				((CSubSvrSession*)Socket)->Terminate();
			}
			break;
		}
		nErrCode=0;
	}
}
//------------------------------------------------------------------------
CCheckNameService::CCheckNameService()
		:CWndBase(WND_WIDTH,WND_HEIGHT*2,"Robots")
{
	FUNCTION_BEGIN;
	m_nZoneid = 0;
	m_timeshutdown=0;
	m_forceclose=false;
	m_niocpworkthreadcount=0;
	m_boStartService = false;
	m_boshutdown=false;
	m_svridx = 0;
	m_nTradeid=0;
	m_szsvrcfgdburl[0]=0;
	m_szgamesvrparamdburl[0]=0;
	m_szSvrName[0]=0;
	m_szZoneName[0]=0;
	m_nZoneid=0;
	m_szmanageip[0]=0;
	m_manageport=5000;
	m_boConnectManage=false;
	m_msgProcessThread=NULL;
	m_LogicProcessThread=NULL;
	m_dwUserOnlyId=0;
	m_dwAccountOnlyId=0;
	m_dwStrOnlyId=0;
	m_iocp.SetRecycleThreadCallBack(RecycleThreadCallBackProxy, this);
	m_Svr2SvrLoginCmd.svr_id=m_svridx;
	m_Svr2SvrLoginCmd.svr_type=m_svrtype;

	LoadLocalConfig();
	char szTemp[MAX_PATH];
	char szruntime[20];
	timetostr(time(NULL),szruntime,20);
	sprintf_s(szTemp,MAX_PATH,"CNS[%s : CheckNameSvr](BuildTime: %s)-(RunTime: %s)",m_szSvrName,__BUILD_DATE_TIME__,szruntime);
	SetTitle(szTemp);
}
//------------------------------------------------------------------------
HMODULE g_AutoCompleteh=0;
CCheckNameService::~CCheckNameService(){
	if (g_AutoCompleteh){
		FreeLibrary(g_AutoCompleteh);
		g_AutoCompleteh=0;
	}
}
//------------------------------------------------------------------------
void CCheckNameService::RecycleThreadCallBack()
{
	FUNCTION_BEGIN;
	FUNCTION_WRAPPER(true,NULL);

	time_t curtime = time(NULL);
	if (m_boStartService) 
	{
		static time_t s_timeAction=time(NULL)+2;
		if (curtime > s_timeAction && !m_boshutdown) 
		{
			m_TcpConnters.timeAction();
			s_timeAction = time(NULL) + 4;
		}
		static time_t s_timeSaveSvrParam=time(NULL)+60*1;
		curtime = time(NULL);
		if (curtime > s_timeSaveSvrParam && !m_boshutdown)
		{
			SaveServerParam( 0 ,3000 );
			s_timeSaveSvrParam=time(NULL)+60*1;
		}
	}
}
//------------------------------------------------------------------------
DWORD WINAPI OnExceptionBeginCallBack(LPEXCEPTION_POINTERS ExceptionInfo)
{
	CCheckNameService* psvr=CCheckNameService::instance_readonly();
	if (psvr && psvr->m_boStartService==true){ psvr->SaveServerParam(0,100); }
	return 0;
}
//------------------------------------------------------------------------
void CCheckNameService::StartService()
{
	FUNCTION_BEGIN;
	if (!g_onexceptionbegincallback){ 
		minidump_type=minidump_type | MiniDumpWithPrivateReadWriteMemory | MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithProcessThreadData;
		g_onexceptionbegincallback=&OnExceptionBeginCallBack;	
	}
	if (m_boStartService || !LoadLocalConfig()) 
		return;

	g_logger.forceLog(zLogger::zINFO,_STARTSERVICE_LOG_);
	stUrlInfo cfgui(0, m_szsvrcfgdburl, true);
	CSqlClientHandle* cfgsqlc = cfgui.NewSqlClientHandle();
	if (cfgsqlc)
	{
		if (cfgsqlc->setHandle())
		{
			m_iocp.Init(m_niocpworkthreadcount);
			m_iocp.SetIocpCallBack(true, 500 , 800 , 500);
			m_svrdatadb.putURL(_SVR_PARAM_DBHASHCODE_,m_szgamesvrparamdburl,false,16);

			if (LoadSvrConfig(cfgsqlc) && LoadServerParam()) 
			{
				LoadSysParam(cfgsqlc);

				m_boStartService = false;
				m_msgProcessThread=CThreadFactory::CreateBindClass(this,&CCheckNameService::SimpleMsgProcessThread,(void*)NULL);
				if (m_msgProcessThread)
				{
					m_msgProcessThread->Start(false);
					m_LogicProcessThread=CThreadFactory::CreateBindClass(this,&CCheckNameService::LogicProcessThread,(void*)NULL);
					if (m_LogicProcessThread)
					{
						m_LogicProcessThread->Start(false);
						m_boStartService = true;
					}
				}
				if (!m_boStartService)
				{
					m_boStartService = true;
					StopService();
					m_boStartService = false;
				}
			} 
			else 
			{
				m_boStartService = true;
				StopService();
				m_boStartService = false;
			}
		}
		cfgsqlc->unsetHandle();
		SAFE_DELETE(cfgsqlc);
	}
	if (m_boStartService)
	{
		g_logger.forceLog(zLogger::zINFO,"启动服务成功...");
	} else {
		g_logger.forceLog(zLogger::zINFO,"启动服务失败...");
	}
}
//------------------------------------------------------------------------
void CCheckNameService::RefSvrRunInfo(){
	FUNCTION_WRAPPER_RESET(true);
	if (!m_boshutdown){
		CThreadMonitor::getme().DebugPrintAllThreadDebufInfo();
	}
}
//------------------------------------------------------------------------
void CCheckNameService::ShutDown()
{
	FUNCTION_BEGIN;
	if (m_boshutdown){return;}
	if (!m_boStartService){return;}
	m_boshutdown=true;
	CLD_Accepter* pservice=m_TcpServices.getservice(_SUBSVR_CONN_,0);
	if (pservice && pservice->GetClientsCount()==0)
	{
		SaveServerParam(0,0);
	}

	StopService();
	SaveServerParam(0,0);
	this->Processmsg();
	Sleep(2000);
}
//------------------------------------------------------------------------
void CCheckNameService::StopService()
{
	FUNCTION_BEGIN;
	if (!m_boStartService) {
		return;
	}

	g_logger.forceLog(zLogger::zINFO,"开始停止服务...");

	do {
		//AILOCKT(m_toplock);
		if (m_LogicProcessThread){	m_LogicProcessThread->Suspend();}
		if (m_LogicProcessThread){	m_LogicProcessThread->Resume(); }
	}while(false);	

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
	clearpmap2(m_checkstrmap);
	m_boStartService = false;
	g_logger.forceLog(zLogger::zINFO,"停止服务成功...");
}
//------------------------------------------------------------------------
bool CCheckNameService::LoadLocalConfig()
{
	FUNCTION_BEGIN;
	CXMLConfigParse config;
	config.InitConfig();
	if (m_szsvrcfgdburl[0]==0)
	{
		config.readstr("svrcfg",m_szsvrcfgdburl,sizeof(m_szsvrcfgdburl)-1,"msmdb://game:k3434yiuasdh848@127.0.0.1:0/\".\\db\\mydb_svrcfg.mdb\"");
		if (m_szsvrcfgdburl[0]==0)
		{
			return false;
		}
	}
	if (m_szgamesvrparamdburl[0]==0)
	{
		config.readstr("chksvrparam",m_szgamesvrparamdburl,sizeof(m_szgamesvrparamdburl)-1,"msmdb://game:k3434yiuasdh848@127.0.0.1:0/\".\\db\\mydb_svrparam.mdb\"");
		if (m_szgamesvrparamdburl[0]==0)
		{
			return false;
		}
	}
	if (m_szSvrName[0]==0)
	{
		config.readstr("svrname",m_szSvrName,sizeof(m_szSvrName)-1,"测试区");
	}
	if (m_szmanageip[0]==0)
	{
		config.readstr("manageip",m_szmanageip,sizeof(m_szmanageip)-1,"127.0.0.1");
	}
	m_manageport=config.readvalue("manageport",(WORD)5000);
	m_boConnectManage=(config.readvalue("openmanage",(BYTE)1)==1);
	m_boAutoStart=(config.readvalue("autostart",(BYTE)0)==1);
	m_niocpworkthreadcount=safe_min(config.readvalue("iocpworks",(int)0),36);

	if (m_svridx==0)
	{
		m_svridx = config.readvalue("svrid",(WORD)0,true) & 0x7fff;
		if (m_svridx==0)
		{
			return false;
		}
	}
	m_Svr2SvrLoginCmd.svr_id=m_svridx;
	return true;
}
//------------------------------------------------------------------------
bool CCheckNameService::LoadSvrConfig(CSqlClientHandle* sqlc)
{
	FUNCTION_BEGIN;
	if (!sqlc) {
		return false;
	}
	char extip_port[2048] = { 0 };
	char dburl[2048] = { 0 };
	WORD wtradeid=-1;
	dbCol serverinfo_define[] = 
	{ 
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
		{_DBC_SA_("extip_port", DB_STR, extip_port)},		//对客户端开放的 IP 端口
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
			m_subsvrport= info[0].SubsvrPort;
			if (m_TcpServices.put(&m_iocp, m_subsvrport, "重复性校验交换端口", _SUBSVR_CONN_, 0, m_szLocalIp) != 0) 
			{
				g_logger.error("重复性校验交换端口 ( %s : %d ) 监听失败...", m_szLocalIp, m_subsvrport);
				return false;
			}
			CEasyStrParse parse1, parse2;
			do {
				AILOCKT(cfg_lock);
				parse1.SetParseStr(dburl, "\x9, ;", "''", NULL);
				for (int i = 0; i < parse1.ParamCount(); i++) 
				{
					parse2.SetParseStrEx(parse1[i], "='", "''",NULL);
					if (parse2.ParamCount() == 4) 
					{
						int nmaxdb=atoi(parse2[1]);
						nmaxdb=(nmaxdb==0)?16:nmaxdb;
						nmaxdb=safe_max(nmaxdb,1);

						stUrlInfo ui(atoi(parse2[0]), parse2[3],atoi(parse2[2]) != 0,nmaxdb);
						if (!ui.geturlerror()) 
						{
							if (!m_datadb.putURL(&ui)) 
							{
								g_logger.error("重复性校验 数据库 ( %s ) 连接失败...", parse1[i]);
								return false;
							}
							DWORD hashcode = ui.hashcode;
						} 
						else 
						{
							g_logger.error("重复性校验 数据库 ( %s ) 连接失败...", parse1[i]);
							return false;
						}
					} 
					else 
					{
						g_logger.error("重复性校验 数据库连接字符串 ( %s ) 解析失败...", parse1[i]);
						return false;
					}
				}
			}while(false);
			boret = true;
		}
		if (boret) 
		{
			int idx=0;
			ncount = sqlc->execSelectSql(vformat("select top %d * from "DB_CONFIG_TBL" where id>0 and id<>%u and deleted=0 ", ncount, m_svridx), serverinfo_define, (unsigned char *) info);
			if (ncount > 0 ) 
			{
				do
				{
					AILOCKT(cfg_lock);
					m_svrlistmap.clear();
				} while (false);

				unsigned int idx = 0;
				if (boret) 
				{
					for (int i = 0; i < ncount; i++) 
					{
						if ( info[i].svr_type==_SUPERGAMESVR_TYPE || info[i].svr_type==_GAMESVR_TYPE || info[i].svr_type==_DBSVR_TYPE || info[i].svr_type==_LOGINSVR_TYPE ) 
						{
							do
							{
								AILOCKT(cfg_lock);
								m_svrlistmap[info[i].svr_marking] = info[i];
							} while (false);
						}
					}
				}
			}
		}
	}
	return boret;
}
//------------------------------------------------------------------------
bool CCheckNameService::LoadSysParam(CSqlClientHandle* sqlc)
{
	FUNCTION_BEGIN;
	if (!sqlc) {
		return false;
	}

	m_Svr2SvrLoginCmd.wgame_type=0;
	m_Svr2SvrLoginCmd.wzoneid=m_nZoneid;

	return true;
}
//------------------------------------------------------------------------
bool CCheckNameService::LoadServerParam(){
	FUNCTION_BEGIN;

	GETAUTOSQL(CSqlClientHandle *, sqlc, m_svrdatadb, _SVR_PARAM_DBHASHCODE_);
	if (!sqlc){ return false; }
	int tmp=0;
	dbCol svrparam_define[] = 
	{ 
		{_DBC_SA_("UserOnlyID",		DB_DWORD, m_dwUserOnlyId)}, 
		{_DBC_SA_("AccountOnlyID",	DB_DWORD, m_dwAccountOnlyId)}, 
		{_DBC_SA_("StrOnlyID",		DB_DWORD, m_dwStrOnlyId)}, 
		{_DBC_SA_("PlatformID",		DB_DWORD, m_nTradeid )}, 
		{_DBC_SA_("ZoneID",			DB_DWORD, m_nZoneid )}, 
		{_DBC_NULL_},  
	};

	const char * TempFormName = "逗你玩儿";

	int ncur = sqlc->execSelectSql( vformat("select top 1 * from "DB_CHKSERVERPARAM_TBL" where id=%u and type=%u ", m_svridx, m_svrtype), svrparam_define, (unsigned char*) &tmp );
	g_logger.forceLog(zLogger::zINFO,"运营平台[ %s ] ID = %d",TempFormName, m_nTradeid );

	if (ncur!=1){	return false;	}

	if (m_dwUserOnlyId>=_MAX_ONLYID_ || m_dwAccountOnlyId>=_MAX_ONLYID_ || m_dwStrOnlyId>=_MAX_ONLYID_)
	{ 
		g_logger.error("UserOnlyID,AccountOnlyID,StrOnlyID 必须都小于 %d",_MAX_ONLYID_);
		return false; 
	}
	bool boisok=true;

	if (m_dwUserOnlyId < _MIN_ONLYID_ )
	{	
		m_dwUserOnlyId=_random(100)+100;
		boisok = false;	
	}

	if ( m_dwAccountOnlyId < _MIN_ONLYID_ )
	{	
		m_dwAccountOnlyId=_random(100)+100;
		boisok=false;	
	}

	if (m_dwStrOnlyId<_MIN_ONLYID_)
	{	
		m_dwStrOnlyId=_random(100)+100;
		boisok=false;	
	}

	if (!boisok)
	{
		g_logger.error("useronlyid,accountonlyid,stronlyid 必须都大于 %d",_MIN_ONLYID_);
	}else
	{
		g_logger.forceLog(zLogger::zINFO,"useronlyid: %u   accountonlyid: %u  stronlyid: %u",m_dwUserOnlyId,m_dwAccountOnlyId,m_dwStrOnlyId);
	}
	return (boisok && SaveServerParam(0,10));
}
//------------------------------------------------------------------------
bool CCheckNameService::SaveServerParam(int naddtime,int naddid)
{
	FUNCTION_BEGIN;

	g_logger.forceLog(zLogger::zINFO,"UserOnlyID: %u   AccountOnlyID: %u  stronlyid: %u  write_useronlyid: %u   write_accountonlyid: %u  write_stronlyid: %u",
		m_dwUserOnlyId,m_dwAccountOnlyId,m_dwStrOnlyId,m_dwUserOnlyId+naddid,m_dwAccountOnlyId+naddid,m_dwStrOnlyId+naddid);

	GETAUTOSQL(CSqlClientHandle *, sqlc, m_svrdatadb, _SVR_PARAM_DBHASHCODE_);
	if (!sqlc){ return false; }
	DWORD dwUserOnlyId		= m_dwUserOnlyId+naddid;
	DWORD dwAccountOnlyId	= m_dwAccountOnlyId+naddid;
	DWORD dwStrOnlyId		= m_dwStrOnlyId+naddid;

	if( dwUserOnlyId < 40000 )
	{
		g_logger.error( " dwUserverOnlyID Error : %d" , dwUserOnlyId );
		return false;
	}

	int tmp=0;

	dbCol svrparam_define[] = 
	{ 
		{_DBC_SA_("id", DB_WORD, m_svridx)}, 
		{_DBC_SA_("type", DB_WORD, m_svrtype)}, 
		{_DBC_SA_("UserOnlyID", DB_DWORD, dwUserOnlyId)}, 
		{_DBC_SA_("AccountOnlyID", DB_DWORD, dwAccountOnlyId)}, 
		{_DBC_SA_("StrOnlyID", DB_DWORD, dwStrOnlyId)}, 
		{_DBC_SA_("PlatformID", DB_DWORD, CCheckNameService::getMe().m_nTradeid)}, 
		{_DBC_SA_("ZoneID", DB_DWORD, m_nZoneid)},
		{_DBC_NULL_},  
	};

	int nret=sqlc->execUpdate(DB_CHKSERVERPARAM_TBL, svrparam_define, (unsigned char *) &tmp, vformat(" id=%u and type=%u ", m_svridx, m_svrtype));
	if (nret > 0) 
	{
		return true;
	}else
	{
		nret=sqlc->execInsert(DB_CHKSERVERPARAM_TBL, svrparam_define, (unsigned char *) &tmp);
		if (nret > 0) 
		{
			return true;
		}
	}
	return false;
}
//------------------------------------------------------------------------
bool CCheckNameService::OnInit()
{	
	LoadLocalConfig();
	if (m_boAutoStart){
		OnStartService();
	}
	return true;
}
//------------------------------------------------------------------------
void CCheckNameService::OnIdle()
{
	static time_t lastshow=time(NULL);
	if (time(NULL)>lastshow){
		SetStatus(0,"type: %d(id: %d) %d:%s",m_svrtype,m_svridx,0,"");
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
void CCheckNameService::OnQueryClose(bool& boClose)
{
	if (m_boStartService){boClose=false;}
	else {return;}
	if (m_boshutdown){return;}
	this->OnStopService();
}
//------------------------------------------------------------------------
void CCheckNameService::OnUninit()
{

}
//------------------------------------------------------------------------
void CCheckNameService::OnStartService(){
	StartService();
	if (m_boStartService){EnableCtrl( false );}
}
//------------------------------------------------------------------------
void CCheckNameService::OnStopService(){
	if (m_forceclose || (MessageBox(0,"你确定要关闭游戏 全局字符串重复检测服务器 么?","警告",MB_OKCANCEL | MB_DEFBUTTON2)==IDOK)){
		if (!m_forceclose){g_logger.forceLog(zLogger::zINFO,"服务器执行手动关闭任务");}
		ShutDown();
		if (!m_boStartService){EnableCtrl( true );}
		this->Close();
	}
}
//------------------------------------------------------------------------
void CCheckNameService::OnConfiguration()
{
	SetLog(0,"重新加载配置文件...");
}
//------------------------------------------------------------------------
long CCheckNameService::OnTimer( int nTimerID ){
	return 0;
}
//------------------------------------------------------------------------
fnSaveGMOrder g_SaveGMOrder=NULL;
//------------------------------------------------------------------------
bool CCheckNameService::OnCommand( char* szCmd )
{
	g_SaveGMOrder(szCmd);
	g_logger.forceLog(zLogger::zINFO,"服务器管理员执行GM命令 %s",szCmd);
	return true;
}
//------------------------------------------------------------------------
long CCheckNameService::OnCommand( int nCmdID )
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
bool CCheckNameService::CreateToolbar()
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

	INIT_AUTOCOMPLETE(m_hCmdEdit,"AutoComplete.dll","InitAutoComplete","SaveGMOrder",".\\cmdlist\\checknamesvrcmd.txt",g_SaveGMOrder,g_AutoCompleteh);
	SetWindowLong( m_hCmdEdit, GWL_USERDATA,(long)this);
	m_EditMsgProc=(WNDPROC)SetWindowLong (m_hCmdEdit, GWL_WNDPROC, (long)WinProc);
	return m_hWndToolbar ? true : false;
}
//------------------------------------------------------------------------
bool CCheckNameService::Init( HINSTANCE hInstance )
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
void CCheckNameService::OnClearLog(){
	__super::OnClearLog();
}
//------------------------------------------------------------------------
void CCheckNameService::EnableCtrl( bool bEnableStart )
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
		CCheckNameService::getMe().SetLog(level.showcolor,msg);
	}
}
//------------------------------------------------------------------------
void OnCreateInstance(CWndBase *&pWnd)
{
	OleInitialize(NULL);
	pWnd=CCheckNameService::instance();

	CXMLConfigParse config;
	config.InitConfig();
	int nshowlvl=config.readvalue("showlvl",(int)5);
	int nloglvl=config.readvalue("writelvl",(int)3);
	char szlogpath[MAX_PATH];
	config.readstr("logpath",szlogpath,sizeof(szlogpath)-1,vformat(".\\log\\checknamesvr%d_log\\",CCheckNameService::getMe().m_Svr2SvrLoginCmd.svr_id),true);

	g_logger.setLevel(nloglvl, nshowlvl);
	g_logger.SetLocalFileBasePath(szlogpath);
	g_logger.SetShowLogFunc(ShowLogFunc);
}
//------------------------------------------------------------------------
int OnDestroyInstance( CWndBase *pWnd ){
	if ( pWnd ){
		CCheckNameService::delMe();
		OleUninitialize();
		pWnd = NULL;
	}
	return 0;
}
//------------------------------------------------------------------------