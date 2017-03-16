/**
*	created:		2013-4-7   22:54
*	filename: 		FKSingleton
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKSyncObjLock.h"
//------------------------------------------------------------------------
template <typename T>
class SingletonFactory{
public:
	static T* instance(){
		return new T();
	}
};
//------------------------------------------------------------------------
template <typename T, typename MANA = SingletonFactory<T> > 
class Singleton{
private:
	Singleton(const Singleton&);
	const Singleton & operator= (const Singleton &);
protected:
	static CIntLock m_singletonlock;
	static T* ms_Singleton;
	Singleton( void ){
	}

	~Singleton( void ){
	}
public:

	static __inline void delMe(void){

		if (ms_Singleton){
			AILOCKT(m_singletonlock);
			if (ms_Singleton){
				delete ms_Singleton;
				ms_Singleton = 0;
			}
		}
	}

	static __inline T* instance( void ){
		if (!ms_Singleton){
			AILOCKT(m_singletonlock);
			if (!ms_Singleton){
				ms_Singleton = MANA::instance();
			}
		}
		return ms_Singleton;
	}

	static __inline T* instance_readonly( void ){
		return ms_Singleton;
	}

	static __inline T& getMe(void){
		return *instance();
	}
};
//------------------------------------------------------------------------
template <typename T, typename MANA>
T* Singleton<T,MANA>::ms_Singleton = 0;
//------------------------------------------------------------------------
template <typename T, typename MANA>
CIntLock Singleton<T,MANA>::m_singletonlock;
//------------------------------------------------------------------------