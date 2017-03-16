/**
*	created:		2013-4-6   23:11
*	filename: 		FKOutput
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKOutput.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include <tchar.h>
//------------------------------------------------------------------------
CLD_Output::CLD_Output(bool boFileNameReadOnly)
{
	char stPath[MAX_PATH]={0};
	GetModuleFileNameA(g_hinstance, stPath, MAX_PATH-1);
	char* p = strrchr(stPath,'.');
	if(p){*p='_';p++;}

	time_t ti = time(NULL);

	struct tm _newtime;
	localtime_s( &_newtime, &ti);

	char szTime[MAX_PATH];
	strftime(szTime,MAX_PATH,"%y%m%d%H%M%S ", &_newtime );

	memset(m_strFile, 0x00, sizeof(m_strFile));
	sprintf_s(m_strFile,sizeof(m_strFile)-1,"%s_%s.log",stPath,szTime);
	m_boFileNameReadOnly=boFileNameReadOnly;
}
//------------------------------------------------------------------------
CLD_Output::CLD_Output(const char * pch,bool boFileNameReadOnly)
{
	assert(pch != NULL);
	memset(m_strFile, 0x00, sizeof(m_strFile));
	strcpy_s(m_strFile,sizeof(m_strFile), pch);
	m_boFileNameReadOnly=boFileNameReadOnly;
}
//------------------------------------------------------------------------
CLD_Output::~CLD_Output()
{
}
//------------------------------------------------------------------------
void CLD_Output::SetFileName(const char * pch)
{
	if (pch && !m_boFileNameReadOnly)
	{
		memset(m_strFile, 0x00, sizeof(m_strFile));
		strcpy_s(m_strFile,sizeof(m_strFile), pch);
	}
}
//------------------------------------------------------------------------
void CLD_Output::Set360FileTime(const char* sztime)
{
	if (sztime && sztime[0]!=0)
	{
		memset(m_strTime, 0x00, sizeof(m_strTime));
		strcpy_s(m_strTime,sizeof(m_strTime), sztime);
	}
}
//------------------------------------------------------------------------
char * CLD_Output::GetFileName()
{
	return m_strFile;
}
//------------------------------------------------------------------------
char* CLD_Output::GetFileTime()
{
	return m_strTime;
}
//------------------------------------------------------------------------
void CLD_Output::WriteBuf(void *lpbuf, int buflen)
{
	assert(lpbuf != NULL);
	FILE* fp=NULL; 
	try
	{
		errno_t err = fopen_s(&fp,m_strFile,"ab");
		if (fp && err==0){
			while (buflen>0){
				if (buflen>=MAX_WRITE_NUM){
					fwrite(lpbuf,MAX_WRITE_NUM,1,fp);
					lpbuf=(void *)(DWORD(lpbuf)+MAX_WRITE_NUM);
					buflen=buflen-MAX_WRITE_NUM;
				}else{
					fwrite(lpbuf,buflen,1,fp);
					buflen=0;
				}
			}
			fclose(fp);fp=NULL;
		}
	}catch (...){			
		if (fp){fclose(fp);fp=NULL;}
	}
}
//------------------------------------------------------------------------
void CLD_Output::WriteInt(__int64 OutInt)
{
	char out[128];
	sprintf_s(out,sizeof(out), "%I64u \r\n", OutInt);
	WriteString(out);
}
//------------------------------------------------------------------------
void __cdecl CLD_Output::WriteString(bool bNewLine, const char * lpFmt, ...)
{
	assert(lpFmt != NULL);
	char szText[2048];
	va_list argList;
	va_start(argList, lpFmt);
	vsprintf_s(szText,sizeof(szText), lpFmt, argList);
	va_end(argList);

	WriteString(szText, bNewLine);
}
//------------------------------------------------------------------------
void CLD_Output::WriteString(const char * pstr, bool bNewLine)
{
	AILOCKP(n_logInpLock);
	assert(pstr != NULL);
	FILE* fp=NULL; 
	try
	{
		errno_t err = fopen_s(&fp,m_strFile,"ab");
		if (fp && err==0){
			fputs(pstr,fp);
			if (bNewLine){
				fputs("\r\n",fp);
			}
			fclose(fp);fp=NULL;
		}
	}catch (...){
		if (fp){fclose(fp);fp=NULL;}
	}
}
//------------------------------------------------------------------------
void CLD_Output::NewLine()
{
	FILE* fp=NULL; 
	try
	{
		errno_t err = fopen_s(&fp,m_strFile,"ab");
		if (fp && err==0){
			fputs("\r\n",fp);
			fclose(fp);fp=NULL;
		}
	}catch (...){
		if (fp){fclose(fp);fp=NULL;}
	}
}
//------------------------------------------------------------------------
void __cdecl CLD_Output::ShowMessageBox(const char * lpFmt, ...)
{
	assert(lpFmt != NULL);
	char szText[2048];
	va_list argList;
	va_start(argList, lpFmt);
	vsprintf_s(szText,sizeof(szText), lpFmt, argList);
	va_end(argList);
	MessageBoxA(0, szText, "ShowMessageBox", 0);
}
//------------------------------------------------------------------------
void CLD_Output::ShowMessageBox(__int64 OutInt)
{
	char out[128];
	sprintf_s(out,sizeof(out), "%I64u", OutInt);
	MessageBoxA(0, out, "ShowMessageBox", 0);
}
//------------------------------------------------------------------------