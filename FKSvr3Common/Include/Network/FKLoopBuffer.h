/**
*	created:		2013-4-6   22:58
*	filename: 		FKLoopBuffer
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKBaseDefine.h"
#include "../FKSyncObjLock.h"
//------------------------------------------------------------------------
#define DEF_LOOPBUF_SIZE               1024*5+16-1	
#define LOOPBUF_EXTEND_SIZE            64-1
//------------------------------------------------------------------------
class CLD_LoopBuf
{
public:
	CLD_LoopBuf(int nBufSize=DEF_LOOPBUF_SIZE);
	virtual ~CLD_LoopBuf();
public:
	virtual bool InitIdleBuf(int nSize=0,bool boForceInit=false);

	__declspec(property(get=GetDataBufPointer,put=SetDataBufPointer)) char* pData;  
	__declspec(property(get=GetDataBufLen)) int nDataLen;
	__declspec(property(get=GetIdleBufPointer,put=SetIdleBufPointer)) char* pIdle;  
	__declspec(property(get=GetIdleBufLen)) int nIdleLen;
	__declspec(property(get=GetMaxSize)) int nMaxSize;

	char*	GetDataBufPointer();
	void	SetDataBufPointer(char* Value);
	int		GetDataBufLen();

	char*	GetIdleBufPointer();
	void	SetIdleBufPointer(char* Value);
	int		GetIdleBufLen();

	int		GetMaxSize();

	void	Clear();
	bool	ReSize(int nSize);

	bool	SetCanRead(bool boCanRead){
		bool boRet=m_boCanRead;
		m_boCanRead=boCanRead;
		return boRet;
	}
	bool	SetCanWrite(bool boCanWrite){
		bool boRet=m_boCanWrite;
		m_boCanWrite=boCanWrite;
		return boRet;
	}

	char*	GetBuffer(){ return m_Buf; }

	size_t m_initBufSize;
protected:
	bool	move_data_pos(char* pData);
	bool	move_Idle_pos(char* pData);
protected:
	char* m_Buf;     
	int m_nBufSize;  
	char* m_pData;   
	int m_nLen;     
	char* m_pIdle;    
	int m_nIdleLen;  
	bool m_boCanRead;
	bool m_boCanWrite;
protected:
	CIntLock m_locker;
	CLD_LoopBuf(const CLD_LoopBuf&);

	const CLD_LoopBuf & operator= (const CLD_LoopBuf &);
};
//------------------------------------------------------------------------
template <int STATICBUFSIZE>
class CLD_StaticLoopBuf:public CLD_LoopBuf
{
public:
	CLD_StaticLoopBuf():CLD_LoopBuf(0){
		int nBufSize=ROUNDNUM2(STATICBUFSIZE+1,8);
		nBufSize--;

		m_Buf=&m_buffer;

		m_Buf[0]=0;
		m_Buf[nBufSize-1]=0;
		m_Buf[nBufSize]=0;

		m_nBufSize=nBufSize;

		m_nIdleLen=nBufSize;
		m_pIdle=m_Buf;

		m_pData=m_Buf;
	}

	virtual ~CLD_StaticLoopBuf(){
		m_Buf=NULL;
		m_nBufSize=0;
		m_nLen=0;
		m_nIdleLen=0;
		m_pData=NULL;
		m_pIdle=NULL;
	}

	virtual bool InitIdleBuf(int nSize=0,bool boForceInit=false){
		char* pRetBuf=NULL;
		__try
		{
			if(nSize<=m_nIdleLen){
				pRetBuf= m_pIdle;
			}else if (m_boCanWrite && m_boCanRead || boForceInit){
				if(nSize<=(m_nBufSize-m_nLen)){
					MoveMemory(m_Buf,m_pData,m_nLen+1);

					m_pIdle=m_Buf+m_nLen;
					m_nIdleLen=m_nBufSize-m_nLen;

					m_pData=m_Buf;

					pRetBuf= m_pIdle;
				}
			}
		}__except(EXCEPTION_EXECUTE_HANDLER){
			pRetBuf=NULL;
		}
		if(pRetBuf){
			pRetBuf[0]=0;
			pRetBuf[nSize]=0;
			return true;
		}
		return false;
	}
protected:
	char m_buffer[ROUNDNUM2(STATICBUFSIZE+1,8)]; 
};
//------------------------------------------------------------------------