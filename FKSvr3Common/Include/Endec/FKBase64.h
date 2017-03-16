/**
*	created:		2013-4-7   21:16
*	filename: 		FKBase64
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKBaseDefine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//------------------------------------------------------------------------
struct stBase64Setup
{
	char base64_map[65];
	char base64_mod;
	BYTE base64_decode_map[256];
public:
	stBase64Setup();
public:
	void init_base64_map(char* pmap,char mod);

	int base64_encode_size(int src_len);
	int base64_decode_size(char *src, int src_len);

	int base64_encode(char *src, int src_len, char *dst,int dst_len);
	int base64_decode(char *src, int src_len, char *dst,int dst_len);
};
//------------------------------------------------------------------------
extern stBase64Setup g_base64;
//------------------------------------------------------------------------
int base64_encode(char *src, int src_len, char *dst,int dst_len);
int base64_decode(char *src, int src_len, char *dst,int dst_len);

int base64_encode_size(char *src, int src_len);
int base64_decode_size(char *src, int src_len);
//------------------------------------------------------------------------