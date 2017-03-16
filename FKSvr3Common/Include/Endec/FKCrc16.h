/**
*	created:		2013-4-6   23:56
*	filename: 		FKCrc16
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
void crc16_init(unsigned short *uCrc16);
void crc16_update(unsigned short *uCrc16, unsigned char *pBuffer, unsigned long uBufSize);
void crc16_final(unsigned short *uCrc16);

void crc16ccitt_init(unsigned short *uCcitt16);
void crc16ccitt_update(unsigned short *uCcitt16, unsigned char *pBuffer, unsigned long uBufSize);
void crc16ccitt_final(unsigned short *uCcitt16);
//------------------------------------------------------------------------
__inline unsigned short crc16data(unsigned char *pData, unsigned long uSize){
	unsigned short ret;
	crc16_init(&ret);
	crc16_update(&ret,pData,uSize);
	crc16_final(&ret);
	return ret;
}
//------------------------------------------------------------------------
__inline unsigned short crccit16data(unsigned char *pData, unsigned long uSize){
	unsigned short ret;
	crc16ccitt_init(&ret);
	crc16ccitt_update(&ret,pData,uSize);
	crc16ccitt_final(&ret);
	return ret;
}
//------------------------------------------------------------------------