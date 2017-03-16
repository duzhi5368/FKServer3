/**
*	created:		2013-4-7   22:10
*	filename: 		FKRandomGenerator
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <windows.h>
#include <WinCrypt.h>
//------------------------------------------------------------------------
class CTrueRandomGenerator
{
public:
	CTrueRandomGenerator();
	~CTrueRandomGenerator();
public:
	__int64 Generate64();
	UINT32  Generate32();
private:
	HCRYPTPROV m_hCrypt;
};
//------------------------------------------------------------------------
extern CTrueRandomGenerator* GetRandomGenerator();
//------------------------------------------------------------------------