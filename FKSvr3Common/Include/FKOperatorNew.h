/**
*	created:		2013-4-9   20:06
*	filename: 		FKOperatorNew
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------
#pragma warning(disable:4291)		
//------------------------------------------------------------------------
#define _poolallocator_(T)		safe_lookaside_allocator< T >
//------------------------------------------------------------------------
#define  DEC_OP_NEW(T)			private:				\
	static _poolallocator_(T)	 m_##T##allocator;	\
public:					\
	static long	m_nrefcount;			\
	static void* operator  new(size_t n){	\
	if (n==sizeof(T)){ InterlockedIncrement(&m_nrefcount);return LOOKASIDE_GETMEM(m_##T##allocator); }else{ return ::operator new(n); }; \
}	\
	template<class __P1>	\
	static void* operator  new(size_t n,__P1 p1){	\
	if (n==sizeof(T)){ InterlockedIncrement(&m_nrefcount);return LOOKASIDE_GETMEM(m_##T##allocator); }else{ return ::operator new(n); }; \
}	\
	template<class __P1,class __P2>	\
	static void* operator  new(size_t n,__P1 p1,__P2 p2){	\
	if (n==sizeof(T)){ InterlockedIncrement(&m_nrefcount);return LOOKASIDE_GETMEM(m_##T##allocator); }else{ return ::operator new(n); }; \
}	\
	template<class __P1,class __P2,class __P3>	\
	static void* operator  new(size_t n,__P1 p1,__P2 p2,__P3 p3){	\
	if (n==sizeof(T)){ InterlockedIncrement(&m_nrefcount);return LOOKASIDE_GETMEM(m_##T##allocator); }else{ return ::operator new(n); }; \
}	\
	template<class __P1,class __P2,class __P3,class __P4>	\
	static void* operator  new(size_t n,__P1 p1,__P2 p2,__P3 p3,__P4 p4){	\
	if (n==sizeof(T)){ InterlockedIncrement(&m_nrefcount);return LOOKASIDE_GETMEM(m_##T##allocator); }else{ return ::operator new(n); }; \
}	\
	static void operator  delete(void *p, size_t n){	\
	if(p){ if (n==sizeof(T)){ InterlockedDecrement(&m_nrefcount);m_##T##allocator.freemem((T*)p); }else{ return ::operator delete(p); }; };\
} 	
//------------------------------------------------------------------------
#define  IMP_OP_NEW(T)	_poolallocator_(T) T::m_##T##allocator;		long T::m_nrefcount=0;		
//------------------------------------------------------------------------