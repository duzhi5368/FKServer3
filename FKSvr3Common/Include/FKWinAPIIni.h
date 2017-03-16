/**
*	created:		2013-4-8   13:14
*	filename: 		FKWinAPIIni
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKBaseDefine.h"
#include "Dump/FKDumpErrorBase.h"
//------------------------------------------------------------------------
class CWinApiIni
{
public:
	char m_szFileName[MAX_PATH];
	char m_szOpSecName[MAX_PATH];
public:
	__inline CWinApiIni()
	{
		GetModuleFileNameA(g_hinstance, m_szFileName, MAX_PATH);
		char* p = strrchr(m_szFileName,'.');
		if(p){*p='_';p++;}
		strcat(m_szFileName,".ini");
	}

	__inline CWinApiIni(LPSTR ptzFileName)
	{
		strcpy_s(m_szFileName,MAX_PATH-1, ptzFileName);
	}

	__inline void SetOpSection(LPSTR ptzSectionName)
	{
		strcpy_s(m_szOpSecName,MAX_PATH-1, ptzSectionName);
	}

	__inline void SetOpFile(LPSTR ptzFileName)
	{
		strcpy_s(m_szFileName,MAX_PATH-1, ptzFileName);
	}

	__inline UINT ReadInt(LPSTR ptzSectionName, LPSTR ptzKeyName, INT iDefault = 0)
	{
		return GetPrivateProfileIntA(ptzSectionName, ptzKeyName, iDefault, m_szFileName);
	}

	__inline BOOL WriteInt(LPSTR ptzSectionName, LPSTR ptzKeyName, INT iValue = 0)
	{
		CHAR tzString[30];
		wsprintfA(tzString, "%d", iValue);
		return WritePrivateProfileStringA(ptzSectionName, ptzKeyName, tzString, m_szFileName);
	}

	__inline DWORD ReadString(LPSTR ptzSectionName, LPSTR ptzKeyName, LPSTR ptzReturnedString, DWORD dwSize, LPSTR ptzDefault)
	{
		return GetPrivateProfileStringA(ptzSectionName, ptzKeyName, ptzDefault, ptzReturnedString, dwSize, m_szFileName);
	}

	__inline BOOL WriteString(LPSTR ptzSectionName, LPSTR ptzKeyName,
		LPSTR ptzString)
	{
		return WritePrivateProfileStringA(ptzSectionName, ptzKeyName, ptzString,
			m_szFileName);
	}

	__inline __int64 ReadInt64(LPSTR ptzSectionName, LPSTR ptzKeyName, __int64 iDefault)
	{
		CHAR tzString[64]={0};
		ReadString(ptzSectionName,ptzKeyName,tzString,sizeof(tzString)-1,"");
		return _strtoi64(tzString,NULL,10);
	}

	__inline BOOL WriteInt64(LPSTR ptzSectionName, LPSTR ptzKeyName, __int64 iValue)
	{
		CHAR tzString[64]={0};
		sprintf_s(tzString,sizeof(tzString)-1,"%I64d",iValue);
		return WriteString(ptzSectionName, ptzKeyName, tzString);
	}

	__inline BOOL ReadStruct(LPSTR ptzSectionName, LPSTR ptzKeyName, PVOID pvStruct, UINT uSize)
	{
		return GetPrivateProfileStructA(ptzSectionName, ptzKeyName, pvStruct, uSize, m_szFileName);
	}

	__inline BOOL WriteStruct(LPSTR ptzSectionName, LPSTR ptzKeyName, PVOID pvStruct, UINT uSize)
	{
		return WritePrivateProfileStructA(ptzSectionName, ptzKeyName, pvStruct, uSize, m_szFileName);
	}

	__inline DWORD ReadSection(LPSTR ptzSectionName, LPSTR ptzReturnBuffer, DWORD dwSize)
	{
		return GetPrivateProfileSectionA(ptzSectionName, ptzReturnBuffer, dwSize, m_szFileName);
	}

	__inline DWORD WriteSection(LPSTR ptzSectionName, LPSTR ptzString)
	{
		return WritePrivateProfileSectionA(ptzSectionName, ptzString, m_szFileName);
	}

	__inline DWORD ReadSectionNames(LPSTR ptzReturnBuffer, DWORD dwSize)
	{
		return GetPrivateProfileSectionNamesA(ptzReturnBuffer, dwSize, m_szFileName);
	}

	__inline UINT ReadInt(LPSTR ptzKeyName, INT iDefault = 0)
	{
		return GetPrivateProfileIntA(m_szOpSecName, ptzKeyName, iDefault, m_szFileName);
	}

	__inline BOOL WriteInt(LPSTR ptzKeyName, INT iValue = 0)
	{
		CHAR tzString[30];
		wsprintfA(tzString, "%d", iValue);
		return WritePrivateProfileStringA(m_szOpSecName, ptzKeyName, tzString, m_szFileName);
	}

	__inline DWORD ReadString(LPSTR ptzKeyName, LPSTR ptzReturnedString, DWORD dwSize, LPSTR ptzDefault)
	{
		return GetPrivateProfileStringA(m_szOpSecName, ptzKeyName, ptzDefault, ptzReturnedString, dwSize, m_szFileName);
	}

	__inline BOOL WriteString(LPSTR ptzKeyName, LPSTR ptzString)
	{
		return WritePrivateProfileStringA(m_szOpSecName, ptzKeyName, ptzString, m_szFileName);
	}

	__inline __int64 ReadInt64(LPSTR ptzKeyName, __int64 iDefault)
	{
		CHAR tzString[64]={0};
		ReadString(ptzKeyName,tzString,sizeof(tzString)-1,"");
		return _strtoi64(tzString,NULL,10);
	}

	__inline BOOL WriteInt64(LPSTR ptzKeyName, __int64 iValue)
	{
		CHAR tzString[64]={0};
		sprintf_s(tzString,sizeof(tzString)-1,"%I64d",iValue);
		return WriteString(ptzKeyName, tzString);
	}

	__inline BOOL ReadStruct(LPSTR ptzKeyName, PVOID pvStruct, UINT uSize)
	{
		return GetPrivateProfileStructA(m_szOpSecName, ptzKeyName, pvStruct, uSize, m_szFileName);
	}

	__inline BOOL WriteStruct(LPSTR ptzKeyName, PVOID pvStruct, UINT uSize)
	{
		return WritePrivateProfileStructA(m_szOpSecName, ptzKeyName, pvStruct, uSize, m_szFileName);
	}

	__inline DWORD ReadSection(LPSTR ptzReturnBuffer, DWORD dwSize)
	{
		return GetPrivateProfileSectionA(m_szOpSecName, ptzReturnBuffer, dwSize, m_szFileName);
	}

	__inline DWORD WriteSection(LPSTR ptzString)
	{
		return WritePrivateProfileSectionA(m_szOpSecName, ptzString, m_szFileName);
	}
};
//------------------------------------------------------------------------