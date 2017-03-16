/**
*	created:		2013-4-9   17:42
*	filename: 		FKGameGatewaySvr
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKGameGatewaySvr.h"
#include "Res/resource.h"
//------------------------------------------------------------------------
#pragma warning(disable:4239)		//使用了非标准扩展 : “参数” : 从“GameGateService::RecycleThreadCallBack::stcheckSession”转换到“zHashManagerBase<valueT,e1>::removeValue_Pred_Base &”
//------------------------------------------------------------------------
#define _CLIENT_CONN_				0
#define _FLASH_AUTHORITY_CONN_		1
//------------------------------------------------------------------------
#define _SVR_CONNETER_				0
#define _SUPERGAME_SVR_CONNETER_	1
//------------------------------------------------------------------------
char		szFlashCheckStr[512]={0};
int			szFlashCheckStrLen=0;
int			nFlashCheckSvrPort=0;
//------------------------------------------------------------------------
class CFlashAuthority:public CLoopbufIocpClientSocketTask
{
public:
	DEC_OP_NEW(CFlashAuthority);

	CFlashAuthority(CLD_IocpBaseAccepter* Owner, SOCKET s):CLoopbufIocpClientSocketTask(Owner,s){};

	virtual void DoRead(){
		CLD_LoopBuf* pbuf = GetRecvBufer();
		GetIocpObj().AddBuf_Send(szFlashCheckStr,szFlashCheckStrLen);
		g_logger.debug("[发送flash权限回执]%s:%d(%s) %s...", GetRemoteAddress(), GetRemotePort(), ((CMIocpTcpAccepters::CSubAccepter*)GetAccepter())->getdis(),pbuf->pData);
	}
};
//------------------------------------------------------------------------
IMP_OP_NEW(CFlashAuthority);
//------------------------------------------------------------------------
CLD_IocpClientSocket* GateTcpServices::CreateIocpClient(CMIocpTcpAccepters::CSubAccepter* pAccepter, SOCKET s)
{
	FUNCTION_BEGIN;
	GameGateService* gateway=GameGateService::instance();
	if (gateway->m_boshutdown || !gateway->m_boStartService){
		return NULL;
	}
	switch (pAccepter->gettype())
	{
	case _FLASH_AUTHORITY_CONN_:
		{
			if (szFlashCheckStrLen>0 && szFlashCheckStr[0]!=0 && nFlashCheckSvrPort!=0){ 
				CFlashAuthority* pqpsocket = new CFlashAuthority(pAccepter, s);
				return (CLD_IocpClientSocket *) pqpsocket;
			}
		}
		break;
	case _CLIENT_CONN_:
		{
			CUserSession* pqpsocket = new CUserSession(pAccepter, s);
			pqpsocket->m_ipType=((CGameSubAccepter*)pAccepter)->m_ipinfo.type;
			AILOCKT(gateway->m_clintsession);

			gateway->m_clintsession.insert(pqpsocket);
			return (CLD_IocpClientSocket *) pqpsocket;
		}
		break;
	}
	return NULL;
}
//------------------------------------------------------------------------
void GateTcpServices::OnIocpClientConnect(CLD_Socket* Socket)
{
	FUNCTION_BEGIN;
	GameGateService::getMe().tcpstate_changed=true;
	CMIocpTcpAccepters::CSubAccepter* pAccepter = (CMIocpTcpAccepters::CSubAccepter*) ((CLD_IocpClientSocket*) Socket)->GetAccepter();
	if (pAccepter){
		g_logger.debug("%s:%d(%s) 连接成功...", Socket->GetRemoteAddress(), Socket->GetRemotePort(), pAccepter->getdis());
	}else{
		g_logger.debug("%s:%d 连接成功...", Socket->GetRemoteAddress(), Socket->GetRemotePort());
	}
}
//------------------------------------------------------------------------
void GateTcpServices::OnClientDisconnect(CLD_Socket* Socket)
{
	FUNCTION_BEGIN;
	GameGateService::getMe().tcpstate_changed=true;
	CMIocpTcpAccepters::CSubAccepter* pAccepter = (CMIocpTcpAccepters::CSubAccepter*) ((CLD_IocpClientSocket*) Socket)->GetAccepter();
	switch (pAccepter->gettype())
	{
	case _FLASH_AUTHORITY_CONN_:
		{
			((CFlashAuthority*)Socket)->Terminate();
		}
		break;
	case _CLIENT_CONN_:
		{
			GameGateService* gateway=GameGateService::instance();

			CGameGateWayUser* pGateWayUser=((CUserSession*)Socket)->m_GateWayUser;
			if (pGateWayUser && pGateWayUser->m_szPlayerNmae[0]!=0){
				g_logger.debug("玩家 %s 角色 %s 断开连接",pGateWayUser->m_szAccount,pGateWayUser->m_szPlayerNmae);
			}

			do {
				AILOCKT(gateway->m_gatewayconnter);
				pGateWayUser->notifySvrRemoveUser();
				pGateWayUser->m_Owner=NULL;
			} while (false);

			do {
				AILOCKT(gateway->m_clintsession);
				gateway->m_clintsession.erase((CUserSession*)Socket);
			} while (false);
		}
		break;
	}
	g_logger.debug("%s:%d(%s) 连接断开...", Socket->GetRemoteAddress(), Socket->GetRemotePort(), pAccepter->getdis());
}
//------------------------------------------------------------------------
void GateTcpServices::OnClientError(CLD_Socket* Socket, CLD_TErrorEvent ErrEvent, OUT int& nErrCode, char* sErrMsg)
{
	FUNCTION_BEGIN;
	GameGateService::getMe().tcpstate_changed=true;
	g_logger.debug("%s:%d 连接异常(%d->%s)...", Socket->GetRemoteAddress(), Socket->GetRemotePort(), nErrCode, sErrMsg);
	CMIocpTcpAccepters::CSubAccepter* pAccepter = (CMIocpTcpAccepters::CSubAccepter*) ((CLD_IocpClientSocket*) Socket)->GetAccepter();
	if (nErrCode!=0){
		switch (pAccepter->gettype())
		{
		case _FLASH_AUTHORITY_CONN_:
			{
				((CFlashAuthority*)Socket)->Terminate();
			}
			break;
		case _CLIENT_CONN_:
			{
				((CUserSession*)Socket)->Terminate(__FF_LINE__);
			}
			break;
		}
		nErrCode=0;
	}
}
//------------------------------------------------------------------------
GameGateService::GameGateService()
	:CWndBase(WND_WIDTH,WND_HEIGHT*2,"GameGateService_Wnd")
{
	FUNCTION_BEGIN;
	m_lastRunTick = 0;
	m_timeshutdown=0;
	m_forceclose=false;

	m_niocpworkthreadcount=0;
	m_boStartService = false;
	m_boshutdown=false;
	tcpstate_changed=false;
	m_svridx = 0;
	m_szsvrcfgdburl[0]=0;
	m_szSvrName[0]=0;
	m_szZoneName[0]=0;
	m_nZoneid=0;
	m_wgatewayowner=0;
	m_msgProcessThread=NULL;
	m_LogicProcessThread=NULL;
	m_szmanageip[0]=0;
	m_manageport=5000;
	m_boConnectManage=false;

	m_iocp.SetRecycleThreadCallBack(RecycleThreadCallBackProxy, this);

	m_Svr2SvrLoginCmd.svr_id=m_svridx;
	m_Svr2SvrLoginCmd.svr_type=m_svrtype;

	static char GateTcpStateBuf[1024 * 4];

	LoadLocalConfig();
	char szTemp[MAX_PATH];
	char szruntime[20];
	timetostr(time(NULL),szruntime,20);
	sprintf_s(szTemp,MAX_PATH,"GG[%s : GameGateWaySvr](BuildTime: %s)-(RunTime: %s)",m_szSvrName,__BUILD_DATE_TIME__,szruntime);
	SetTitle(szTemp);
}
//------------------------------------------------------------------------
HMODULE g_AutoCompleteh=0;
//------------------------------------------------------------------------
GameGateService::~GameGateService(){
	if (g_AutoCompleteh){
		FreeLibrary(g_AutoCompleteh);
		g_AutoCompleteh=0;
	}
}
//------------------------------------------------------------------------
void GameGateService::RecycleThreadCallBack()
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

		static time_t m_timeRefSvrState=time(NULL);
		static time_t m_timeLastRefTime=time(NULL);
		if (curtime > m_timeRefSvrState) {
			bool sendtcpinfo=tcpstate_changed;
			RefStateCmd();
			if ( sendtcpinfo || ((curtime-m_timeLastRefTime)>(60*5*5)) ){
				char tmpGateTcpStateBuf[1024 * 4];
				int nstatesize=0;
				Send2GameSvrs((char *) &tmpGateTcpStateBuf,nstatesize);
				m_timeLastRefTime=time(NULL);
			}
			m_timeRefSvrState = time(NULL) + 60*5;
		}
	}
}
//------------------------------------------------------------------------
bool GameGateService::Send2GameSvrs(void* pcmd,int ncmdlen){
	CSyncMap< DWORD,CGameGateWayConnecter* >::iterator it;
	CGameGateWayConnecter* pGamesvr=NULL;
	AILOCKT(m_gatewayconnter);
	for (it=m_gatewayconnter.begin();it!=m_gatewayconnter.end();it++){
		pGamesvr=it->second;
		if (pGamesvr && !pGamesvr->isTerminate() && pGamesvr->IsConnected()){
			pGamesvr->SendSvrCmd(pcmd,ncmdlen);
		}
	}
	return true;
}
//------------------------------------------------------------------------
void GameGateService::RefStateCmd()
{
	FUNCTION_BEGIN;
	if (tcpstate_changed)
	{
		AILOCKT(cfg_lock);
		static unsigned char svrtcpstatechangecmdbuf[stBasePacket::MAX_PACKET_SIZE + 512];

		do{
			AILOCKT(m_TcpServices.getall());
			GateTcpServices::Sock2IpPort_iterator it;
			GateTcpServices::CGameSubAccepter* pAccepter=NULL;
			for (it=m_TcpServices.begin();it!=m_TcpServices.end();it++){
				pAccepter=(GateTcpServices::CGameSubAccepter*)it->second;
				if (pAccepter && pAccepter->gettype()==_CLIENT_CONN_){
					pAccepter->m_ipinfo.ncount=pAccepter->GetClientsCount();
					pAccepter->m_ipinfo.state= pAccepter->IsConnected()?0:-1;
				}
			}
		}while(false);
		tcpstate_changed=false;
	}
}
//------------------------------------------------------------------------
void GameGateService::StartService()
{
	FUNCTION_BEGIN;
	if (!g_onexceptionbegincallback){ 
		minidump_type=minidump_type | MiniDumpWithPrivateReadWriteMemory | MiniDumpWithDataSegs | MiniDumpWithHandleData | MiniDumpWithProcessThreadData;
	}
	if (m_boStartService || !LoadLocalConfig()) {
		return;
	}
	g_logger.forceLog(zLogger::zINFO,"开始启动服务...");
	stUrlInfo cfgui(0, m_szsvrcfgdburl/*"msmdb://game:k3434yiuasdh848@127.0.0.1:0/\".\\db\\mydb_svrcfg.mdb\""*/, true);
	CSqlClientHandle* cfgsqlc = cfgui.NewSqlClientHandle();
	if (cfgsqlc){
		if (cfgsqlc->setHandle()){
			m_iocp.Init(m_niocpworkthreadcount);
			m_iocp.SetIocpCallBack(true, 500 , 800 , 500);
			if (LoadSvrConfig(cfgsqlc)) {
				LoadSysParam(cfgsqlc);

				m_boStartService = false;
				m_msgProcessThread=CThreadFactory::CreateBindClass(this,&GameGateService::SimpleMsgProcessThread,(void*)NULL);
				if (m_msgProcessThread){
					m_msgProcessThread->Start(false);
					m_LogicProcessThread=CThreadFactory::CreateBindClass(this,&GameGateService::LogicProcessThread,(void*)NULL);
					if (m_LogicProcessThread){
						m_LogicProcessThread->Start(false);
						m_boStartService = true;
					}
				}
				if (!m_boStartService){
					m_boStartService = true;
					StopService();
					m_boStartService = false;
				}
			} else {
				m_boStartService = true;
				StopService();
				m_boStartService = false;
			}
		}
		cfgsqlc->unsetHandle();
		SAFE_DELETE(cfgsqlc);
	}
	if (m_boStartService){
		g_logger.forceLog(zLogger::zINFO,"启动服务成功...");
		m_TcpConnters.timeAction(true);
	} else {
		g_logger.forceLog(zLogger::zINFO,"启动服务失败...");
	}
}
//------------------------------------------------------------------------
void GameGateService::RefSvrRunInfo(){
	FUNCTION_WRAPPER_RESET(true);
	if (!m_boshutdown){
		CThreadMonitor::getme().DebugPrintAllThreadDebufInfo();
	}
	g_logger.forceLog(zLogger::zINFO,"GateWayUser:%u , Session:%u",CGameGateWayUser::m_nrefcount,CUserSession::m_nrefcount);
}
//------------------------------------------------------------------------
void GameGateService::ShutDown()
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
void GameGateService::StopService()
{
	FUNCTION_BEGIN;
	if (!m_boStartService) {
		return;
	}
	g_logger.forceLog(zLogger::zINFO,"开始停止服务...");

	do {
		if (m_msgProcessThread){ m_msgProcessThread->Suspend();}
		if (m_msgProcessThread){ m_msgProcessThread->Resume();}
	}while(false);

	do{
		AILOCKT(m_gatewayconnter);
		m_gatewayconnter.clear();
	} while (false);

	do{
		AILOCKT(m_clintsession);
		m_clintsession.clear();
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

	clearsyncplist(m_waitdelgateuser);
	m_boStartService = false;
	g_logger.forceLog(zLogger::zINFO,"停止服务成功...");
}
//------------------------------------------------------------------------
bool GameGateService::LoadLocalConfig()
{
	FUNCTION_BEGIN;

	char* pLocalName = (__argc > 1) ? __argv[1] : NULL;

	CXMLConfigParse config;
	config.InitConfig(pLocalName);

	if (m_szsvrcfgdburl[0]==0){
		config.readstr("svrcfg",m_szsvrcfgdburl,sizeof(m_szsvrcfgdburl)-1,"msmdb://game:k3434yiuasdh848@127.0.0.1:0/\".\\db\\mydb_svrcfg.mdb\"");
		if (m_szsvrcfgdburl[0]==0){
			return false;
		}
	}
	if (m_szSvrName[0]==0){
		config.readstr("svrname",m_szSvrName,sizeof(m_szSvrName)-1,"测试区");
	}

	nFlashCheckSvrPort=config.readvalue("flashchecksvr",(int)0,true);
	nFlashCheckSvrPort=(nFlashCheckSvrPort!=0)?( (nFlashCheckSvrPort==1)?_FLASH_AUTHORITY_PORT_:nFlashCheckSvrPort ):0;
	if (szFlashCheckStr[0]==0 && nFlashCheckSvrPort!=0){
		config.readstr("flashauthority",szFlashCheckStr,sizeof(szFlashCheckStr)-1,_FLASH_AUTHORITY_RET_);
		szFlashCheckStrLen=strlen(szFlashCheckStr)+1;
		if ( szFlashCheckStr[0]==0 || szFlashCheckStrLen<=0 ){
			g_logger.error("flashauthority is null!");
			return false;
		}
	}
	if (m_szmanageip[0]==0){
		config.readstr("manageip",m_szmanageip,sizeof(m_szmanageip)-1,"127.0.0.1");
	}
	m_manageport=config.readvalue("manageport",(WORD)5000);
	m_boConnectManage=(config.readvalue("openmanage",(BYTE)1)==1);
	m_boAutoStart=(config.readvalue("autostart",(BYTE)0)==1);
	m_niocpworkthreadcount=safe_min(config.readvalue("iocpworks",(int)0),36);

	if (m_svridx==0){
		m_svridx = config.readvalue("svrid",(WORD)0,true) & 0x7fff;
		if (m_svridx==0){
			return false;
		}
	}
	m_Svr2SvrLoginCmd.svr_id=m_svridx;
	return true;
}
//------------------------------------------------------------------------
bool GameGateService::LoadSvrConfig(CSqlClientHandle* sqlc)
{
	FUNCTION_BEGIN;
	if (!sqlc) {
		return false;
	}

	return true;
}
//------------------------------------------------------------------------
bool GameGateService::decodetoken(DWORD loginsvr_id,stLoginToken* pSrcToken,stLoginToken* pDstToken,DWORD tock){
	FUNCTION_BEGIN;
	if (pSrcToken){
		do{
			AILOCKT(m_loginsvrenc);
			loginsvrencryptmap::iterator it=m_loginsvrenc.find(loginsvr_id);
			CEncrypt* penc=NULL;
			if (it!=m_loginsvrenc.end()){
				penc=&it->second;
			}
			if (penc) {
				stLoginToken tmp = *pSrcToken;
				if (tmp.decode(penc,tock)){
					if (!pDstToken){ pDstToken=pSrcToken; }
					CopyMemory(pDstToken, &tmp, sizeof(*pDstToken));
					return true;
				}
			}
		}while(false);
	}
	return false;
}
//------------------------------------------------------------------------
bool GameGateService::LoadSysParam(CSqlClientHandle* sqlc)
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
bool GameGateService::OnInit()
{
	LoadLocalConfig();
	if (m_boAutoStart){
		OnStartService();
	}
	return true;
}
//------------------------------------------------------------------------
void GameGateService::OnIdle()
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
			g_logger.forceLog(zLogger::zINFO,"GateWayUser:%u , Session:%u",
				CGameGateWayUser::m_nrefcount,CUserSession::m_nrefcount);
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
void GameGateService::OnQueryClose(bool& boClose)
{
	if (m_boStartService){boClose=false;}
	else {return;}
	if (m_boshutdown){return;}
	this->OnStopService();
}
//------------------------------------------------------------------------
void GameGateService::OnUninit()
{

}
//------------------------------------------------------------------------
void GameGateService::OnStartService(){
	StartService();
	if (m_boStartService){EnableCtrl( false );}
}
//------------------------------------------------------------------------
void GameGateService::OnStopService(){
	if (m_forceclose || (MessageBox(0,"你确定要关闭 游戏服务器网关 么?","警告",MB_OKCANCEL | MB_DEFBUTTON2)==IDOK)){
		if (!m_forceclose){g_logger.forceLog(zLogger::zINFO,"服务器执行手动关闭任务");}
		ShutDown();
		if (!m_boStartService){EnableCtrl( true );}
		this->Close();
	}
}
//------------------------------------------------------------------------
void GameGateService::OnConfiguration()
{
	SetLog(0,"重新加载配置文件...");
}
//------------------------------------------------------------------------
long GameGateService::OnTimer( int nTimerID ){
	return 0;
}
//------------------------------------------------------------------------
fnSaveGMOrder g_SaveGMOrder=NULL;
//------------------------------------------------------------------------
bool GameGateService::OnCommand( char* szCmd )
{
	g_SaveGMOrder(szCmd);
	g_logger.forceLog(zLogger::zINFO,"服务器管理员执行GM命令 %s",szCmd);
	return true;
}
//------------------------------------------------------------------------
long GameGateService::OnCommand( int nCmdID )
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
bool GameGateService::CreateToolbar()
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

	INIT_AUTOCOMPLETE(m_hCmdEdit,"AutoComplete.dll","InitAutoComplete","SaveGMOrder",".\\cmdlist\\gamegatecmd.txt",g_SaveGMOrder,g_AutoCompleteh);

	SetWindowLong( m_hCmdEdit, GWL_USERDATA,(long)this);
	m_EditMsgProc=(WNDPROC)SetWindowLong (m_hCmdEdit, GWL_WNDPROC, (long)WinProc);
	return m_hWndToolbar ? true : false;
}
//------------------------------------------------------------------------
bool GameGateService::Init( HINSTANCE hInstance )
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
void GameGateService::OnClearLog(){
	__super::OnClearLog();
}
//------------------------------------------------------------------------
void GameGateService::EnableCtrl( bool bEnableStart )
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
bool s_usePerformance = true;
//------------------------------------------------------------------------
void GameGateService::RunStep()
{
	if( !s_usePerformance )
		return ;

	DWORD _Now = GetTickCount();

	if( _Now - m_lastRunTick >= 50 )
	{
		m_lastRunTick = _Now;

		AILOCKT( m_clintsession );

		CSyncSet< CUserSession* >::iterator _iter;
		for( _iter = m_clintsession.begin(); _iter != m_clintsession.end(); _iter++ )
		{
			CUserSession* _Session = *_iter;
			if( _Session )
			{
				_Session->postcmds();
			}
		}
	}
}
//------------------------------------------------------------------------
static CIntLock msglock;
//------------------------------------------------------------------------
void __stdcall ShowLogFunc(zLogger::zLevel& level,const char* logtime,const char* msg){	
	if (msg) {
		GameGateService::getMe().SetLog(level.showcolor,msg);
	}
}
//------------------------------------------------------------------------
void OnCreateInstance(CWndBase *&pWnd)
{
	OleInitialize(NULL);
	pWnd=GameGateService::instance();

	CXMLConfigParse config;
	config.InitConfig();
	int nshowlvl=config.readvalue("showlvl",(int)5);
	int nloglvl=config.readvalue("writelvl",(int)3);
	char szlogpath[MAX_PATH];
	config.readstr("logpath",szlogpath,sizeof(szlogpath)-1,vformat(".\\log\\gamegateway%d_log\\",GameGateService::getMe().m_Svr2SvrLoginCmd.svr_id),true);

	g_logger.setLevel(nloglvl, nshowlvl);
	g_logger.SetLocalFileBasePath(szlogpath);
	g_logger.SetShowLogFunc(ShowLogFunc);
}
//------------------------------------------------------------------------
int OnDestroyInstance( CWndBase *pWnd ){
	if ( pWnd ){
		GameGateService::delMe();
		OleUninitialize();
		pWnd = NULL;
	}
	return 0;
}
//------------------------------------------------------------------------