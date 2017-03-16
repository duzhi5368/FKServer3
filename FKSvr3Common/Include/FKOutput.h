/**
*	created:		2013-4-6   23:09
*	filename: 		FKOutput
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include "FKVsVer8Define.h"
#include "FKSyncObjLock.h"
#include "FKWinFileIO.h"
//------------------------------------------------------------------------
#ifdef _DEBUG
#include <crtdbg.h>
#endif
//------------------------------------------------------------------------
#pragma warning(disable: 4267)
#define  MAX_WRITE_NUM		1024*64
//------------------------------------------------------------------------
class CLD_Output
{
private:
	char m_strFile[MAX_PATH - 1];
	char m_strTime[MAX_PATH];
	bool m_boFileNameReadOnly;
public:
	CInpLock n_logInpLock;
public:
	CLD_Output(bool boFileNameReadOnly=false);
	CLD_Output(const char * pch,bool boFileNameReadOnly=false);
	virtual ~CLD_Output();
public:
	void SetFileName(const char * pch);
	void Set360FileTime(const char* sztime);
	char * GetFileName();
	char * GetFileTime();
	void WriteBuf(void *lpbuf, int buflen);
	void WriteInt(__int64 OutInt);
	void __cdecl WriteString(bool bNewLine, const char * lpFmt, ...);
	void WriteString(const char * pstr, bool bNewLine = false);
	void NewLine();

	static void __cdecl ShowMessageBox(const char * lpFmt, ...);
	static void ShowMessageBox(__int64 OutInt);
	static __inline void __cdecl OutDebugMsg(const char * lpFmt, ...)
	{
#ifdef _DEBUG
		assert(lpFmt != NULL);
		char szText[2048];
		va_list argList;
		va_start(argList, lpFmt);
		vsprintf_s(szText,sizeof(szText), lpFmt, argList);
		va_end(argList);
		_RPT1(_CRT_WARN, "%s\n", szText);
#else
		assert(lpFmt != NULL);
		char szText[2048];
		va_list argList;
		va_start(argList, lpFmt);
		vsprintf_s(szText,sizeof(szText), lpFmt, argList);
		va_end(argList);

		strcat(szText,"\n");
		::OutputDebugString(szText);
#endif
	}

	static __inline void OutDebugMsg(__int64 OutInt)
	{
#ifdef _DEBUG
		_RPT1(_CRT_WARN, "%I64u\n", OutInt);
#else
		char szText[128];
		sprintf_s(szText,sizeof(szText),"%I64u\n",OutInt);
		::OutputDebugString(szText);
#endif
	}
};
//------------------------------------------------------------------------
typedef CLD_Output COutput;
extern COutput g_appout;
//------------------------------------------------------------------------