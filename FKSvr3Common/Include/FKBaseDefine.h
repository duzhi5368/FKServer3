/**
*	created:		2013-3-22   19:06
*	filename: 		FKBaseDefine
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#ifdef _WIN32

	#pragma warning(disable:4201)
	#pragma warning(disable:4996)
	#pragma warning(disable:4100)
	#pragma warning(disable:4511)
	#pragma warning(disable:4512)
	#pragma warning(disable:4127)
	#pragma warning(disable:4005)			
	#pragma warning(disable:4510)
	#pragma warning(disable:4610)
	#pragma warning(disable:4244)

	#ifndef _WIN32_WINNT 
		#define _WIN32_WINNT 0x500 
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN	
	#endif		

	#include <Windows.h>

	#define bzero ZeroMemory
	#define bcopy(a,b,c) memcpy((b),(a),(c))

	#define strtoull	_strtoui64 
	#define strtoll		_strtoi64

	#define atoll		_atoi64
	#define atoull		(unsigned __int64)_atoi64

	#define strcasecmp	_stricmp
	#define stricmp     _stricmp
	#define lltostr		_i64toa
	#define ulltostr	_ui64toa

#endif	
//------------------------------------------------------------------------
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef signed short SWORD;
#ifndef _WIN32
	typedef unsigned int DWORD;
#endif
typedef signed int SDWORD;
typedef unsigned long long QWORD;
typedef signed long long SQWORD;
//------------------------------------------------------------------------
#define _abs(a)		(((a) < (0)) ? (-a) : (a))
#define _swap(a, b)	((a) ^= (b) ^= (a) ^= (b))

#define SAFE_DELETE(x) { if (x) { delete (x); (x) = NULL; } }
#define SAFE_DELETE_VEC(x) { if (x) { delete [] (x); (x) = NULL; } }
#define SAFE_DELETEARRAY(x)		SAFE_DELETE_VEC(x)
#define SAFE_RELEASE(p)  if(p) { (p)->Release(); (p) = NULL; }
#define SAFE_EXEC_NULL(p,code)	if(p) { code; (p) = NULL; }
#define COUNT_OF(X) (sizeof(X)/sizeof((X)[0]))
#define count_of(X) COUNT_OF(X)
#define MAKEBIT(x) (1<<(x))
#define BIT(X)  MAKEBIT(X)

#define FILLOBJ(p,fillvalue)		memset( p, (BYTE)(fillvalue), sizeof( *(p) ))
#define ZEROOBJ(p)					bzero(p,sizeof(*(p)));

#define ZEROMB2MB(p,m1,m2)			bzero(&(p)->m1,	((char*)(&(p)->m2))-((char*)(&(p)->m1))+sizeof((p)->m2)	);	

#define MEMBEROFFSET(_st,_mem)		(DWORD)(&((_st*)(NULL))->##_mem)

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |   \
	((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif

#define GETCURREIP(x)	DWORD x=NULL; {	 \
	__asm push eax			\
	__asm call __curreip##x		\
	__asm __curreip##x:		\
	__asm pop eax		\
	__asm mov x,eax		\
	__asm pop eax }		


#define TOSTR(x)  #x
#define TOSTR1(x)      TOSTR(x)
#define __FUNC_LINE__	"("TOSTR1(__LINE__)"):"__FUNCTION__
#define __FILE_LINE__	__FILE__"("TOSTR1(__LINE__)")"
#define __FILE_FUNC__	__FILE__":"__FUNCTION__
#define __FF_LINE__		__FILE__"("TOSTR1(__LINE__)"):"__FUNCTION__
#define __FF_LINE_T__	__FILE__"("TOSTR1(__LINE__)")["__TIMESTAMP__"]:"__FUNCTION__

#define STATIC_ASSERTMN2(p1,p2,p3,p4)		
#define ROUNDNUM2(value,num)		( ((value) + ((num)-1)) & (~((num)-1)) )
//------------------------------------------------------------------------
template< typename _dstproc,typename _srcproc >
__inline _dstproc ForceConvert( _dstproc dst_proc,_srcproc src_proc ){
	union{
		_srcproc _f;
		_dstproc _t;
	}ut;
	ut._f = src_proc;
	dst_proc=ut._t;
	return ut._t;
}

template< typename _dstproc,typename _srcproc >
__inline _dstproc ForceConvert(_srcproc src_proc ){
	union{
		_srcproc _f;
		_dstproc _t;
	}ut;
	ut._f = src_proc;
	return ut._t;
}

#define  ProcConvert	ForceConvert

template<class _Ty> inline
const _Ty& safe_min(const _Ty& _Left, const _Ty& _Right){	
	return (_Right < _Left ? _Right : _Left);
}

template<class _Ty> inline
const _Ty& safe_max(const _Ty& _Left, const _Ty& _Right){	
	return (_Left < _Right ? _Right : _Left);
}

#define _STACK_AUTOEXEC_(name,initcode,uninitcode)		struct name{ name(){ initcode;}; ~##name(){ uninitcode;}; };

struct stAutoAlloc{	char* m_p;	stAutoAlloc(){m_p=NULL;};~stAutoAlloc(){SAFE_DELETE_VEC(m_p);};};

#define STACK_ALLOCA(t,p,n)		t p=NULL;int sas##p=(safe_max<int>(n,16)*sizeof(p[0]));stAutoAlloc aac##p;if (sas##p>32*1024)	\
{p=(t)(new char[sas##p]);aac##p.m_p=(char*)p;*((DWORD*)p)=0; }else{p=(t)(new char[sas##p]);aac##p.m_p=(char*)p;*((DWORD*)p)=0;};\

#define ZSTACK_ALLOCA(t,p,n)		STACK_ALLOCA(t,p,n);if(p){ZeroMemory(p,sas##p);};

//------------------------------------------------------------------------
#pragma pack(push,1)
//------------------------------------------------------------------------
template <typename _DT,int _SIZE>
struct	stConstArray
{
private:
	_DT thearray[_SIZE];
public:
	_DT& operator []  (int  index){return thearray[index];}
	const _DT& operator[] (int index) const {return thearray[index];}
};
//------------------------------------------------------------------------
template<typename T>
class Typedef2BASE{
public:
	typedef T BASE;
};
//------------------------------------------------------------------------
template <typename _DT1,typename _DT2> class stlink2{ _DT1 _p1;_DT2 _p2;};
template <typename _DT1,typename _DT2,typename _DT3> class stlink3{ _DT1 _p1;_DT2 _p2;_DT3 _p3;};
template <typename _DT1,typename _DT2,typename _DT3,typename _DT4> class stlink4{ _DT1 _p1;_DT2 _p2;_DT3 _p3;_DT4 _p4;};
template <typename _DT1,typename _DT2,typename _DT3,typename _DT4,typename _DT5> class stlink5{ _DT1 _p1;_DT2 _p2;_DT3 _p3;_DT4 _p4;_DT5 _p5;};

template<class _A,class _P1,class _P2> struct stunion2{ union{ struct { _P1 _p1;_P2 _p2; };  _A _value;  }; };
template<class _A,class _P1,class _P2,class _P3> struct stunion3{ union{ struct { _P1 _p1;_P2 _p2;_P3 _p3; };  _A _value;  }; };
template<class _A,class _P1,class _P2,class _P3,class _P4> struct stunion4{ union{ struct { _P1 _p1;_P2 _p2;_P3 _p3;_P4 _p4; };  _A _value;  }; };
template<class _A,class _P1,class _P2,class _P3,class _P4,class _P5> struct stunion5{ union{ struct { _P1 _p1;_P2 _p2;_P3 _p3;_P4 _p4;_P5 _p5; };  _A _value;  }; };
template<class _A,class _P1,class _P2,class _P3,class _P4,class _P5,class _P6> struct stunion6{ union{ struct { _P1 _p1;_P2 _p2;_P3 _p3;_P4 _p4;_P5 _p5;_P6 _p6; };  _A _value;  }; };
template<class _A,class _P1,class _P2,class _P3,class _P4,class _P5,class _P6,class _P7> struct stunion7{ union{ struct { _P1 _p1;_P2 _p2;_P3 _p3;_P4 _p4;_P5 _p5;_P6 _p6;_P7 _p7; };  _A _value;  }; };
template<class _A,class _P1,class _P2,class _P3,class _P4,class _P5,class _P6,class _P7,class _P8> struct stunion8{ union{ struct { _P1 _p1;_P2 _p2;_P3 _p3;_P4 _p4;_P5 _p5;_P6 _p6;_P7 _p7;_P8 _p8; };  _A _value;  }; };
//------------------------------------------------------------------------
#pragma pack(pop)
//------------------------------------------------------------------------