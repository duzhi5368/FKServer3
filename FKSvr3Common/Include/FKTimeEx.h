/**
*	created:		2013-3-22   22:33
*	filename: 		FKTimeEx
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <time.h>
#include "FKBaseDefine.h"
#include <stdio.h>
#include <wchar.h>
#include <MMSystem.h>
//------------------------------------------------------------------------
#pragma comment( lib,"Winmm.lib")
//------------------------------------------------------------------------
class CTimer
{
private:
	enum TimerStatus { tsRun, tsStop, tsPause };
public:
	CTimer(bool bPlay = false,bool boRDTSC=true);
	~CTimer(void);
public:
	DWORD GetTime(int nPrecision = 1000);
	float GetTimef(float fPrecision = 1000.0);
	void Play(void);
	void Stop(void);
	void Pause(void);
	void Reset(void);
private:
	__int64 GetCurrentCount(void);
private:
	static __int64 m_n64Freq;		
	__int64 m_n64TimeBegin;			
	__int64 m_n64TimeEnd;			
	TimerStatus m_TimerStatus;		
	bool m_boRDTSC;
}; 
//------------------------------------------------------------------------
extern CTimer g_timer;
//------------------------------------------------------------------------
time_t 					GetTimeSec(); 
time_t 					GetTimeSecEx();
long long				GetTimeMsecs();   
long long				GetTimeMsecsEx();
//------------------------------------------------------------------------