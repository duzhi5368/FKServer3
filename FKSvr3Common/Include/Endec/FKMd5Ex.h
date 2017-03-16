/**
*	created:		2013-4-6   23:50
*	filename: 		FKMd5Ex
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKMd5.h"
//------------------------------------------------------------------------
bool MD5Data(const void* pData,size_t size,MD5_DIGEST *pMD5  );
bool MD5File(const char* pszFile,MD5_DIGEST *pMD5  );
bool MD5String(const char* string,MD5_DIGEST* pMD5  );
//------------------------------------------------------------------------