/**
*	created:		2013-4-7   22:43
*	filename: 		FKWndBase
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKWndBase.h"
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>
#include "../Include/FKOutput.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include "../Include/FKMisc.h"
#include "../Include/FKLogger.h"
//------------------------------------------------------------------------
CWndBase::CWndBase(int width,int height,const char* wndClassName,bool ismainwindow)
:m_wndClassName(wndClassName),m_nwidth(width),m_nheight(height),m_ismainwindow(ismainwindow)
{
	FUNCTION_BEGIN;

	m_hWndStatus	= NULL;
	m_hWndList		= NULL;
	m_hWndToolbar	= NULL;
	m_hCmdEdit		= NULL;
	m_hWnd			= NULL;
	m_hInstance		= NULL;

	ZeroMemory(&m_pTitle,sizeof(m_pTitle));
	m_ListViewMsgProc=NULL;
	m_EditMsgProc=NULL;
	ZeroMemory(m_szStatus,sizeof(m_szStatus));

	m_listdataary.resize(MAX_LISTITEMCOUMT+1);
	m_currlistdataidx=0;

	m_logparamarray.resize(MAX_LISTITEMCOUMT+1);
	m_currlogparamidx=0;
}
//------------------------------------------------------------------------
CWndBase::~CWndBase()
{
	FUNCTION_BEGIN;
	if (m_ismainwindow){g_mainwindowhandle=0;}
	if (m_hCmdEdit!=0) { ::DestroyWindow(m_hCmdEdit);m_hCmdEdit=0;}
	if (m_hWndToolbar!=0){	::DestroyWindow(m_hWndToolbar);m_hWndToolbar=0;}
	if (m_hWndList!=0){	::DestroyWindow(m_hWndList);m_hWndList=0;}
	if (m_hWndStatus!=0){	::DestroyWindow(m_hWndStatus);m_hWndStatus=0;}
	if (m_hWnd!=0){	::DestroyWindow(m_hWnd);m_hWnd=0;}
	m_logparamarray.clear();
	m_listdataary.clear();
}
//------------------------------------------------------------------------
void CWndBase::Close()
{
	::PostMessage(m_hWnd,WM_CLOSE,0,0);
}
//------------------------------------------------------------------------
void CWndBase::Processmsg()
{
	MSG		tmpmsg;
	memset( &tmpmsg, 0, sizeof( tmpmsg ) );

	while (true){
		if ( !PeekMessage(&tmpmsg, NULL, 0, 0,PM_NOREMOVE) ){
			return;	
		}
		if ( GetMessage(&tmpmsg, NULL, 0, 0) ==0 ){
			return ;
		}

		TranslateMessage( &tmpmsg );
		DispatchMessage( &tmpmsg );
	}
}
//------------------------------------------------------------------------
int CWndBase::Run()
{
	memset( &m_msg, 0, sizeof( m_msg ) );

	while (true){
		if ( !PeekMessage(&m_msg, NULL, 0, 0,PM_NOREMOVE) ){
			OnIdle();
			Sleep(1);
		}
		if ( GetMessage(&m_msg, NULL, 0, 0) ==0 ){
			return (int) m_msg.wParam;
		}
		TranslateMessage( &m_msg );
		DispatchMessage( &m_msg );
	}
	return 0;
}
//------------------------------------------------------------------------
void CWndBase::Uninit()
{
	OnUninit();

	UnregisterClass( m_pTitle, m_hInstance );
}
//------------------------------------------------------------------------
void CWndBase::SetLog( int nFontColor, const char *pMsg, ... )
{
	FUNCTION_BEGIN;
	SETLOGPARAM *pParam =NULL;

	INFOLOCK(m_logparamarray);  
	if (m_currlogparamidx<0 || m_currlogparamidx>=MAX_LISTITEMCOUMT){
		m_currlogparamidx=0;
	}
	pParam=&m_logparamarray[m_currlogparamidx];
	m_currlogparamidx++;
	UNINFOLOCK(m_logparamarray);  

	if ( !pParam )
		return;

	pParam->nFontColor = nFontColor;

	SYSTEMTIME st;
	GetLocalTime( &st );
	wsprintf( pParam->szDate, "%04d-%02d-%02d %02d:%02d:%02d", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );

	va_list	vList;
	va_start( vList, pMsg );
	vsprintf_s( pParam->szText, sizeof(pParam->szText)-1, pMsg, vList );
	va_end  ( vList );

	pParam->szText[sizeof(pParam->szText)-1]=0;
	PostMessage( m_hWnd, UM_SETLOG, (WPARAM) pParam, 0 );
}
//------------------------------------------------------------------------
void CWndBase::SetErr( int nErrCode )
{
	FUNCTION_BEGIN;
	char Msg[1024-32]={0};

	FormatMessage( 
		FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL,
		nErrCode,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
		Msg,
		sizeof(Msg)-1,
		NULL );

	char* p=strrchr( Msg, '\r' );
	if (p){
		*(p) = ' ';
	}
	Msg[sizeof(Msg)-1]=0;
	SetLog( 0, "[Win32 Error] %s", Msg );
}
//------------------------------------------------------------------------
UINT_PTR CWndBase::CreateTimer(int nTimerID,int nTime)
{
	return ::SetTimer(m_hWnd,nTimerID,nTime,(TIMERPROC)&WinProc);
}
//------------------------------------------------------------------------
bool CWndBase::FreeTimer(int nTimerID)
{
	return (::KillTimer(m_hWnd,nTimerID)==TRUE);
}
//------------------------------------------------------------------------
void CWndBase::SetStatus(int nStatindex ,const char *pMsg, ... )
{
	FUNCTION_BEGIN;
	if (nStatindex>=30)
	{
		return;
	}
	WPARAM nIdx=(WPARAM)nStatindex;
	char* szBuf=m_szStatus[nStatindex];

	char szTempbuf[1024]={0};

	va_list	vList;
	va_start( vList, pMsg );
	vsprintf_s( szTempbuf, sizeof(szTempbuf)-1, pMsg, vList );
	va_end  ( vList );

	if (strcmp(szTempbuf,szBuf)!=0)
	{
		strcpy_s(szBuf,sizeof(m_szStatus[nStatindex])-1,szTempbuf);
		szBuf[sizeof(m_szStatus[nStatindex])-1]=0;
		PostMessage( m_hWnd, UM_SETSTATUS, nIdx, (LPARAM) szBuf );
	}
}
//------------------------------------------------------------------------
long CWndBase::OnCreate()
{
	return 0;
}
//------------------------------------------------------------------------
long CWndBase::OnSize( int nWidth, int nHeight )
{	
	RECT rcTb, rcSt;

	GetWindowRect( m_hWndToolbar, &rcTb );
	MoveWindow( m_hWndToolbar, 0, 0, 
		nWidth, rcTb.bottom - rcTb.top, TRUE );

	GetWindowRect( m_hWndStatus, &rcSt );
	MoveWindow( m_hWndStatus, 0, nHeight - (rcSt.bottom - rcSt.top), 
		nWidth, rcSt.bottom - rcSt.top, TRUE );

	MoveWindow( m_hWndList, 0, rcTb.bottom - rcTb.top - 1, 
		nWidth, nHeight - ((rcTb.bottom - rcTb.top) + (rcSt.bottom - rcSt.top)) + 2, TRUE );

	return 0;
}
//------------------------------------------------------------------------
long CWndBase::OnDrawItem( int nCtlID, DRAWITEMSTRUCT *pDIS )
{
	LISTDATA *pData = ListGetItemData( pDIS->itemID );

	ListDrawItem( pDIS, pData, 0 );
	ListDrawItem( pDIS, pData, 1 );

	return 0;
}
//------------------------------------------------------------------------
long CWndBase::OnTimer( int nTimerID )
{
	return 0;
}
//------------------------------------------------------------------------
long CWndBase::OnDestroy()
{
	ListClearAll();
	PostQuitMessage( 0 );

	return 0;
}
//------------------------------------------------------------------------
long CWndBase::OnSetStatus( int nIdx,const char* pMsg )
{
	SendMessage( m_hWndStatus, SB_SETTEXT, (long)nIdx, (long) pMsg );

	return 0;
}
//------------------------------------------------------------------------
long CWndBase::OnSetLog( SETLOGPARAM *pParam )
{
	LISTDATA *pData =NULL;
	if ( ListView_GetItemCount( m_hWndList ) >= MAX_LISTITEMCOUMT ){
		ListClearAll(); 	
	}

	pData =NULL;
	INFOLOCK(m_listdataary);  
	if (m_currlistdataidx<0 || m_currlistdataidx>=MAX_LISTITEMCOUMT){
		m_currlistdataidx=0;
	}
	pData=&m_listdataary[m_currlistdataidx];
	m_currlistdataidx++;
	UNINFOLOCK(m_listdataary);  

	pData->crFont=pParam->nFontColor;
	LV_ITEM lvi;
	memset( &lvi, 0, sizeof( lvi ) );
	lvi.mask	= LVIF_TEXT | LVIF_PARAM;
	lvi.iItem	= ListView_GetItemCount( m_hWndList );
	lvi.pszText	= pParam->szDate;
	lvi.lParam	= (LPARAM) pData;
	ListView_InsertItem( m_hWndList, &lvi );
	ListView_SetItemText( m_hWndList, lvi.iItem, 1, pParam->szText );
	ListView_EnsureVisible( m_hWndList, lvi.iItem, TRUE );
	return 0;
}
//------------------------------------------------------------------------
bool CWndBase::CreateWnd()
{
	WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, WinProc, 0, 0, m_hInstance, 
		LoadIcon( m_hInstance, "IDI_ICON" ), 0, 0, "IDR_MENU", m_wndClassName, 0 };

	if ( !RegisterClassEx( &wc ) )
		return false;

	m_hWnd = CreateWindow( m_wndClassName, m_pTitle, WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, m_nwidth, m_nheight, 0, 0, m_hInstance, this );

	SetWindowLong( m_hWnd, GWL_USERDATA,(long)this);
	if (m_ismainwindow){g_mainwindowhandle=m_hWnd;}
	return true;
}
//------------------------------------------------------------------------
bool CWndBase::CreateList()
{
	m_hWndList = CreateWindow( WC_LISTVIEW, "", 
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDRAWFIXED | LVS_EX_CHECKBOXES,
		0, 0, 0, 0, m_hWnd, 0, m_hInstance, 0 );

	if ( !m_hWndList )
		return false;

	ListView_SetExtendedListViewStyleEx( m_hWndList, 0, LVS_EX_FULLROWSELECT );

	LVCOLUMN lvc;
	lvc.mask		= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt			= LVCFMT_LEFT;
	lvc.cx			= 130;
	lvc.pszText		= "时间";
	ListView_InsertColumn( m_hWndList, 0, &lvc );
	lvc.cx			= m_nwidth - 140;
	lvc.pszText		= "消息";
	ListView_InsertColumn( m_hWndList, 1, &lvc );
	SetWindowLong( m_hWndList, GWL_USERDATA,(long)this);
	m_ListViewMsgProc=(WNDPROC)SetWindowLong (m_hWndList, GWL_WNDPROC, (long)WinProc);
	return true;
}
//------------------------------------------------------------------------
bool CWndBase::CreateStatus()
{
	m_hWndStatus = CreateWindow( STATUSCLASSNAME, "", 
		WS_CHILD | WS_VISIBLE | WS_BORDER | SBS_SIZEGRIP,
		100, 100, 500, 300, m_hWnd, 0, m_hInstance, 0 );

	if ( !m_hWndStatus )
		return false;

	return true;
}
//------------------------------------------------------------------------
LISTDATA * CWndBase::ListGetItemData( int nItem )
{
	LV_ITEM lvi;
	memset( &lvi, 0, sizeof( lvi ) );
	lvi.mask	= LVIF_PARAM;
	lvi.iItem	= nItem;
	ListView_GetItem( m_hWndList, &lvi );

	return (LISTDATA *) lvi.lParam;
}
//------------------------------------------------------------------------
void CWndBase::ListDrawItem( DRAWITEMSTRUCT *pDIS, LISTDATA *pData, int nSubItem )
{
	char szText[1024]={0};
	ListView_GetItemText( m_hWndList, pDIS->itemID, nSubItem, szText, sizeof( szText )-1 );
	szText[sizeof(szText)-1]=0;
	LV_ITEM lvi;
	memset( &lvi, 0, sizeof( lvi ) );
	lvi.mask		= LVIF_STATE;
	lvi.iItem		= pDIS->itemID;
	lvi.stateMask	= 0xFFFF;
	ListView_GetItem( m_hWndList, &lvi );
	bool bHighlight = (lvi.state & LVIS_DROPHILITED) || (lvi.state & LVIS_SELECTED);

	RECT rcItem;
	ListView_GetSubItemRect( m_hWndList, pDIS->itemID, nSubItem, LVIR_LABEL, &rcItem );

	if ( bHighlight )
	{
		SetBkColor( pDIS->hDC, GetSysColor( COLOR_HIGHLIGHT ) );
		ExtTextOut( pDIS->hDC, 0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL );
		SetTextColor( pDIS->hDC, GetSysColor( COLOR_WINDOW ) );		
	}
	else
	{
		SetBkColor( pDIS->hDC, GetSysColor( COLOR_WINDOW ) );
		ExtTextOut( pDIS->hDC, 0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL );
		SetTextColor( pDIS->hDC, pData->crFont );
	}

	DrawText( pDIS->hDC, szText, strlen( szText ), &rcItem, 
		DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT | DT_VCENTER );
}
//------------------------------------------------------------------------
void CWndBase::ListClearAll()
{
	for ( int i = 0; i < ListView_GetItemCount( m_hWndList ); i++ ){}
	ListView_DeleteAllItems( m_hWndList );

	INFOLOCK(m_listdataary);  
	m_currlistdataidx=0;
	UNINFOLOCK(m_listdataary);  
	INFOLOCK(m_listdataary);  
	m_currlogparamidx=0;
	UNINFOLOCK(m_listdataary);  
}
//------------------------------------------------------------------------
char* CWndBase::GetTitle()
{
	FUNCTION_BEGIN;
	if (m_pTitle[0]!=0){return m_pTitle;}
	else if(m_hWnd)
	{
		ZeroMemory(&m_pTitle,sizeof(m_pTitle));
		GetWindowText(m_hWnd,m_pTitle,(sizeof(m_pTitle))-1);
	}
	return "";
}
//------------------------------------------------------------------------
void CWndBase::SetTitle(const char* szTitle){
	FUNCTION_BEGIN;
	ZeroMemory(&m_pTitle,sizeof(m_pTitle));
	strcpy_s(m_pTitle,(sizeof(m_pTitle))-1,szTitle);
	if(m_hWnd){SetWindowText(m_hWnd,m_pTitle);}
}
//------------------------------------------------------------------------
long CWndBase::WinProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam ){
	if (hWnd==0){return 0;}
	CWndBase *pThis=(CWndBase*)GetWindowLong( hWnd, GWL_USERDATA );
	if (pThis && hWnd==pThis->m_hWndList){

		if ((nMsg==WM_CHAR
			&& ((WCHAR)wParam==VK_CANCEL ) 
			&& GetKeyState(VK_CONTROL)<0)
			|| nMsg==WM_LBUTTONDBLCLK ){
				LVHITTESTINFO	lvhittest;
				GetCursorPos( &lvhittest.pt );
				ScreenToClient( hWnd, &lvhittest.pt );
				ListView_HitTest( hWnd, &lvhittest );
				if (lvhittest.iItem>=0){
					char szSub1[128]={0};
					ListView_GetItemText( hWnd,lvhittest.iItem, 0, szSub1, sizeof( szSub1 )-1 );
					char szSub2[1024]={0};
					ListView_GetItemText( hWnd,lvhittest.iItem, 1, szSub2, sizeof( szSub2 )-1 );

					char szText[1024+128]={0};
					sprintf_s(szText,sizeof(szText)-1,"%s\x9%s\0",szSub1,szSub2);
					SetClipboardText(szText);
				}
				return 0;
		}
		return CallWindowProc(pThis->m_ListViewMsgProc,hWnd,nMsg,wParam,lParam);
	}else if (pThis && hWnd==pThis->m_hCmdEdit){
		if (nMsg==WM_CHAR && wParam==VK_RETURN){
			char szcmdbuf[1024*4];
			::GetWindowText(hWnd,szcmdbuf,sizeof(szcmdbuf)-1);
			pThis->OnCommand(szcmdbuf);
			::SetWindowText(hWnd,"");
		}
		return CallWindowProc(pThis->m_EditMsgProc,hWnd,nMsg,wParam,lParam);
	}else{
		if (pThis){
			switch(nMsg)
			{
			case WM_CREATE:
				pThis = (CWndBase *) ((CREATESTRUCT *) lParam)->lpCreateParams;
				return pThis->OnCreate();

			case WM_SIZE:
				return pThis->OnSize( LOWORD( lParam ), HIWORD( lParam ) );

			case WM_DRAWITEM:
				return pThis->OnDrawItem( wParam, (DRAWITEMSTRUCT *) lParam );

			case WM_COMMAND:
				return pThis->OnCommand( LOWORD( wParam ) );

			case WM_TIMER:
				return pThis->OnTimer( wParam );

			case WM_CLOSE:
				{
					bool boClose=true;
					pThis->OnQueryClose(boClose);
					if(!boClose){
						return 0;
					}
				}
				break;
			case WM_DESTROY:
				return pThis->OnDestroy();

			case UM_SETLOG:
				return pThis->OnSetLog( (SETLOGPARAM *) wParam );
			case UM_SETSTATUS:
				return pThis->OnSetStatus(wParam,(char *)lParam);

			}
		}else if(WM_CREATE==nMsg){
			pThis = (CWndBase *) ((CREATESTRUCT *) lParam)->lpCreateParams;
			return pThis->OnCreate();
		}
	}
	return DefWindowProc( hWnd, nMsg, wParam, lParam );
}
//------------------------------------------------------------------------
int __stdcall myWinMain(HMODULE hModule,const char* param){
	FUNCTION_BEGIN;
	try{
		CWndBase *pWnd = NULL;
		OnCreateInstance(pWnd);
		if (pWnd){
			if ( !pWnd->Init( hModule ) ){
				MessageBox( NULL, "Failed to initialize windows", pWnd->GetTitle(), MB_ICONERROR );
				OnDestroyInstance( pWnd );
				return -1;
			}
			pWnd->Run();
			pWnd->Uninit();
		}
		return OnDestroyInstance( pWnd );
	}
	catch (std::exception& e){
		g_logger.error("[ %s : PID=%d : TID=%d ] exception: %s \r\nCallStack:--------------------------------------------------\r\n",__FUNC_LINE__,::GetCurrentProcessId(),::GetCurrentThreadId(),e.what());
		PrintThreadCallStack(NULL);
		g_logger.error("======================================\r\n\r\n");
	}
	return 0;
}
//------------------------------------------------------------------------
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE h, char * p, int n ){
	int nRet=0;
	EXCEPTION_CATCH_INIT(hInstance);
	nRet= myWinMain(::GetModuleHandle(NULL),::GetCommandLine());
	EXCEPTION_CATCH_UNINIT;
	return nRet;
}
//------------------------------------------------------------------------
int _tmain(int argc, _TCHAR* argv[]){
	int nRet=0;
	EXCEPTION_CATCH_INIT(::GetModuleHandle(NULL));
	nRet= myWinMain(::GetModuleHandle(NULL),::GetCommandLine());
	EXCEPTION_CATCH_UNINIT;
	return nRet;
}
//------------------------------------------------------------------------