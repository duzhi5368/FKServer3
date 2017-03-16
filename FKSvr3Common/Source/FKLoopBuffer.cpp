/**
*	created:		2013-4-6   23:02
*	filename: 		FKLoopBuffer
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/Network/FKLoopBuffer.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include "../Include/STLTemplate/FKSyncList.h"
#include "../Include/FKTimeMonitor.h"
//------------------------------------------------------------------------
#define _MIN_BUFFER_SIZE_		512
//------------------------------------------------------------------------
CLD_LoopBuf::CLD_LoopBuf(int nBufSize){
	FUNCTION_BEGIN;
	m_nBufSize=0;
	m_nLen=0;
	m_nIdleLen=0;
	m_pData=NULL;
	m_pIdle=NULL;
	m_boCanRead=true;
	m_boCanWrite=true;

	m_Buf=NULL;
	m_nBufSize=0;

	m_initBufSize=nBufSize;
	ReSize(nBufSize);
}
//------------------------------------------------------------------------
CLD_LoopBuf::~CLD_LoopBuf()
{
	FUNCTION_BEGIN;
	if(m_Buf!=NULL){
		__mt_char_alloc.deallocate(m_Buf);
		m_Buf=NULL;
	}
	m_nBufSize=0;
	m_nLen=0;
	m_nIdleLen=0;
	m_pData=NULL;
	m_pIdle=NULL;
}
//------------------------------------------------------------------------
void CLD_LoopBuf::Clear()
{
	FUNCTION_BEGIN;
	if (m_Buf){
		m_Buf[0]=0;
		m_Buf[m_nBufSize-1]=0;
		m_Buf[m_nBufSize]=0;
	}
	m_nLen=0;
	m_pData=m_Buf;

	m_nIdleLen=m_nBufSize;
	m_pIdle=m_Buf;
}
//------------------------------------------------------------------------
bool CLD_LoopBuf::ReSize(int nSize)
{
	int nNewBufSize=ROUNDNUM2(nSize+8+1,_MIN_BUFFER_SIZE_)-8;
	if (m_nLen<(nNewBufSize-1)){
		char* pNewBuf=__mt_char_alloc.allocate(nNewBufSize);

		nNewBufSize--;
		if(pNewBuf!=NULL){
			pNewBuf[0]=0;
			pNewBuf[nNewBufSize-1]=0;
			pNewBuf[nNewBufSize]=0;

			if (m_pData && m_nLen>0){
				MoveMemory(pNewBuf,m_pData,m_nLen+1);
			}

			char * _POldBuf = m_Buf;

			m_Buf=pNewBuf;
			m_nBufSize=nNewBufSize;

			m_pIdle=m_Buf+m_nLen;
			m_nIdleLen=m_nBufSize-m_nLen;
			m_pIdle[0]=0;

			m_pData=m_Buf;

			if( _POldBuf )
			{
				__mt_char_alloc.deallocate( _POldBuf );
			}

			return true;
		}
	}
	return false;
}
//------------------------------------------------------------------------
int  CLD_LoopBuf::GetDataBufLen(){
	return m_nLen;
}
//------------------------------------------------------------------------
char* CLD_LoopBuf::GetIdleBufPointer(){
	return m_pIdle;
}
//------------------------------------------------------------------------
void CLD_LoopBuf::SetIdleBufPointer(char* Value){
	move_Idle_pos(Value);
}
//------------------------------------------------------------------------
int CLD_LoopBuf::GetIdleBufLen(){
	return m_nIdleLen;
}
//------------------------------------------------------------------------
int CLD_LoopBuf::GetMaxSize(){
	return m_nBufSize;
}
//------------------------------------------------------------------------
char* CLD_LoopBuf::GetDataBufPointer(){
	return m_pData;
}
//------------------------------------------------------------------------
void CLD_LoopBuf::SetDataBufPointer(char* Value){
	move_data_pos(Value);
}
//------------------------------------------------------------------------
bool CLD_LoopBuf::move_data_pos(char* pData)
{
	FUNCTION_BEGIN;
	bool boRet=false;

	if (m_boCanRead){

		if(pData<m_Buf){
			pData=m_Buf;
		}
		if (pData>m_pIdle){
			pData=m_pIdle;
		}

		if(pData>m_pData){
			m_nLen=m_nLen-(pData-m_pData);
			m_pData=pData;	
		}else if(pData<m_pData){
			m_nLen=m_nLen+(m_pData-pData);
			m_pData=pData;	
		}
		boRet=true;
	}
	return boRet;
}
//------------------------------------------------------------------------
bool CLD_LoopBuf::move_Idle_pos(char* pData)
{
	FUNCTION_BEGIN;
	bool boRet=false;

	if (m_boCanWrite){

		if(pData<m_pData){
			pData=m_pData;
		}
		if (pData>(m_Buf+m_nBufSize)){
			pData=(m_Buf+m_nBufSize);
		}

		int nLen=0;
		if(pData>m_pIdle){
			nLen=pData-m_pIdle;
			m_nIdleLen=m_nIdleLen-nLen;
			m_pIdle=pData;

			m_nLen=m_nLen+nLen;	

			m_Buf[m_nBufSize]=0;
			m_pIdle[0]=0;
		}else if(pData<m_pIdle){
			nLen=m_pIdle-pData;
			m_nIdleLen=m_nIdleLen+nLen;
			m_pIdle=pData;

			m_nLen=m_nLen-nLen;	

			m_Buf[m_nBufSize]=0;
			m_pIdle[0]=0;
		}
		boRet=true;
	}
	return boRet;
}
//------------------------------------------------------------------------
bool CLD_LoopBuf::InitIdleBuf(int nSize,bool boForceInit)
{
	char* pRetBuf=NULL;

	nSize=safe_min<int>(nSize,64*1024);

	if(nSize<=m_nIdleLen){
		pRetBuf= m_pIdle;
	}else if (m_boCanWrite && m_boCanRead || boForceInit){
		if(nSize<=(m_nBufSize-m_nLen)){
			MoveMemory(m_Buf,m_pData,m_nLen+1);

			m_pIdle=m_Buf+m_nLen;
			m_nIdleLen=m_nBufSize-m_nLen;

			m_pData=m_Buf;

			pRetBuf= m_pIdle;
		}else if((nSize>0)&&(nSize>(m_nBufSize-m_nLen))){
			int nNewBufSize=m_nBufSize+m_nBufSize;
			while((nNewBufSize-m_nLen)<nSize){
				nNewBufSize=nNewBufSize+safe_max(m_nBufSize,256);
			}
			ReSize(nNewBufSize);
			pRetBuf=m_pIdle;
		}	
	}

	if(pRetBuf){
		pRetBuf[0]=0;
		pRetBuf[nSize]=0;
		return true;
	}
	return false;
}
//------------------------------------------------------------------------