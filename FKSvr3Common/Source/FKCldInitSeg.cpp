/**
*	created:		2013-4-8   15:03
*	filename: 		FKCldInitSeg
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <wchar.h>
#include "Dump/FKDumpErrorBase.h"
#include "Dump/FKDumpError.h"
#include "STLTemplate/FKSyncList.h"
#include "FKThread.h"
#include "FKOutput.h"
#include "FKLogger.h"
#include "FKSyncObjLock.h"
#include "STLTemplate/FKFrameAllocator.h"
#include "Network/FKAsynSocket.h"
#include "STLTemplate/FKLookasideAlloc.h"
#include "FKTimeMonitor.h"
#include "Network/FKPacket.h"
//------------------------------------------------------------------------
#pragma warning(disable:4355) 
#pragma warning(disable:4073)
#pragma init_seg(lib)
//------------------------------------------------------------------------
LPTOP_LEVEL_EXCEPTION_FILTER SystemPre=NULL;
HINSTANCE g_hinstance=0;
HANDLE	g_mainwindowhandle=0;
bool stProcessShareData::s_ishost=false;
stProcessShareData* stProcessShareData::g_shareData=NULL;
//------------------------------------------------------------------------
_TH_VAR_INIT(char*,sStrDumpStackCallback,(char*)NULL);
pGetModuleFileNameFunc g_geterrormodulefilename=NULL;
pOnExceptionBeginCallBack g_onexceptionbegincallback=NULL;
pOnExceptionEndCallBack g_onexceptionendcallback=NULL;
//------------------------------------------------------------------------
char * g_dumpbasepath=".\\error_log\\";
//------------------------------------------------------------------------
unsigned long stStackFrameAllocator::m_initframeSize=0;
stStackFrameAllocator init_tls_FrameAllocator={0,0,NULL,{0}};
_TH_VAR_INIT(stStackFrameAllocator,tls_FrameAllocator,init_tls_FrameAllocator);

stThreadObjInfo init_tls_CurrThreadObj={NULL,0,0};
_TH_VAR_INIT(stThreadObjInfo,tls_CurrThreadObj,init_tls_CurrThreadObj);

_TH_VAR_INIT_ARRAY(char,tls_loop_charbuffer,MAX_TLS_LOOPCHARBUFFER+1,'\0');
//------------------------------------------------------------------------
std::allocator< char > CSimpleAllocator::_ty_alloc_0_128;
safe_lookaside_allocator< char[256],64 > CSimpleAllocator::_ty_alloc_128; 
safe_lookaside_allocator< char[512],48 > CSimpleAllocator::_ty_alloc_256;
safe_lookaside_allocator< char[512*2],32 > CSimpleAllocator::_ty_alloc_512;
safe_lookaside_allocator< char[512*3],16 > CSimpleAllocator::_ty_alloc_512x2;
safe_lookaside_allocator< char[512*4],16 > CSimpleAllocator::_ty_alloc_512x3;
safe_lookaside_allocator< char[512*5],16 > CSimpleAllocator::_ty_alloc_512x4; 
CSimpleAllocator CSimpleAllocator::_ty_alloc;
//------------------------------------------------------------------------
COutput	g_appout(true);
//------------------------------------------------------------------------
zLogger::zLevel zLogger::zOFF("OFF",zLogger::eALL+1,zLogger::eALL+1,false,0);
zLogger::zLevel zLogger::zFORCE("FORCE",zLogger::eFORCE,zLogger::eFORCE,false,0x00ff0000);

zLogger::zLevel zLogger::zFATAL("FATAL",zLogger::eFATAL,zLogger::eFATAL,true,0x000000ff);

zLogger::zLevel zLogger::zERROR("ERROR",zLogger::eERROR,zLogger::eERROR,false,0x000000ff);
zLogger::zLevel zLogger::zALARM("ALARM",zLogger::eALARM,zLogger::eALARM,false,0x000000ff);

zLogger::zLevel zLogger::zWARN("WARN",zLogger::eWARN,zLogger::eWARN,false,0x000000ff);
zLogger::zLevel zLogger::zIFFY("IFFY",zLogger::eIFFY,zLogger::eIFFY,false,0);
zLogger::zLevel zLogger::zINFO("INFO",zLogger::eINFO,zLogger::eINFO,false,0);
zLogger::zLevel zLogger::zTRACE("TRACE",zLogger::eTRACE,zLogger::eTRACE,false,0);
zLogger::zLevel zLogger::zDEBUG("DEBUG",zLogger::eDEBUG,zLogger::eDEBUG,false,0);
zLogger::zLevel zLogger::zGBUG("GBUG",zLogger::eGBUG,zLogger::eGBUG,false,0);
//------------------------------------------------------------------------
CSyncVector<zLogger*> zLogger ::m_loggers;
//------------------------------------------------------------------------
zLogger g_logger(std::string(""));
zLogger g_luaLogger(std::string(""));
//------------------------------------------------------------------------
volatile unsigned long int CNet::refcount = 0;
CIntLock CNet::netreflock;
//------------------------------------------------------------------------
//sockettask.cpp
_TH_VAR_INIT_ARRAY(char,tls_sendpacket_charbuffer,_MAX_SEND_PACKET_SIZE_+1,'\0');
_TH_VAR_INIT_ARRAY(char,tls_packetbuf_charbuffer,_MAX_SEND_PACKET_SIZE_+1,'\0');
//------------------------------------------------------------------------
//gatewaySvrSession.cpp  gatewayclientSession.cpp
_TH_VAR_INIT_ARRAY(char,tls_gatewayproxydata_charbuffer,_MAX_SEND_PACKET_SIZE_+1,'\0');
//------------------------------------------------------------------------
_TH_VAR_INIT_ARRAY(char,tls_msgbuilder_charbuff,_MAX_SEND_PACKET_SIZE_+1,'\0');
//------------------------------------------------------------------------