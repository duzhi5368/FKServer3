/**
*	created:		2013-4-6   23:48
*	filename: 		FKMd5
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#ifndef UINT4
typedef unsigned long int UINT4;
#endif
//------------------------------------------------------------------------
typedef unsigned char MD5_DIGEST[16];
//------------------------------------------------------------------------
typedef struct {
	UINT4 i[2];                    
	UINT4 buf[4];                                     
	unsigned char in[64];                               
	MD5_DIGEST digest;      
} MD5_CTX;
//------------------------------------------------------------------------
static void MD5_Transform (UINT4 *buf, UINT4 *in);
//------------------------------------------------------------------------
void MD5Init(MD5_CTX *mdContext, unsigned long pseudoRandomNumber = 0);
void MD5Update(MD5_CTX *mdContext, unsigned char *inBuf, unsigned int inLen);
void MD5Final(MD5_CTX *mdContext,MD5_DIGEST* pMd5=0);
//------------------------------------------------------------------------