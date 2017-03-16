/**
*	created:		2013-4-8   15:31
*	filename: 		FKLookasideAlloc
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/STLTemplate/FKLookasideAlloc.h"
#include "../Include/FKLogger.h"
#include "../Include/boost/pool/singleton_pool.hpp"
//------------------------------------------------------------------------
using namespace boost;
using namespace std;
//------------------------------------------------------------------------
const DWORD c_dwLookasideAllocFreeTime = 10 * 60 * 1000;
size_t gLookasideAllocSize = 0;
const bool _UserNewAllocater = true;
//------------------------------------------------------------------------
struct poolTarget128{};
struct poolTarget256{};
struct poolTarget512{};
struct poolTarget512_2{};
struct poolTarget512_3{};
struct poolTarget512_4{};
struct poolTarget512_5{};

typedef singleton_pool<poolTarget128, 128> ipool1;
typedef singleton_pool<poolTarget256, 256> ipool2;
typedef singleton_pool<poolTarget512, 521> ipool3;
typedef singleton_pool<poolTarget512_2, 521> ipool4;
typedef singleton_pool<poolTarget512_3, 521> ipool5;
typedef singleton_pool<poolTarget512_4, 521> ipool6;
typedef singleton_pool<poolTarget512_5, 521> ipool7;
//------------------------------------------------------------------------
char* CSimpleAllocator::getmem(int n){
	return allocate(n);
}
void CSimpleAllocator::freemem(void* p){
	deallocate(p);
}
//------------------------------------------------------------------------
char* CSimpleAllocator::allocate(int n){
	int nbufsize=(int)ROUNDNUM2(n+1,4);
	BYTE* pret=NULL;
	if (nbufsize<=128){
		pret=(BYTE*)( _ty_alloc_0_128.allocate( (BYTE)nbufsize,NULL ) );
		*pret=(BYTE)nbufsize;
	}else if (nbufsize<=256){
		pret=(BYTE *)LOOKASIDE_GETMEM(_ty_alloc_128);
		*pret=(BYTE)(128+1);
	}else if (nbufsize<=512){
		pret=(BYTE *)LOOKASIDE_GETMEM(_ty_alloc_256);
		*pret=(BYTE)(128+2);
	}else if (nbufsize<=512*2){
		pret=(BYTE *)LOOKASIDE_GETMEM(_ty_alloc_512);
		*pret=(BYTE)(128+3);
	}else if (nbufsize<=512*3){
		pret=(BYTE *)LOOKASIDE_GETMEM(_ty_alloc_512x2);
		*pret=(BYTE)(128+4);
	}else if (nbufsize<=512*4){
		pret=(BYTE *)LOOKASIDE_GETMEM(_ty_alloc_512x3);
		*pret=(BYTE)(128+5);
	}else if (nbufsize<=512*5){
		pret=(BYTE *)LOOKASIDE_GETMEM(_ty_alloc_512x4);
		*pret=(BYTE)(128+6);
	}else{
		pret=(BYTE *)malloc(nbufsize);
		*pret=(BYTE)(0xff);
	}
	pret++;

	return (char*)pret;
}	
//------------------------------------------------------------------------
void CSimpleAllocator::deallocate(void* p)
{
	BYTE* pret=(BYTE*)p;
	pret--;
	int nbufsize=(BYTE)(*pret);

	switch (nbufsize){
case (128+1):	_ty_alloc_128.freemem(pret); break;
case (128+2):	_ty_alloc_256.freemem(pret); break;
case (128+3):	_ty_alloc_512.freemem(pret); break;
case (128+4):	_ty_alloc_512x2.freemem(pret); break;
case (128+5):	_ty_alloc_512x3.freemem(pret); break;
case (128+6):	_ty_alloc_512x4.freemem(pret); break;
case 0xff:		free(pret);break;
default:
	{
		if (nbufsize<=128){
			_ty_alloc_0_128.deallocate((char*)pret,nbufsize);
		}else{
			g_logger.error("CSimpleAllocator::deallocate:´íÎóµÄÖ¸Õë!");
		}
	}
	break;
	}
}
//------------------------------------------------------------------------