/**
*	created:		2013-4-8   14:53
*	filename: 		FKSyncObjLock
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/FKSyncObjLock.h"
#include "../Include/FKError.h"
#include "../Include/FKThread.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include "../Include/FKTimeMonitor.h"
//------------------------------------------------------------------------
CLD_CCriticalSection::CLD_CCriticalSection()
{
	InitializeCriticalSectionAndSpinCount( &m_csAccess,4000);
}
//------------------------------------------------------------------------
CLD_CCriticalSection::~CLD_CCriticalSection()
{
	DeleteCriticalSection( &m_csAccess );
}
//------------------------------------------------------------------------
void CLD_CCriticalSection::Lock()
{
	EnterCriticalSection( &m_csAccess );
}
//------------------------------------------------------------------------
void CLD_CCriticalSection::Unlock()
{
	LeaveCriticalSection( &m_csAccess );
}
//------------------------------------------------------------------------
CLD_CMutex::CLD_CMutex( char *pName )
{		
	m_hMutex=NULL;
	m_boCreate=false;
	if (pName){
		open(pName);
	}
}	
//------------------------------------------------------------------------
CLD_CMutex::~CLD_CMutex()
{
	close();
}
//------------------------------------------------------------------------
bool CLD_CMutex::open( char *pName )
{
	if (m_hMutex == NULL)
	{
		m_hMutex=::OpenMutex(MUTEX_ALL_ACCESS, FALSE, pName);
		if (m_hMutex == NULL)
		{
			m_hMutex = CreateMutex( NULL, FALSE, pName );
			if ( m_hMutex == NULL )
			{
				throw CLDError( "CLD_CMutex::CLD_CMutex() ´´½¨Ê§°Ü" );
			}
			m_boCreate=true;
		}
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
void CLD_CMutex::close()
{
	if (m_hMutex != NULL){
		CloseHandle( m_hMutex );
		m_hMutex=NULL;
	}
	m_boCreate=false;
}
//------------------------------------------------------------------------
void CLD_CMutex::Lock( int nTimeWait)
{
	if (m_hMutex)
	{
		WaitForSingleObject( m_hMutex, nTimeWait );
	}
}
//------------------------------------------------------------------------
void CLD_CMutex::Unlock()
{
	if (m_hMutex)
	{
		ReleaseMutex( m_hMutex );
	}
}
//------------------------------------------------------------------------