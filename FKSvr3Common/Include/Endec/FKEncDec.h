/**
*	created:		2013-4-6   23:47
*	filename: 		FKEncDec
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <stdlib.h>
#include "FKMd5Ex.h"
#include "FKDes.h"
#include "FKCrc16.h"
#include "FKCrc32.h"
//------------------------------------------------------------------------
#define  RC5_8_ROUNDS  8
//------------------------------------------------------------------------
class CEncrypt
{
public:
	CEncrypt();
	enum encMethod{
		ENCDEC_NONE=0,
		ENCDEC_RC5=1,
		ENCDEC_DES=2,
		ENCDEC_FORCE_DWORD=0x7fffffff,
	};
	void random_key_des(DES_cblock *ret);
	void set_key_des(const_DES_cblock *key);
	void set_key_rc5(const unsigned char *data, int nLen, int rounds = RC5_8_ROUNDS);

	int encdec(void *data, unsigned int nLen, bool enc);

	void setEncMethod(encMethod method);
	encMethod getEncMethod() const;
private:
	void DES_encrypt1(DES_LONG *data, DES_key_schedule *ks, int enc);
	int encdec_des(unsigned char *data, unsigned int nLen, bool enc);
private:
	encMethod method;
	DES_key_schedule key_des;
	bool haveKey_des;
};
//------------------------------------------------------------------------