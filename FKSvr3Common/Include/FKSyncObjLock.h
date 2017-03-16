/**
*	created:		2013-3-22   19:34
*	filename: 		FKSyncObjLock
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <stdio.h>
#include "FKBaseDefine.h"
#include "FKNoncopyable.h"
//------------------------------------------------------------------------		
class stLockTNone
{
public:
	virtual void Lock(){ return; }
	virtual void Unlock(){ return; }
};
//------------------------------------------------------------------------
class stLockPNone
{
public:
	virtual void Lock( int nTimeWait = INFINITE){ return; }
	virtual void Unlock(){ return; }
};
//------------------------------------------------------------------------
class CLD_CCriticalSection:public stLockTNone
{
public:
	CRITICAL_SECTION	m_csAccess;
public:
	CLD_CCriticalSection();
	virtual ~CLD_CCriticalSection();

	virtual void Lock();
	virtual void Unlock();
};
//------------------------------------------------------------------------
class CLD_CMutex:public stLockPNone
{
public:
	HANDLE	m_hMutex;
	bool m_boCreate;
public:
	CLD_CMutex( char *pName=NULL );
	virtual ~CLD_CMutex();

	virtual void Lock( int nTimeWait = INFINITE);
	virtual void Unlock();

	bool open( char *pName );
	void close();

	bool iscreate()	{return m_boCreate;}
};
//------------------------------------------------------------------------
class CLD_AutoInfoIntLock :private zNoncopyable
{
public:
	CLD_AutoInfoIntLock(stLockTNone& obj,const char* fflinet_name=NULL)
		:m_unlockobj(obj){
			obj.Lock();
	}
	~CLD_AutoInfoIntLock(){
		m_unlockobj.Unlock();
	}
private:
	stLockTNone& m_unlockobj;
};
//------------------------------------------------------------------------
class CLD_AutoInfoIntLockTime:private zNoncopyable
{
public:
	CLD_AutoInfoIntLockTime(stLockPNone& obj,const char* fflinet_name=NULL,int nTimeWait = INFINITE)
		:m_unlockobj(obj)
	{
		obj.Lock(nTimeWait);
	}
	~CLD_AutoInfoIntLockTime()
	{
		m_unlockobj.Unlock();
	}
private:
	stLockPNone& m_unlockobj;
};
//------------------------------------------------------------------------
class CLD_AutoInterlockedIncDec:private zNoncopyable
{
public:
	CLD_AutoInterlockedIncDec(long* paddr)
		:m_pref(paddr)
	{
		if (m_pref)
		{
			InterlockedIncrement(m_pref);
		}		
	}
	~CLD_AutoInterlockedIncDec()
	{
		if (m_pref)
		{
			InterlockedDecrement(m_pref);
		}
	}
private:
	long* m_pref;
};
//------------------------------------------------------------------------
typedef CLD_CCriticalSection				CIntLock;		
typedef CLD_CMutex							CInpLock;		
//------------------------------------------------------------------------
typedef CLD_AutoInfoIntLock					CAIntLock;		
typedef CLD_AutoInfoIntLockTime				CAInpLock;		
//------------------------------------------------------------------------
#define INFOLOCK(a)							(a).Lock()
#define UNINFOLOCK(a)						(a).Unlock()
//------------------------------------------------------------------------
#define AILOCKT(a)							CAIntLock tAutoInfoIntLock((a))
#define AILOCKP(a)							CAInpLock tAutoInfoIntLockMutex((a))
//------------------------------------------------------------------------