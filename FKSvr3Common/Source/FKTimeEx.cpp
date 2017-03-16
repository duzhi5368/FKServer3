/**
*	created:		2013-3-22   22:36
*	filename: 		FKTimeEx
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKTimeEx.h"
#include "../Include/FKSyncObjLock.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include <WinSock2.h>
//------------------------------------------------------------------------
CTimer g_timer(true);
//------------------------------------------------------------------------
__int64 CTimer::m_n64Freq=0;
//------------------------------------------------------------------------
CTimer::CTimer(bool bPlay,bool boRDTSC)
		: m_n64TimeBegin(0)
		, m_n64TimeEnd(0)
		, m_TimerStatus(tsStop)
		, m_boRDTSC(boRDTSC)
{
	FUNCTION_BEGIN;

	if (m_boRDTSC)
	{
		if (m_n64Freq==0)
		{
			LARGE_INTEGER tmp;
			if (QueryPerformanceFrequency(&tmp) == FALSE)
			{
				m_boRDTSC=false;
			}
			else
			{
				m_n64Freq = tmp.QuadPart;
			}
		}
	}
	if (!m_boRDTSC)
	{
		m_n64Freq=1000;
	}
	if (bPlay)
	{
		Play();
	}
}
//------------------------------------------------------------------------
CTimer::~CTimer(void)
{
	FUNCTION_BEGIN;
	if (m_TimerStatus != tsStop)
	{
		Stop();
	}
}
//------------------------------------------------------------------------
DWORD CTimer::GetTime(int nPrecision)
{
	if (nPrecision<1)
	{
		nPrecision = 1;
	}
	if (m_TimerStatus != tsRun)
	{
		if (m_n64TimeEnd < m_n64TimeBegin)
		{ 
			m_n64TimeEnd = m_n64TimeBegin;
		}
		if (nPrecision!=m_n64Freq)
		{
			return DWORD((m_n64TimeEnd - m_n64TimeBegin) * nPrecision / m_n64Freq);
		}
		else
		{
			return DWORD(m_n64TimeEnd - m_n64TimeBegin);
		}
	}
	else
	{
		if (nPrecision!=m_n64Freq)
		{
			return DWORD((GetCurrentCount() - m_n64TimeBegin) * nPrecision / m_n64Freq);
		}
		else
		{
			return DWORD(GetCurrentCount() - m_n64TimeBegin);
		}
	}
}
//------------------------------------------------------------------------
float CTimer::GetTimef(float fPrecision ){
	if (fPrecision<1.0)
	{
		fPrecision=1.0;
	}
	if (m_TimerStatus != tsRun)
	{
		if (m_n64TimeEnd < m_n64TimeBegin){	m_n64TimeEnd = m_n64TimeBegin; }
		return (float)(((double)(m_n64TimeEnd - m_n64TimeBegin)) * fPrecision / (double)m_n64Freq);
	}
	else
	{
		return (float)(((double)(GetCurrentCount() - m_n64TimeBegin)) * fPrecision / (double)m_n64Freq);
	}
}
//------------------------------------------------------------------------
void CTimer::Reset(void)
{
	m_TimerStatus = tsRun;
	m_n64TimeBegin = GetCurrentCount();
}
//------------------------------------------------------------------------
void CTimer::Play(void)
{
	if (m_TimerStatus == tsStop)
	{
		m_n64TimeBegin = GetCurrentCount();
	}
	m_TimerStatus = tsRun;
}
//------------------------------------------------------------------------
void CTimer::Stop(void)
{
	m_n64TimeEnd = GetCurrentCount();
	m_TimerStatus = tsStop;
}
//------------------------------------------------------------------------
void CTimer::Pause(void)
{
	m_n64TimeEnd = GetCurrentCount();
	m_TimerStatus = tsPause;
}
//------------------------------------------------------------------------
__int64 CTimer::GetCurrentCount(void)
{
	if (m_boRDTSC)
	{
		LARGE_INTEGER tmp;
		QueryPerformanceCounter(&tmp);
		return tmp.QuadPart;
	}
	else
	{
		return timeGetTime();
	}
}
//------------------------------------------------------------------------
time_t GetTimeSec()
{
	return time(NULL);
}
//------------------------------------------------------------------------
time_t GetTimeSecEx()
{
	static time_t dwStartsec=GetTimeSec();
	static long long dwStartTickCount=GetTickCount();
	return dwStartsec+(time_t)((GetTickCount()-dwStartTickCount) / 1000);
}
//------------------------------------------------------------------------
long long	GetTimeMsecs()
{
	struct timeval rtm;
	SYSTEMTIME sys;

	GetLocalTime( &sys ); 
	FILETIME ft;
	SystemTimeToFileTime(&sys,   &ft);   
	unsigned   __int64 tt=0;
	DWORD*   p64   =   (DWORD*)&tt;   
	p64[0]   =   ft.dwLowDateTime;   
	p64[1]   =   ft.dwHighDateTime; 

	tt=tt-116444736000000000i64;
	tt /= 10; 
	rtm.tv_sec = (long)(tt / 1000000i64);
	rtm.tv_usec = (long)(tt % 1000000i64);

	long long retval = rtm.tv_sec;
	retval *= 1000;
	retval += rtm.tv_usec / 1000;
	return retval;
}
//------------------------------------------------------------------------
long long	GetTimeMsecsEx()
{
	static long long	 i64StartMsecs=GetTimeMsecs();
	static long long	 dwStartTickCount=GetTickCount();
	return i64StartMsecs+(GetTickCount()-dwStartTickCount);
}
//------------------------------------------------------------------------