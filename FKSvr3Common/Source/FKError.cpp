/**
*	created:		2013-3-22   19:40
*	filename: 		FKError
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKError.h"
//------------------------------------------------------------------------
CLDError::CLDError( char *pMsg ,int nErrorCode,bool syserror)
{
	if (!syserror)
	{
		m_nErrorCode=nErrorCode;
		strcpy_s( m_szMsg,sizeof(m_szMsg), pMsg );
	}
	else
	{
		char sWinErrMsgBuf[512]={0};
		formatsyserror(nErrorCode,sWinErrMsgBuf,sizeof(sWinErrMsgBuf));
		sprintf_s(m_szMsg,sizeof(m_szMsg)-1,"%s : %s",pMsg,sWinErrMsgBuf);
	}
}
//------------------------------------------------------------------------
CLDError::~CLDError()
{
	m_nErrorCode=0;
}
//------------------------------------------------------------------------
const char * CLDError::GetMsg()
{
	return m_szMsg;
}
//------------------------------------------------------------------------
void _outputerr( char *pMsg, ... )
{
	char szBuffer[ERROR_MAXBUF] = {0,};

	va_list	stream;		
	va_start( stream, pMsg );
	vsprintf_s( szBuffer,sizeof(szBuffer), pMsg, stream );
	va_end( stream );

	OutputDebugString( szBuffer );
}
//------------------------------------------------------------------------