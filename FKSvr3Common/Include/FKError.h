/**
*	created:		2013-3-22   19:39
*	filename: 		FKError
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <exception>
#include "FKVsVer8Define.h"
//------------------------------------------------------------------------
#define ERROR_MAXBUF	1024
using namespace std;
//------------------------------------------------------------------------
class CLDError : public std::exception
{
protected:
	char	m_szMsg[ERROR_MAXBUF];
	int		m_nErrorCode;
public:
	CLDError( char *pMsg ,int nErrorCode=0,bool syserror=false );
	virtual ~CLDError();

	virtual const char * GetMsg();
	virtual const int GetErrorCode()
	{
		return m_nErrorCode;
	}
	virtual const char *what() const _THROW0()
	{
		return m_szMsg;
	}
	virtual void formatsyserror(int nerror,char* sWinErrMsgBuf,int nlen)
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY,NULL,nerror,0,sWinErrMsgBuf,nlen-1,NULL);
		char* p=strrchr( sWinErrMsgBuf, '\r' );
		if (p){*(p) = ' ';}
	}
};
//------------------------------------------------------------------------
void _outputerr( char *pMsg, ... );
//------------------------------------------------------------------------