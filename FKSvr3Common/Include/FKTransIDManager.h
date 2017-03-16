/**
*	created:		2013-4-9   16:17
*	filename: 		FKTransIDManager
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#ifndef _WIN32
#include <pthread.h>
#include <ext/pool_allocator.h>
#endif
//------------------------------------------------------------------------
#include "FKSyncObjLock.h"
#include "STLTemplate/FKSyncList.h"
#include "FKError.h"
#include "FKNoncopyable.h"
//------------------------------------------------------------------------
#define _MAX_TRANSID_  ((TID)-1)
//------------------------------------------------------------------------
template <typename TID,class T>
class zTransidManage:private zNoncopyable,public CIntLock
{
public:
	typedef std::map<TID,T>  transmap;
	typedef typename transmap::iterator	 iterator;
protected:
	TID maxID;
	TID minID;
	TID curMaxID;

	transmap  m_transmap;

	void init(TID _safe_min,TID _safe_max){
		minID=_safe_min;
		maxID=_safe_max;
		curMaxID=minID;
		if (_safe_min<=0 || _safe_max<0 || _safe_min>=_safe_max){
			throw CLDError( "zTransidManage::init() 初始化参数错误" );
		}
	}
public:
	zTransidManage(){
		init(1,_MAX_TRANSID_);
	}

	zTransidManage(TID startID){
		init(startID,_MAX_TRANSID_);
	}

	zTransidManage(TID startID,TID endID){
		init(startID,endID);
	}

	virtual TID genTransid(){
		TID nretid=0;
		do {
			AILOCKT(*this);
			if (curMaxID>=maxID){ curMaxID=minID; }
			nretid=curMaxID;
			curMaxID++;
		} while (false);
		return nretid;
	}

	iterator begin(){
		return m_transmap.begin();
	}

	iterator end(){
		return m_transmap.end();
	}

	transmap& getmap(){
		return m_transmap;
	}

	bool put(const T& value,TID& i64Transid){
		i64Transid=genTransid();
		AILOCKT(*this);
		if (i64Transid!=0){	
			iterator it=m_transmap.find(i64Transid);
			if (it==m_transmap.end()){
				m_transmap[i64Transid]=value;
				return true;
			}
		}
		return false;
	}
	bool get(TID i64Transid,T& value){
		AILOCKT(*this);
		iterator it=m_transmap.find(i64Transid);
		if (it!=m_transmap.end()){
			value=it->second;
			return true;
		}
		return false;
	}
	bool remove_get(TID i64Transid,T& value){
		AILOCKT(*this);
		iterator it=m_transmap.find(i64Transid);
		if (it!=m_transmap.end()){
			value=it->second;
			m_transmap.erase(it);
			return true;
		}
		return false;
	}

	bool remove(TID i64Transid){
		AILOCKT(*this);
		iterator it=m_transmap.find(i64Transid);
		if (it!=m_transmap.end()){
			m_transmap.erase(it);
			return true;
		}
		return false;
	}

	void clear(){	AILOCKT(*this);m_transmap.clear(); }
	size_t size(){ AILOCKT(*this);return m_transmap.size(); }
};
//------------------------------------------------------------------------