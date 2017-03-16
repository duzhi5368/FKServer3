/**
*	created:		2013-4-7   22:13
*	filename: 		FKRandomPool
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKRandomGenerator.h"
#include <vector>
#include <hash_map>
#include "FKSyncObjLock.h"
//------------------------------------------------------------------------
using namespace std;
using namespace stdext;
//------------------------------------------------------------------------
// 需要在g_TrueRandomGenerator 已经初始化后才能被正确使用 
class RandomPool
{
	typedef std::vector< DWORD > PoolElemVec;
	enum	RandPoolSize
	{
		PoolSize = 100000
	};
public:
	RandomPool();
	~RandomPool();
public:
	DWORD					GetOneRand();
	DWORD					GetOneRandLess( DWORD dst );
	DWORD					GetOneRandBetween( DWORD, DWORD );
private:
	void					InitPool();
	void					StepPtr();
private:
	PoolElemVec				m_Pool;
	DWORD					m_ElePtr;
};
//------------------------------------------------------------------------
extern RandomPool g_RandomPool;
//------------------------------------------------------------------------
// 消息计数器
class MsgCounter
{
public :
	MsgCounter();
	~MsgCounter();

	inline void AddAllCounter()
	{
		m_AllMsgCounter++;
		if( m_AllMsgCounter >= 4200000000 )
		{
			_Reset();
		}
	}

	void	AddMsg( int mainid, int subid );
	void	AddSendCounter();
private:
	void	_Reset();
	void	_Print();
private:
	stdext::hash_map< unsigned int , unsigned int > m_CounterMap;
	unsigned int	m_AllMsgCounter;
	unsigned int	m_SortTimeDelta;
	int				m_LastPrintTime;
	DWORD			m_SendCounter;
	CIntLock		m_Locker;
};
//------------------------------------------------------------------------
extern MsgCounter g_msgCounter;
//------------------------------------------------------------------------