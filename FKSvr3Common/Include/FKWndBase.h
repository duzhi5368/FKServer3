/**
*	created:		2013-4-7   22:40
*	filename: 		FKWndBase
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#pragma comment( lib, "comctl32.lib" )
//------------------------------------------------------------------------
#include "FKBaseDefine.h"
#include "STLTemplate/FKSyncList.h"
#include <CommCtrl.h>
#include "FKVsVer8Define.h"
#include "FKLogger.h"
//------------------------------------------------------------------------
#define UM_SETLOG			WM_USER + 1250
#define UM_SETSTATUS		WM_USER + 1251
#define WND_WIDTH			320*2
#define WND_HEIGHT			240
#define MAX_LISTITEMCOUMT	500
//------------------------------------------------------------------------
struct LISTDATA
{
	long crFont;
};
//------------------------------------------------------------------------
struct SETLOGPARAM
{
	int		 nFontColor;
	char	 szDate[32];
	char	 szText[1024];
};
//------------------------------------------------------------------------
class CWndBase
{
protected:
	CSyncVector< SETLOGPARAM > m_logparamarray;
	int						m_currlogparamidx;
	char					m_szStatus[32][32*3];
	CSyncVector< LISTDATA >	m_listdataary;
	int						m_currlistdataidx;
	const int				m_nwidth;
	const int				m_nheight;
	const char*				m_wndClassName;
	WNDPROC					m_ListViewMsgProc;
	WNDPROC					m_EditMsgProc;
	char 					m_pTitle[32*6];
	MSG						m_msg;
	bool					m_ismainwindow;

	virtual bool CreateWnd();
	virtual bool CreateList();
	virtual bool CreateStatus();
	void ListDrawItem( DRAWITEMSTRUCT *pDIS, LISTDATA *pData, int nSubItem );
	static long __stdcall WinProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam ); 
public:

	HINSTANCE	m_hInstance;
	HWND		m_hWnd;
	HWND		m_hWndToolbar,m_hCmdEdit, m_hWndList, m_hWndStatus;

	CWndBase(int width=WND_WIDTH,int height=WND_HEIGHT,const char* wndClassName="CLD_WNDBASE",bool ismainwindow=true);
	virtual ~CWndBase();

	virtual bool Init( HINSTANCE hInstance )=0;
	int Run();
	void Processmsg();
	void Uninit();

	void Close();

	virtual void SetLog( int nFontColor,const char *pMsg, ... );
	virtual void SetErr( int nErrCode );
	void __cdecl SetStatus( int nStatindex ,const char *pMsg, ...  );

	UINT_PTR CreateTimer(int nTimerID,int nTime);
	bool FreeTimer(int nTimerID);

	void ListClearAll();
	LISTDATA * ListGetItemData( int nItem );

	virtual long OnCreate();
	virtual long OnSize( int nWidth, int nHeight );
	virtual long OnDrawItem( int nCtlID, DRAWITEMSTRUCT *pDIS );
	virtual long OnCommand( int nCmdID )=0;
	virtual bool OnCommand( char* szCmd ){ return true; }
	virtual long OnTimer( int nTimerID );
	virtual void OnQueryClose(bool& boClose){};
	virtual long OnDestroy();
	virtual long OnSetLog( SETLOGPARAM *pParam );
	virtual long OnSetStatus( int nIdx,const char* pMsg );

	virtual void OnIdle(){};

	virtual bool OnInit()			{ return true; }
	virtual void OnUninit()			{}
	virtual void OnStartService()	{}
	virtual void OnStopService()	{}
	virtual void OnConfiguration()	{}
	virtual void OnClearLog()		{ zLogger::save();ListClearAll(); }
	virtual void OnExit()			{ PostMessage( m_hWnd, WM_CLOSE, 0, 0 ); }

	void SetTitle(const char* szTitle);
	char* GetTitle();
};
//------------------------------------------------------------------------
void		OnCreateInstance (CWndBase *&pWnd);
int			OnDestroyInstance( CWndBase *pWnd );
//------------------------------------------------------------------------
#define NOT_USEWNDBASE		int __mymain(HMODULE hModule,const char* pcommon);	\
	void OnCreateInstance (CWndBase *&pWnd){ pWnd=NULL;}	\
	int	OnDestroyInstance( CWndBase *pWnd ){return __mymain(::GetModuleHandle(NULL),::GetCommandLine());}
//------------------------------------------------------------------------