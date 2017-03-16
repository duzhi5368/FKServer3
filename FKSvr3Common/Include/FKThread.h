/**
*	created:		2013-3-22   19:58
*	filename: 		FKThread
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#pragma comment (lib, "Kernel32.lib")
//------------------------------------------------------------------------		
#include "FKNoncopyable.h"
#include "FKBaseDefine.h"
#include "FKError.h"
#include <map>
//------------------------------------------------------------------------
#define _TH_VAR(T,V)							__declspec(thread) T V;
#define _TH_VAR_INIT(T,V,code)					__declspec(thread) T V;
#define _TH_VAR_ARRAY(T,V,count)				__declspec(thread) T V[count];
#define _TH_VAR_INIT_ARRAY(T,V,count,initobj)	__declspec(thread) T V[count]={ initobj };
#define _TH_VAR_GET(V)							V	
#define _TH_VAR_PTR(V)							&V
#define _TH_VAR_SET(V,A)						V=(A);
#define _TH_VAR_SET_PTR(A)		
#define _TH_VAR_INDEX(V)						0
#define _TH_VAR_SIZEOF(V)						sizeof(V)		
#define _TH_VAR_COUNT_OF(V)						COUNT_OF(V)		
#define _TH_VAR_FREE			
//------------------------------------------------------------------------
typedef unsigned int(__stdcall *pThreadFunc)(void*);
//------------------------------------------------------------------------
#define smBeginThread(psa,cbStack,pfnStartAddr,pvParam, dwCreate,pdwThreadId)  (HANDLE)_beginthreadex(psa,(unsigned int)cbStack,(pThreadFunc)pfnStartAddr,(void*)pvParam,dwCreate,(unsigned*)pdwThreadId);
//------------------------------------------------------------------------
class CLD_ThreadBase:private zNoncopyable
{
private:
	static unsigned  int __stdcall ThreadProxy( void *pvParam );
protected:
	virtual int runthread(){return 0;};
	virtual void OnTerminate(){return;};
protected:
	HANDLE m_hThread;
	int  m_ThreadId;
	bool m_Terminated;
	bool m_boSuspended;
	bool m_boDelThisOnTerminate;
	int m_dwExitCode;
	int m_nPriority;
	int m_nDestroyTimeOut;
protected:
	CLD_ThreadBase(bool boCreateSuspended , bool boDelThisOnTerminate=false);
public:
	__int64 m_i64Kerneltime;
	__int64 m_i64Usertime;

	enum _DESTROY_TYPE
	{
		_dtWAIT=0,			
		_dtTRY,                         
		_dtTERMINATE,			 
	};      

	~CLD_ThreadBase(void);     

	bool Start(bool boCreateSuspended=false);
	bool SetPriority(int pri);
	virtual bool Suspend();
	virtual bool Resume(); 
	bool IsRunning(void);
	void Terminate();  
	bool IsTerminated();  
	int Waitfor(DWORD const dwWaitTimes=INFINITE);
	int TerminateForce(DWORD dwExitCode=0);
	int Destroy(_DESTROY_TYPE dt,DWORD & dwExitCode,DWORD const dwWaitTimes=INFINITE);
	DWORD GetExitCode();
	HANDLE GetHandle();
	DWORD GetId();
	bool getruntime();
public:
	static CLD_ThreadBase* getThreadObj();
	static DWORD getcurid(){return ::GetCurrentThreadId();};
	static HANDLE getcurhandle(){return ::GetCurrentThread();};
};
//------------------------------------------------------------------------
struct stThreadObjInfo{
	CLD_ThreadBase* pCurrThreadObj;
	DWORD ThreadId;
	DWORD PID;
};
//------------------------------------------------------------------------
extern _TH_VAR(stThreadObjInfo,tls_CurrThreadObj);
//------------------------------------------------------------------------
template<class _BTP1>
class CLDThread:public CLD_ThreadBase
{
public:
	CLDThread(bool boCreateSuspended , _BTP1 pParam,bool boDelThisOnTerminate=false)
		:m_BindParam(pParam),CLD_ThreadBase(boCreateSuspended,boDelThisOnTerminate){
	}
	bool Start(bool boCreateSuspended,_BTP1 pParam){
		m_BindParam=pParam;
		return CLD_ThreadBase::Start(boCreateSuspended);
	}
	virtual int Run(_BTP1 pvParam){return 0;};
	virtual int Run(CLD_ThreadBase* thread_this,_BTP1 pvParam){return Run(pvParam);};
protected:
	_BTP1 m_BindParam;
private:
	virtual int runthread(){return Run(this,m_BindParam);}
};
//------------------------------------------------------------------------
typedef CLDThread<void*> CThread;
//------------------------------------------------------------------------
#define BINDTHFUNC(funcname,pT,pname) unsigned int __stdcall funcname(CLD_ThreadBase* pthread,pT pname)
#define BINDTHOBJFUNC(OT,funcname,pT,pname) unsigned int __stdcall OT##funcname(CLD_ThreadBase* pthread,pT pname)
//------------------------------------------------------------------------
template<class _BTP1>
class CBindThread:public CLDThread<_BTP1>
{
public:
	typedef unsigned int(__stdcall *PBTRFunc)(CLD_ThreadBase*,_BTP1);

	CBindThread(PBTRFunc func,bool boCreateSuspended,_BTP1 pvParam,bool boDelThisOnTerminate=false)
		:m_func(func),CLDThread<_BTP1>(boCreateSuspended,pvParam,boDelThisOnTerminate)
	{
	}
	bool Bind(PBTRFunc func,bool boCreateSuspended,_BTP1 pvParam){
		if (func){
			if (!m_func)	{m_func=func;};
			return CLDThread<_BTP1>::Start(boCreateSuspended,pvParam);
		}
		return false;
	}
	virtual int Run(CLD_ThreadBase* thread_this,_BTP1 pvParam){
		if (m_func)	{return m_func(thread_this,pvParam);}
		return 0;
	}
protected:
	virtual void OnTerminate(){m_func=NULL;	}
	PBTRFunc m_func;
};
//------------------------------------------------------------------------
enum eBindThreadObjAutoDelType{
	eNotDel,
	eDelOnTerminate,
	eDelOnDestroy,
};
//------------------------------------------------------------------------
template<class _BTOBJ , class _BTP1>
class CBindObjThread:public CLDThread<_BTP1>
{
public:
	typedef unsigned int (__stdcall _BTOBJ::*PBOTRFunc)(CLD_ThreadBase*,_BTP1);

	CBindObjThread(_BTOBJ* theobj, PBOTRFunc func,bool boCreateSuspended,_BTP1 pvParam,
		eBindThreadObjAutoDelType eDelBindObjType=eNotDel,bool boDelThisOnTerminate=false)
		:m_obj(theobj),m_func(func),m_eDelBindObj(eDelBindObjType),CLDThread<_BTP1>(boCreateSuspended,pvParam,boDelThisOnTerminate)
	{
	}
	virtual ~CBindObjThread(){
		if(m_eDelBindObj==eDelOnDestroy && m_obj){SAFE_DELETE(m_obj);};
	}
	bool Bind(_BTOBJ* theobj,PBOTRFunc func,bool boCreateSuspended,_BTP1 pvParam){
		if (func){
			if (!m_func)	{m_func=func;};
			if (!m_obj)	{m_obj=theobj;};
			return CLDThread<_BTP1>::Start(boCreateSuspended,pvParam);
		}
		return false;
	}
	virtual int Run(CLD_ThreadBase* thread_this,_BTP1 pvParam){
		if (m_func && m_obj)	{return (m_obj->*m_func)(thread_this,pvParam);	}
		return 0;
	}
protected:
	virtual void OnTerminate(){
		m_func=NULL;
		if(m_eDelBindObj==eDelOnTerminate && m_obj){SAFE_DELETE(m_obj);};
		m_obj=NULL;
	}
	PBOTRFunc m_func;
	_BTOBJ* m_obj;
	eBindThreadObjAutoDelType m_eDelBindObj;
};
//------------------------------------------------------------------------
class CThreadFactory
{
public:
	template<class _BTP1>
	static CLDThread<_BTP1>* Create(_BTP1 pvParam,bool boCreateSuspended=true,bool boDelThisOnTerminate=false)
	{
		CLDThread<_BTP1>* pt=new CLDThread<_BTP1>(boCreateSuspended,pvParam,boDelThisOnTerminate);
		return (pt);
	}

	template<class _BTP1>
	static CBindThread<_BTP1>* CreateBind(typename CBindThread<_BTP1>::PBTRFunc func,
		_BTP1 pvParam,bool boCreateSuspended=true,bool boDelThisOnTerminate=false)
	{
		CBindThread<_BTP1>* pt=new CBindThread<_BTP1>(func,boCreateSuspended,pvParam,boDelThisOnTerminate);
		return (pt);
	}

	template<class _BTOBJ , class _BTP1>
	static CBindObjThread<_BTOBJ,_BTP1>* CreateBindClass(_BTOBJ* theobj, 
		typename CBindObjThread<_BTOBJ,_BTP1>::PBOTRFunc func,_BTP1 pvParam,
		bool boCreateSuspended=true,
		eBindThreadObjAutoDelType eDelBindObjType = eNotDel,
		bool boDelThisOnTerminate=false)
	{
		CBindObjThread<_BTOBJ,_BTP1>* pt=new CBindObjThread<_BTOBJ,_BTP1>(theobj,func,
			boCreateSuspended,pvParam,eDelBindObjType,boDelThisOnTerminate);
		return (pt);
	}
};
//------------------------------------------------------------------------