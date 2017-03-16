/**
*	created:		2013-4-6   23:51
*	filename: 		FKMd5Ex
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/Endec/FKMd5Ex.h"
#include <stdio.h>
#include <string.h>
#include "../Include/FKVsVer8Define.h"
//------------------------------------------------------------------------
bool MD5Data(const void* pData,size_t size,MD5_DIGEST *pMD5  )
{
	MD5_CTX context;
	MD5Init (&context);
	MD5Update (&context, (unsigned char*)pData, size);
	MD5Final (&context,pMD5);
	return true;
}
//------------------------------------------------------------------------
bool MD5File(const char* pszFile,MD5_DIGEST *pMD5  )
{
	FILE* fp =0; 
	errno_t err = fopen_s(&fp,pszFile,"rb");
	if(!fp || err!=0) return false;

	unsigned char buffer[8192];
	size_t len;
	fseek(fp,0,SEEK_END);
	len = ftell(fp);
	fseek(fp,0,SEEK_SET);

	MD5_CTX context;
	MD5Init (&context);
	while( len ){
		size_t readLen = (len < 8192 ? len : 8192);
		if(1 != fread(buffer,readLen,1,fp)){
			MD5Final (&context,pMD5);
			if (fp){fclose(fp);fp=NULL;}
			return false;
		}
		len -= readLen;
		MD5Update (&context, buffer, readLen);
	}
	MD5Final (&context,pMD5);
	if (fp){fclose(fp);fp=NULL;}
	return true;
}
//------------------------------------------------------------------------
bool MD5String(const char* string,MD5_DIGEST* pMD5  )
{
	size_t size = strlen(string);
	return MD5Data(string,size,pMD5);
}
//------------------------------------------------------------------------