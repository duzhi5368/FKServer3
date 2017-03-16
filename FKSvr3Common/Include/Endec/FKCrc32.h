/**
*	created:		2013-4-6   23:58
*	filename: 		FKCrc32
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
void crc32Init(unsigned long *pCrc32);
void crc32Update(unsigned long *pCrc32, unsigned char *pData, unsigned long uSize);
void crc32Finish(unsigned long *pCrc32);
//------------------------------------------------------------------------
__inline unsigned long crc32data(unsigned char *pData, unsigned long uSize){
	unsigned long ret;
	crc32Init(&ret);
	crc32Update(&ret,pData,uSize);
	crc32Finish(&ret);
	return ret;
}
//------------------------------------------------------------------------