/**
*	created:		2013-4-7   22:12
*	filename: 		FKRandomGenerator
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKRandomGenerator.h"
#include <stdlib.h>
//------------------------------------------------------------------------
CTrueRandomGenerator::CTrueRandomGenerator() 
: m_hCrypt( NULL )
{
	srand(GetTickCount() + 230912);

	int ret = 1;

	if( !CryptAcquireContext( &m_hCrypt, NULL, NULL, PROV_RSA_FULL, 0) ) 
	{
		ret = CryptAcquireContext( &m_hCrypt, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET);	
	}
	if( !ret )
		m_hCrypt = NULL;
}
//------------------------------------------------------------------------
CTrueRandomGenerator::~CTrueRandomGenerator()
{
	if( m_hCrypt )
	{
		CryptReleaseContext(m_hCrypt, 0);
		m_hCrypt = NULL;
	}
}
//------------------------------------------------------------------------
__int64 CTrueRandomGenerator::Generate64()
{
	if(m_hCrypt == NULL)
	{
		__int64 i = 3 * rand() << 3 + GetTickCount();
		return i;
	}

	BYTE data[sizeof(__int64)];
	__int64 r = 0;

	if( m_hCrypt )
	{
		if( CryptGenRandom( m_hCrypt, sizeof(data), data) ) 
		{
			memcpy( &r, data, sizeof(data) );
		}
	}
	return r > 0 ? r : -r;
}
//------------------------------------------------------------------------
UINT32 CTrueRandomGenerator::Generate32()
{
	if(m_hCrypt == NULL)
	{
		__int64 i = 3 * rand() << 3 + GetTickCount();
		return i & 0xFFFFFFFF;
	}

	BYTE data[sizeof(UINT32)];
	UINT32 r = 0;

	if( m_hCrypt )
	{
		if( CryptGenRandom( m_hCrypt, sizeof(data), data) ) 
		{
			memcpy( &r, data, sizeof(data) );
		}
	}
	return r;
}
//------------------------------------------------------------------------
CTrueRandomGenerator* GetRandomGenerator()
{
	static CTrueRandomGenerator g_TrueRandomGenerator;
	return &g_TrueRandomGenerator;
}
//------------------------------------------------------------------------