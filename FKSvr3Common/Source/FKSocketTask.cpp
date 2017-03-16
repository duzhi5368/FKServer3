/**
*	created:		2013-4-7   23:38
*	filename: 		FKSocketTask
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/Network/FKSocketTask.h"
#include "../Include/FKTimeMonitor.h"
//----------------------------------------------------------------------------
#define _FLASH_AUTHORITY_POSTSTR_		"<policy-file-request/>"
//----------------------------------------------------------------------------
unsigned char connect_def_key_16_byte[16] = { 19, 98, 167, 15, 243, 192, 199, 115, 140, 152, 147, 143, 217, 188, 76, 130 };
//----------------------------------------------------------------------------
bool g_boshowproxylog=false;
//----------------------------------------------------------------------------
stBasePacket* __stdcall getpackethdr(const char* pbuf, unsigned int nbuflen, CEncrypt* enc, unsigned int& _de_size,bool& isfullpacket,bool& iserror)
{
	FUNCTION_BEGIN;
	isfullpacket=false;
	iserror=false;
	stBasePacket* pretpackethdr=(stBasePacket*)pbuf;
	if(_de_size==0 && nbuflen >= 8){
		if (enc && enc->getEncMethod() != CEncrypt::ENCDEC_NONE) {
			enc->encdec((void *) &pbuf[_de_size], 8, false);
		}
		_de_size+=8;
	}
	if (_de_size>=8){
		if (pretpackethdr->isfullpacket(nbuflen)){
			isfullpacket=true;
		}
		return pretpackethdr;
	}else{
		return NULL;
	}
}
//----------------------------------------------------------------------------
int __stdcall getmsg2buf(const char*& pbuf, unsigned int& nbuflen, CEncrypt* enc, unsigned int& _de_size,stBaseCmd* pmsg,int nmsgmaxlen)
{
	FUNCTION_BEGIN;
	if (nbuflen==0 || pbuf==NULL){return 0;}
	unsigned long nlen = nbuflen;
	const char* pbuffer = pbuf;

	if (_de_size > nlen || nmsgmaxlen<sizeof(stBaseCmd)) {return -1;}	
	unsigned int size = nlen - _de_size;
	stBasePacket* ppacket = (stBasePacket*) pbuffer;
	if (_de_size<8 || !ppacket->isfullpacket(_de_size)){
		if (size < 8) {	return 0; } 
		else if(_de_size==0){
			if (enc && enc->getEncMethod() != CEncrypt::ENCDEC_NONE) {
				enc->encdec((void *) &pbuffer[_de_size], 8, false);
			}
			_de_size+=8;
			size-=8;
		}
		size=safe_min(size,ROUNDNUM2(ppacket->getpacketsize(), 8)-_de_size);
		if (size>=8){
			if (size > 8) { size =ROUNDNUM2(size-7,8);  ; }	
			if (enc && enc->getEncMethod() != CEncrypt::ENCDEC_NONE) {
				enc->encdec((void *) &pbuffer[_de_size], size, false);
			}
			_de_size += size;
		}
		if (ppacket->size > (DWORD)nmsgmaxlen) {
			return -2;
		}
		if (!ppacket->isfullpacket(_de_size)) {
			return 0;
		}
	}

	nlen = ROUNDNUM2(ppacket->getpacketsize(), 8);
	nbuflen -= nlen;
	pbuf += nlen;
	_de_size -= nlen;

	unsigned long ncmdlen = nmsgmaxlen;
	if (ppacket->iscompress()) 
	{
		if (uncompresszlib((unsigned char *) ppacket->cmd(), ppacket->size, (unsigned char *) pmsg, ncmdlen) != Z_OK 
			|| ncmdlen > (DWORD)nmsgmaxlen) 
		{
			return -3;
		}
	} else {

		ncmdlen = ppacket->size;
		CopyMemory(pmsg, ppacket->cmd(), ppacket->size);
	}
	return ncmdlen;
}
//----------------------------------------------------------------------------
int __stdcall packetbuf(unsigned char* pin, unsigned long ninlen, unsigned char* pout, 
						unsigned long nmaxlen, CEncrypt* enc, int zliblevel,bool issplit)
{
	FUNCTION_BEGIN;


	if (ninlen==0 || pin==NULL || pout==NULL){return 0;}
	if (ninlen > nmaxlen) {return -1;}
	if (nmaxlen < (ninlen+_MIN_PACKETBUF_SIZE_)) {return -2;}

	stBasePacket* ppacket = (stBasePacket*) pout;
	ppacket->init( ( ( ( zliblevel>=Z_BEST_SPEED ) || ( zliblevel==Z_DEFAULT_COMPRESSION ) ) && (ninlen > MINZLIBPACKSIZE) ),issplit );
	if ( ppacket->iscompress() ) {
		unsigned long nzlibbuflen=nmaxlen;
		if (compresszlib(pin, ninlen, (unsigned char *)ppacket->cmd(), nzlibbuflen, safe_min(((int)zliblevel), ((int)Z_BEST_COMPRESSION))) != Z_OK
			|| (nzlibbuflen>nmaxlen) ) {
				return -3;
		}
		ppacket->size = (WORD)nzlibbuflen;
		ppacket->setcmdsize(ninlen);
	}else{
		CopyMemory(ppacket->cmd(), pin, ninlen);
		ppacket->size = (WORD) ninlen;
	}

	unsigned int nretsize = ROUNDNUM2(ppacket->getpacketsize(), 8);
	if (nmaxlen < nretsize) {return -4;}
	if (enc && enc->getEncMethod() != CEncrypt::ENCDEC_NONE) {
		enc->encdec(ppacket, nretsize, true);
	}
	return nretsize;
}
//----------------------------------------------------------------------------
int packet_getcmd(CLD_LoopBuf* pbuf, CEncrypt* enc, unsigned int& _de_size,stBaseCmd* pmsg,int nmsgmaxlen)
{
	unsigned int nlen = pbuf->nDataLen;
	if (nlen==0){return 0;}
	char* pbuffer = ((char*)pbuf->pData);
	int ncmdlen = getmsg2buf((const char*&)pbuffer, nlen, enc, _de_size,pmsg,nmsgmaxlen);
	if (ncmdlen>0){
		pbuf->pData = pbuffer;
	}else if (ncmdlen<0){
		if (_de_size>=8){
			stBasePacket* ptmppacket=(stBasePacket*)pbuffer;
			g_logger.error("获取命令失败 %d (_de_size=%d,nlen=%d(%d) -> %d,%d,%d)",ncmdlen,_de_size,
				pbuf->nDataLen,pbuf->nIdleLen,
				ptmppacket->size,ptmppacket->cmd()->cmd,ptmppacket->cmd()->subcmd );
		}else{
			g_logger.error("获取命令失败 %d ( _de_size=%d,%d(%d) )",ncmdlen,_de_size,pbuf->nDataLen,pbuf->nIdleLen );
		}
	}
	return ncmdlen;
}
//----------------------------------------------------------------------------
extern _TH_VAR_ARRAY(char,tls_packetbuf_charbuffer,_MAX_SEND_PACKET_SIZE_+1);
//----------------------------------------------------------------------------
bool packet_addcmd(CLD_IocpObj& po, CEncrypt* enc, void* pbuf, unsigned int nsize, int zliblevel)
{
	char* szzlibbuf=(char*)_TH_VAR_PTR(tls_packetbuf_charbuffer);
	unsigned int nmaxsize=_TH_VAR_SIZEOF(tls_packetbuf_charbuffer);
	if (nsize>nmaxsize){
		stBaseCmd* pmsg=(stBaseCmd*)pbuf;
		g_logger.forceLog( zLogger::zFATAL, "发送数据失败( %d > %d )( %d,%d )",nsize,nmaxsize,pmsg->cmd,pmsg->subcmd );
		return false;
	}else{
		int nlen = packetbuf((unsigned char*) pbuf, nsize, (unsigned char *)szzlibbuf, nmaxsize, enc, zliblevel,false);
		if ((nlen>0) && (po.AddSendBuf((const char *) szzlibbuf, nlen) == nlen))
		{		

			return true;
		} else {
			stBaseCmd* pmsg=(stBaseCmd*)pbuf;
			g_logger.forceLog( zLogger::zFATAL, "发送数据失败 %d (size=%d -> %d,%d)",nlen,nsize,pmsg->cmd,pmsg->subcmd );
			return false;
		}
	}
}
//----------------------------------------------------------------------------
#define  _DEF_VALID_TIMEOUT_				5
#define  _DEF_CHECK_SIGNAL_INTERVAL_		60*4+rand()%(60*4)
//----------------------------------------------------------------------------
#ifdef _USE_API_LOADLIB_
#define  _DEF_CHECK_SIGNAL_WAITTIME_		60*120
#else
#define  _DEF_CHECK_SIGNAL_WAITTIME_		20
#endif
//----------------------------------------------------------------------------
void TcpTaskTerminatePrintDumpStackCallback(void* param,const char* szFormat,...){
	char sStrCallStack[1024*8];
	va_list args;
	va_start( args, szFormat );
	vsprintf(sStrCallStack,szFormat,args);
	va_end(args);
	g_logger.debug(sStrCallStack);
}
//----------------------------------------------------------------------------
void SvrDisconnectPrintDumpStackCallback(void* param,const char* szFormat,...){
	char sStrCallStack[1024*8];
	va_list args;
	va_start( args, szFormat );
	vsprintf(sStrCallStack,szFormat,args);
	va_end(args);
	((COutput*)param)->WriteString(sStrCallStack,true);
}
//----------------------------------------------------------------------------
CLoopbufIocpClientSocketTask::CLoopbufIocpClientSocketTask(CLD_IocpBaseAccepter* Owner, SOCKET s,int nLoopbufSize)
: CLD_LoopbufIocpClientSocket(Owner, s,nLoopbufSize)
{
	FUNCTION_BEGIN;
	m_IocpObj.SetRecycleThreadCallBack(RecycleThreadCallBackProxy, this);
	m_pUpperEncodeSetter = NULL;	
	terminate = terminate_no;
	m_conntime = time(NULL);
	m_checkconntime=m_conntime+check_signal_interval();
	m_checksignalstate=checksignalstate_no;
	_de_size = 0;
	_rcv_size_last = 0;
	m_dwMaxRecvPacketLen=_MAX_PACKETBUF_SIZE_;
}
//----------------------------------------------------------------------------
void CLoopbufIocpClientSocketTask::Terminate(const char* ffline,const TerminateMethod method){
	if (terminate==terminate_no && method!=terminate_client_active && IsConnected()){
		g_logger.debug("连接 %s:%d 主动调用 terminate() ...", GetRemoteAddress(), GetRemotePort());
		PrintThreadCallStack(TcpTaskTerminatePrintDumpStackCallback,3);
	}
	terminate = method;
}
//----------------------------------------------------------------------------
void CLoopbufIocpClientSocketTask::DoDisconnect(){
	CLD_LoopbufIocpClientSocket::DoDisconnect();
	stQueueMsg* pbufmsg=NULL;
	while(m_msg.Pop(pbufmsg))
		FreePacketBuffer(pbufmsg);
	m_msg.clear();
}
//----------------------------------------------------------------------------
void CLoopbufIocpClientSocketTask::run()
{
	stQueueMsgParam bufferparam(NULL,true);
	stQueueMsg*& pbufmsg=bufferparam.pQueueMsgBuffer;
	while(m_msg.Pop(pbufmsg))
	{
		bufferparam.bofreebuffer=true;
		msgParse(&pbufmsg->cmdBuffer,pbufmsg->cmdsize,&bufferparam);
		if (bufferparam.bofreebuffer)
		{
			FreePacketBuffer(pbufmsg);
		}
		if (isTerminate()){break;}
	}
	postcmds();
}
//----------------------------------------------------------------------------
void CLoopbufIocpClientSocketTask::pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	m_msg.Push(bufferparam->pQueueMsgBuffer);
	return;
}
//----------------------------------------------------------------------------
bool CLoopbufIocpClientSocketTask::packetCheck(stBasePacket* ppacket,bool isfullpacket){
	return true;
}
//----------------------------------------------------------------------------
void CLoopbufIocpClientSocketTask::DoRead()
{
	FUNCTION_BEGIN;
	FUNCTION_WRAPPER(true,NULL);
	if (IsConnected()) {

		bool hasfullpacket=false;
		bool haserror=false;
		int ncmdlen = 0;
		stQueueMsgParam bufferparam(NULL,true);
		stQueueMsg*& pbufmsg=bufferparam.pQueueMsgBuffer;
		CLD_LoopBuf* pbuf = GetRecvBufer();
		if ((unsigned int) pbuf->nDataLen > _rcv_size_last && !isTerminate()) 
		{
			ncmdlen=0;
			stBasePacket* _packet_hdr_=getpackethdr(pbuf->pData,pbuf->nDataLen, m_pUpperEncodeSetter, _de_size,hasfullpacket,haserror);
			if (haserror){ 
				Terminate(__FF_LINE__);	
			}else if ( hasfullpacket){
				packetCheck(_packet_hdr_,hasfullpacket);
				if (!isTerminate()){	
					int tmpMsgBufferSize=ROUNDNUM2(_packet_hdr_->getcmdsize()+64,64);
					if (NewPacketBuffer(pbufmsg,tmpMsgBufferSize+sizeof(*pbufmsg))){
						ncmdlen = packet_getcmd(pbuf, m_pUpperEncodeSetter, _de_size,&pbufmsg->cmdBuffer,tmpMsgBufferSize);
						if (ncmdlen <=0 ){ FreePacketBuffer(pbufmsg); }
					}else{
						ncmdlen=-1;
					}
					if (ncmdlen < 0) {
						Terminate(__FF_LINE__);
					} else {
						if (m_checksignalstate==checksignalstate_no){m_checkconntime=time(NULL)+check_signal_interval();}
						else{m_checkconntime=time(NULL)+check_signal_waittime();}

						while (ncmdlen>0){

							switch(pbufmsg->cmdBuffer.value)
							{
							case stCheckSignalCmd::_value:
								{

									if (m_checksignalstate==checksignalstate_no){

										stCheckSignalCmd cmd((stCheckSignalCmd*)&pbufmsg->cmdBuffer);
										sendcmd(&cmd,sizeof(cmd));
									}
									m_checkconntime=time(NULL)+check_signal_interval();
									m_checksignalstate=checksignalstate_no;
									FreePacketBuffer(pbufmsg);
								}
								break;
							default:
								{
									pbufmsg->cmdsize=ncmdlen;
									pbufmsg->pluscmdoffset=0;
									bufferparam.bofreebuffer=false;
									pushMsgQueue(_packet_hdr_,&pbufmsg->cmdBuffer,pbufmsg->cmdsize,&bufferparam);
								}
								break;
							}
							if ( isTerminate() || !IsConnected() ){break;}
							ncmdlen=0;
							_packet_hdr_=getpackethdr(pbuf->pData,pbuf->nDataLen, m_pUpperEncodeSetter, _de_size,hasfullpacket,haserror);
							if (haserror){ 
								Terminate(__FF_LINE__);	break;
							}else if (hasfullpacket){
								packetCheck(_packet_hdr_,hasfullpacket);
								if (isTerminate()){	break; };
								tmpMsgBufferSize=ROUNDNUM2(_packet_hdr_->getcmdsize()+64,64);
								if (NewPacketBuffer(pbufmsg,tmpMsgBufferSize+sizeof(*pbufmsg))){
									ncmdlen = packet_getcmd(pbuf, m_pUpperEncodeSetter, _de_size,&pbufmsg->cmdBuffer,tmpMsgBufferSize);
									if (ncmdlen <=0 ){ FreePacketBuffer(pbufmsg); }
								}else{
									ncmdlen=-1;
								}
								if (ncmdlen < 0) { Terminate(__FF_LINE__);	break; }
							}
						}
					}
				}
			}
			_rcv_size_last = pbuf->nDataLen;
		} else if (_rcv_size_last > (unsigned int) pbuf->nDataLen) {
			Terminate(__FF_LINE__);
		}
	}
	CLD_LoopbufIocpClientSocket::DoRead();
}
//----------------------------------------------------------------------------
void CLoopbufIocpClientSocketTask::RecycleThreadCallBack()
{
	FUNCTION_BEGIN;
	FUNCTION_WRAPPER(true,NULL);

	if (!isvalid()) {
		if ((time(NULL) - m_conntime) > valid_timeout()) {

			g_logger.debug("%s:%d 连接验证超时(%d 秒)...", GetRemoteAddress(), GetRemotePort(), valid_timeout());
			Terminate(__FF_LINE__);
		}
	} else if (!IsConnected()) {
		Terminate(__FF_LINE__);
	} 
	if (isTerminate()) {
		__super::Close();
		return;
	}else if (time(NULL)>m_checkconntime){
		if (m_checksignalstate==checksignalstate_no){
			stCheckSignalCmd cmd(true);
			sendcmd(&cmd,sizeof(cmd));
			m_checkconntime=time(NULL)+check_signal_waittime();
			m_checksignalstate=checksignalstate_waitrecv;

		}else{
			g_logger.debug("%s:%d 测试信号返回超时...", GetRemoteAddress(), GetRemotePort());
			Terminate(__FF_LINE__);	
			__super::Close();
		}
	}
}
//----------------------------------------------------------------------------
bool CLoopbufIocpClientSocketTask::sendcmd(void* pbuf, unsigned int nsize, int zliblevel )
{
	FUNCTION_BEGIN;
	if (!isTerminate()) 
	{
		if (packet_addcmd(m_IocpObj, m_pUpperEncodeSetter, pbuf, nsize,zliblevel))
		{
			return m_IocpObj.PostSendBuf();
		}
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------
bool CLoopbufIocpClientSocketTask::addcmd(void* pbuf, unsigned int nsize, int zliblevel ){
	FUNCTION_BEGIN;
	if (!isTerminate()) {
		m_boaddcmds=packet_addcmd(m_IocpObj, m_pUpperEncodeSetter, pbuf, nsize,zliblevel);
		return m_boaddcmds;
	}
	return true;
}
//----------------------------------------------------------------------------
bool CLoopbufIocpClientSocketTask::postcmds()
{
	if ( m_boaddcmds ){
		m_boaddcmds=false;
		return m_IocpObj.PostSendBuf();
	}

	return true;	
}
//----------------------------------------------------------------------------
time_t CLoopbufIocpClientSocketTask::valid_timeout(){
	return _DEF_VALID_TIMEOUT_;
};
time_t CLoopbufIocpClientSocketTask::check_signal_interval(){
	return _DEF_CHECK_SIGNAL_INTERVAL_;
}
time_t CLoopbufIocpClientSocketTask::check_signal_waittime(){
	return _DEF_CHECK_SIGNAL_WAITTIME_;
}
//----------------------------------------------------------------------------
CLoopbufIocpConnecterTask::CLoopbufIocpConnecterTask(CLD_IocpHandle* Owner,int nLoopbufSize)
: CLD_LoopbufIocpConnecter(Owner,nLoopbufSize)
{
	FUNCTION_BEGIN;
	m_IocpObj.SetRecycleThreadCallBack(RecycleThreadCallBackProxy, this);
	m_penc = NULL;
	terminate = terminate_no;
	m_conntime = time(NULL);
	m_checkconntime=m_conntime+check_signal_interval();
	m_checksignalstate=checksignalstate_no;
	_de_size = 0;
	_rcv_size_last = 0;
	m_dwMaxRecvPacketLen=_MAX_PACKETBUF_SIZE_;
}
//----------------------------------------------------------------------------
void CLoopbufIocpConnecterTask::RecycleThreadCallBack()
{
	FUNCTION_BEGIN;
	FUNCTION_WRAPPER(true,NULL);	
	if (!isvalid()) {
		if ((time(NULL) - m_conntime) > valid_timeout()) {

			g_logger.debug("%s:%d 连接验证超时(%d 秒)...", GetRemoteAddress(), GetRemotePort(), valid_timeout());
			Terminate(__FF_LINE__);
		}
	} else if (!IsConnected()) {
		Terminate(__FF_LINE__);
	}
	if (isTerminate()) {
		__super::Close();
		return;
	} else if (time(NULL)>m_checkconntime){
		if (m_checksignalstate==checksignalstate_no){
			stCheckSignalCmd cmd(true);
			sendcmd(&cmd,sizeof(cmd));
			m_checkconntime=time(NULL)+check_signal_waittime();
			m_checksignalstate=checksignalstate_waitrecv;

		}else{
			g_logger.debug("%s:%d 测试信号返回超时...", GetRemoteAddress(), GetRemotePort());
			Terminate(__FF_LINE__);	
			__super::Close();
		}
	}
}
//----------------------------------------------------------------------------
void CLoopbufIocpConnecterTask::OnIocpConnect()
{
	FUNCTION_BEGIN;
	m_conntime = time(NULL);
	m_checkconntime=m_conntime+check_signal_interval();
	m_checksignalstate=checksignalstate_no;
	g_logger.debug("服务器 %s:%d 连接成功...", GetRemoteAddress(), GetRemotePort());
}
//----------------------------------------------------------------------------
void CLoopbufIocpConnecterTask::OnDisconnect()
{
	FUNCTION_BEGIN;
	__super::OnDisconnect();
	terminate = terminate_no;
	_de_size = 0;
	_rcv_size_last = 0;
	g_logger.debug("%s:%d 连接断开...", GetRemoteAddress(), GetRemotePort());
	stQueueMsg* pbufmsg=NULL;
	while(m_msg.Pop(pbufmsg))
		FreePacketBuffer(pbufmsg);
	m_msg.clear();
}
//----------------------------------------------------------------------------
void CLoopbufIocpConnecterTask::OnError(CLD_TErrorEvent ErrEvent, OUT int& nErrCode, char* sErrMsg)
{
	FUNCTION_BEGIN;
	g_logger.debug("%s:%d 连接异常(%d->%s)...", GetRemoteAddress(), GetRemotePort(), nErrCode, sErrMsg);
	if (nErrCode != 0) {
		this->Terminate(__FF_LINE__);nErrCode = 0;
	}
}
//----------------------------------------------------------------------------
void CLoopbufIocpConnecterTask::Terminate(const char* ffline,const TerminateMethod method )
{
	if (terminate==terminate_no && method!=terminate_client_active && IsConnected())
	{
		g_logger.debug("连接 %s:%d 主动调用 terminate() ...", GetRemoteAddress(), GetRemotePort());
		PrintThreadCallStack(TcpTaskTerminatePrintDumpStackCallback);
	}
	terminate = method;
}
//----------------------------------------------------------------------------
void CLoopbufIocpConnecterTask::run()
{
	stQueueMsgParam bufferparam(NULL,true);
	stQueueMsg*& pbufmsg=bufferparam.pQueueMsgBuffer;
	while(m_msg.Pop(pbufmsg))
	{
		bufferparam.bofreebuffer=true;
		msgParse(&pbufmsg->cmdBuffer,pbufmsg->cmdsize,&bufferparam);
		if (bufferparam.bofreebuffer)
		{
			FreePacketBuffer(pbufmsg);
		}
		if (isTerminate()){break;}
	}
	postcmds();
}
//----------------------------------------------------------------------------
void CLoopbufIocpConnecterTask::pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam)
{
	m_msg.Push(bufferparam->pQueueMsgBuffer);
	return;
}
//----------------------------------------------------------------------------
bool CLoopbufIocpConnecterTask::packetCheck(stBasePacket* ppacket,bool isfullpacket){
	return true;
}
//----------------------------------------------------------------------------
void CLoopbufIocpConnecterTask::OnRead()
{
	FUNCTION_BEGIN;
	FUNCTION_WRAPPER(true,NULL);
	if (IsConnected()) 
	{
		bool hasfullpacket=false;
		bool haserror=false;
		int ncmdlen = 0;
		stQueueMsgParam bufferparam(NULL,true);
		stQueueMsg*& pbufmsg=bufferparam.pQueueMsgBuffer;
		CLD_LoopBuf* pbuf = GetRecvBufer();

		if ((unsigned int) pbuf->nDataLen > _rcv_size_last && !isTerminate()) 
		{
			ncmdlen=0;
			stBasePacket* _packet_hdr_=getpackethdr(pbuf->pData,pbuf->nDataLen, m_penc, _de_size,hasfullpacket,haserror);
			if (haserror)
			{ 
				Terminate(__FF_LINE__);	
			}
			else if (hasfullpacket)
			{
				packetCheck(_packet_hdr_,hasfullpacket);
				if (!isTerminate())
				{
					int tmpMsgBufferSize=ROUNDNUM2(_packet_hdr_->getcmdsize()+64,64);
					if (NewPacketBuffer(pbufmsg,tmpMsgBufferSize+sizeof(*pbufmsg)))
					{
						ncmdlen = packet_getcmd(pbuf, m_penc, _de_size,&pbufmsg->cmdBuffer,tmpMsgBufferSize);
						if (ncmdlen <=0 ){ FreePacketBuffer(pbufmsg); }
					}
					else
					{
						ncmdlen=-1;
					}
					if (ncmdlen < 0) 
					{
						Terminate(__FF_LINE__);
					} 
					else 
					{
						if (m_checksignalstate==checksignalstate_no){m_checkconntime=time(NULL)+check_signal_interval();}
						else{m_checkconntime=time(NULL)+check_signal_waittime();}

						while (ncmdlen>0) 
						{

							switch(pbufmsg->cmdBuffer.value)
							{
							case stCheckSignalCmd::_value:
								{
									if (m_checksignalstate==checksignalstate_no)
									{
										stCheckSignalCmd cmd((stCheckSignalCmd*)&pbufmsg->cmdBuffer);
										sendcmd(&cmd,sizeof(cmd));
									}
									m_checkconntime=time(NULL)+check_signal_interval();
									m_checksignalstate=checksignalstate_no;
									FreePacketBuffer(pbufmsg);
								}
								break;
							default:
								{
									pbufmsg->cmdsize=ncmdlen;
									pbufmsg->pluscmdoffset=0;
									bufferparam.bofreebuffer=false;
									pushMsgQueue(_packet_hdr_,&pbufmsg->cmdBuffer,pbufmsg->cmdsize,&bufferparam);
								}
								break;
							}
							if ( isTerminate() || !IsConnected() ){break;}
							ncmdlen=0;
							_packet_hdr_=getpackethdr(pbuf->pData,pbuf->nDataLen, m_penc, _de_size,hasfullpacket,haserror);
							if (haserror)
							{ 
								Terminate(__FF_LINE__);	break;
							}
							else if (hasfullpacket)
							{
								packetCheck(_packet_hdr_,hasfullpacket);
								if (isTerminate()){	break; };
								tmpMsgBufferSize=ROUNDNUM2(_packet_hdr_->getcmdsize()+64,64);
								if (NewPacketBuffer(pbufmsg,tmpMsgBufferSize+sizeof(*pbufmsg)))
								{
									ncmdlen = packet_getcmd(pbuf, m_penc, _de_size,&pbufmsg->cmdBuffer,tmpMsgBufferSize);
									if (ncmdlen <=0 ){ FreePacketBuffer(pbufmsg); }
								}
								else
								{
									ncmdlen=-1;
								}
								if (ncmdlen < 0) 
								{ 
									Terminate(__FF_LINE__);	break; 
								}
							}
						}
					}
				}
			}
			_rcv_size_last = pbuf->nDataLen;
		} 
		else if (_rcv_size_last > (unsigned int) pbuf->nDataLen) 
		{
			Terminate(__FF_LINE__);
		}
	}
	CLD_LoopbufIocpConnecter::OnRead();
}
//----------------------------------------------------------------------------
bool CLoopbufIocpConnecterTask::sendcmd(void* pbuf, unsigned int nsize, int zliblevel )
{
	FUNCTION_BEGIN;
	if (!isTerminate()) 
	{
		if (packet_addcmd(m_IocpObj, m_penc, pbuf, nsize,zliblevel))
		{
			return m_IocpObj.PostSendBuf();
		}
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------
bool CLoopbufIocpConnecterTask::addcmd(void* pbuf, unsigned int nsize, int zliblevel ){
	FUNCTION_BEGIN;
	if (!isTerminate()) 
	{
		m_boaddcmds=packet_addcmd(m_IocpObj, m_penc, pbuf, nsize,zliblevel);
		return m_boaddcmds;
	}
	return true;
}
//----------------------------------------------------------------------------
bool CLoopbufIocpConnecterTask::postcmds()
{
	if ( m_boaddcmds )
	{
		m_boaddcmds = false;
		return m_IocpObj.PostSendBuf();
	}
	return true;	
}
//----------------------------------------------------------------------------
time_t CLoopbufIocpConnecterTask::valid_timeout()
{
	return _DEF_VALID_TIMEOUT_;
}
//----------------------------------------------------------------------------
time_t CLoopbufIocpConnecterTask::check_signal_interval()
{
	return _DEF_CHECK_SIGNAL_INTERVAL_;
}
//----------------------------------------------------------------------------
time_t CLoopbufIocpConnecterTask::check_signal_waittime()
{
	return _DEF_CHECK_SIGNAL_WAITTIME_;
}
//----------------------------------------------------------------------------
CClientConnecter::CClientConnecter()
{
	m_boinitconn=false;
	m_autoreconn=true;
	m_penc=NULL;
	_de_size=0;
	_rcv_size_last=0;
	m_conntime = time(NULL);
	m_checkconntime=m_conntime+check_signal_interval();
	terminate=terminate_no;
	m_checksignalstate=checksignalstate_no;
	m_runthread=CThreadFactory::CreateBindClass(this,&CClientConnecter::thrun,m_lpParam);
	if ( m_runthread == NULL )
		return;
	m_runthread->Start(false);
	m_dwMaxRecvPacketLen=_MAX_PACKETBUF_SIZE_;
}
//----------------------------------------------------------------------------
CClientConnecter::~CClientConnecter()
{
	Terminate(__FF_LINE__);
	if (m_runthread)
	{
		m_runthread->Terminate();
		m_runthread->Waitfor();
		SAFE_DELETE(m_runthread);
	}
	stQueueMsg* pbufmsg=NULL;
	while(m_msg.Pop(pbufmsg))
		FreePacketBuffer(pbufmsg);
	m_msg.clear();
	return;
}
//----------------------------------------------------------------------------
time_t CClientConnecter::valid_timeout()
{
	return _DEF_VALID_TIMEOUT_;
};
time_t CClientConnecter::check_signal_interval()
{
	return _DEF_CHECK_SIGNAL_INTERVAL_;
}
time_t CClientConnecter::check_signal_waittime()
{
	return _DEF_CHECK_SIGNAL_WAITTIME_;
}
//----------------------------------------------------------------------------
void CClientConnecter::OnError(CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg){
	FUNCTION_BEGIN;
	g_logger.debug("%s:%d 连接异常(%d->%s)...", GetRemoteAddress(), GetRemotePort(), nErrCode, sErrMsg);
	if (nErrCode != 0) {
		this->Terminate(__FF_LINE__);nErrCode = 0;
	}
}
//----------------------------------------------------------------------------
void CClientConnecter::OnConnect(){
	_de_size = 0;
	_rcv_size_last = 0;
	m_conntime=time(NULL);

	m_conntime = time(NULL);
	m_checkconntime=m_conntime+check_signal_interval();
	m_checksignalstate=checksignalstate_no;
}
//----------------------------------------------------------------------------
void CClientConnecter::OnDisconnect(){
	_de_size = 0;
	_rcv_size_last = 0;
	stQueueMsg* pbufmsg=NULL;
	while(m_msg.Pop(pbufmsg))
		FreePacketBuffer(pbufmsg);
	m_msg.clear();
}
//----------------------------------------------------------------------------
bool CClientConnecter::sendcmd(void* pbuf, unsigned int nsize, int zliblevel,DWORD dwTimeWait){
	if (!isTerminate() && IsConnected())
	{
		char* szzlibbuf=(char*)_TH_VAR_PTR(tls_packetbuf_charbuffer);
		unsigned int nmaxsize=_TH_VAR_SIZEOF(tls_packetbuf_charbuffer);
		int nlen = packetbuf((unsigned char*) pbuf, nsize, (unsigned char *)szzlibbuf, nmaxsize, m_penc, zliblevel,false);
		if ((nlen>0) && (SendBuf((const char *) szzlibbuf, nlen ,dwTimeWait) == nlen)){
			return true;
		} else {
			g_logger.error("发送数据失败 %d (size=%d -> %d,%d)",nlen,nsize,*((BYTE*)pbuf),*(((BYTE*)pbuf)+1) );
			return false;
		}
	}
	return true;
}
//----------------------------------------------------------------------------
void CClientConnecter::Terminate(const char* ffline,const TerminateMethod method ){
	terminate = method;
}
//----------------------------------------------------------------------------
bool CClientConnecter::setserver(const char* ip,int port,bool autoreconn){
	strcpy_s(m_szIp,sizeof(m_szIp)-1,ip);
	m_nport=port;
	m_autoreconn=autoreconn;
	m_boinitconn=false;
	terminate = terminate_no;
	if (m_runthread){
		m_runthread->Resume();
		return true;
	}
	return false;
}
//----------------------------------------------------------------------------
void CClientConnecter::run()
{
	stQueueMsgParam bufferparam(NULL,true);
	stQueueMsg*& pbufmsg=bufferparam.pQueueMsgBuffer;
	while(m_msg.Pop(pbufmsg))
	{
		bufferparam.bofreebuffer=true;
		msgParse(&pbufmsg->cmdBuffer,pbufmsg->cmdsize,&bufferparam);
		if (bufferparam.bofreebuffer)
		{
			FreePacketBuffer(pbufmsg);
		}
		if (isTerminate()){break;}
	}
}
//----------------------------------------------------------------------------
void CClientConnecter::pushMsgQueue(stBasePacket* ppacket,stBaseCmd* pcmd, unsigned int ncmdlen,stQueueMsgParam* bufferparam){
	m_msg.Push(bufferparam->pQueueMsgBuffer);
	return;
}
//----------------------------------------------------------------------------
bool CClientConnecter::packetCheck(stBasePacket* ppacket,bool isfullpacket){
	return true;
}
//----------------------------------------------------------------------------
unsigned int __stdcall CClientConnecter::thrun(CLD_ThreadBase* pthread,void* param){
	bool hasfullpacket=false;
	bool haserror=false;
	int ncmdlen = 0;
	stQueueMsgParam bufferparam(NULL,true);
	stQueueMsg*& pbufmsg=bufferparam.pQueueMsgBuffer;

	GETCURREIP(pcurraddr);
	THREAD_REGDEBUGINFO(thdebuginfo,pcurraddr,"zsConnecterThread",60);

	while(!pthread->IsTerminated()){
		int nsleeptime=20;
		if (IsConnected()){
			if (!isvalid()){
				if ((time(NULL) - m_conntime) > valid_timeout()) {

					g_logger.debug("%s:%d 连接验证超时(%d 秒)...", GetRemoteAddress(), GetRemotePort(), valid_timeout());
					Terminate(__FF_LINE__);
				}
			}
			if (!isTerminate()){
				int nrecv=ReceiveLength();
				while(nrecv>0 && IsConnected()){
					if (m_recvbuf.InitIdleBuf(nrecv)){
						nrecv=ReceiveBuf(m_recvbuf.pIdle,m_recvbuf.nIdleLen);
						if (nrecv>0){ m_recvbuf.pIdle+=nrecv; }
						nrecv=ReceiveLength();
					}else{
						int nerrcode=ERROR_CLOSESOCKET;
						Error(eeReceive,nerrcode,(char *)vformat("recvbuf not has enough buf (%d,%d,%d)",nerrcode,m_recvbuf.nIdleLen,(DWORD)m_recvbuf.pIdle));
						break;
					}
				}
				CLD_LoopBuf* pbuf = &m_recvbuf;
				if ((unsigned int) pbuf->nDataLen > _rcv_size_last && !isTerminate()) {
					ncmdlen=0;
					stBasePacket* _packet_hdr_=getpackethdr(pbuf->pData,pbuf->nDataLen, m_penc, _de_size,hasfullpacket,haserror);
					if (haserror){ 
						Terminate(__FF_LINE__);	
					}else if (hasfullpacket){
						packetCheck(_packet_hdr_,hasfullpacket);
						if (!isTerminate()){	
							int tmpMsgBufferSize=ROUNDNUM2(_packet_hdr_->getcmdsize()+64,64);
							if (NewPacketBuffer(pbufmsg,tmpMsgBufferSize+sizeof(*pbufmsg))){
								ncmdlen = packet_getcmd(pbuf, m_penc, _de_size,&pbufmsg->cmdBuffer,tmpMsgBufferSize);
								if (ncmdlen <=0 ){ FreePacketBuffer(pbufmsg); }
							}else{
								ncmdlen=-1;
							}
							if (ncmdlen<0){
								Terminate(__FF_LINE__);
							}else{
								if (m_checksignalstate==checksignalstate_no){m_checkconntime=time(NULL)+check_signal_interval();}
								else{m_checkconntime=time(NULL)+check_signal_waittime();}

								while(ncmdlen>0){
									switch(pbufmsg->cmdBuffer.value)
									{
									case stCheckSignalCmd::_value:
										{

											if (m_checksignalstate==checksignalstate_no){
												stCheckSignalCmd cmd((stCheckSignalCmd*)&pbufmsg->cmdBuffer);
												sendcmd(&cmd,sizeof(cmd),Z_DEFAULT_COMPRESSION,(DWORD)-1);
											}
											m_checkconntime=time(NULL)+check_signal_interval();
											m_checksignalstate=checksignalstate_no;
											FreePacketBuffer(pbufmsg);
										}
										break;
									default:
										{
											pbufmsg->cmdsize=ncmdlen;
											pbufmsg->pluscmdoffset=0;
											bufferparam.bofreebuffer=false;
											pushMsgQueue(_packet_hdr_,&pbufmsg->cmdBuffer,pbufmsg->cmdsize,&bufferparam);
										}
										break;
									}
									if ( isTerminate() || !IsConnected() ){break;}
									ncmdlen=0;
									_packet_hdr_=getpackethdr(pbuf->pData,pbuf->nDataLen, m_penc, _de_size,hasfullpacket,haserror);
									if (haserror){ 
										Terminate(__FF_LINE__);	break;
									}else if ( hasfullpacket){
										packetCheck(_packet_hdr_,hasfullpacket);
										if (isTerminate()){	break; };
										tmpMsgBufferSize=ROUNDNUM2(_packet_hdr_->getcmdsize()+64,64);
										if (NewPacketBuffer(pbufmsg,tmpMsgBufferSize+sizeof(*pbufmsg))){
											ncmdlen = packet_getcmd(pbuf, m_penc, _de_size,&pbufmsg->cmdBuffer,tmpMsgBufferSize);
											if (ncmdlen <=0 ){ FreePacketBuffer(pbufmsg); }
										}else{
											ncmdlen=-1;
										}
										if (ncmdlen < 0) { Terminate(__FF_LINE__);	break; }
									}
								}
							}
						}
					}
					_rcv_size_last = pbuf->nDataLen;
				} else if (_rcv_size_last > (unsigned int) pbuf->nDataLen) {
					Terminate(__FF_LINE__);
				}else if ( !isTerminate() && time(NULL)>m_checkconntime ){
					if (m_checksignalstate==checksignalstate_no){
						stCheckSignalCmd cmd(true);
						sendcmd(&cmd,sizeof(cmd),Z_DEFAULT_COMPRESSION,(DWORD)-1);
						m_checkconntime=time(NULL)+check_signal_waittime();
						m_checksignalstate=checksignalstate_waitrecv;

					}else{
						g_logger.debug("%s:%d 测试信号返回超时...", GetRemoteAddress(), GetRemotePort());
						Terminate(__FF_LINE__);	
					}
				}
			}
		}else if (!m_boinitconn && m_szIp[0]!=0 && m_nport!=0){
			m_boinitconn=true;
			Open(m_szIp,m_nport);
		}else if (m_autoreconn && m_szIp[0]!=0 && m_nport!=0){
			Open(m_szIp,m_nport);
		}
		if (isTerminate() && IsConnected()){ __super::Close();};
		Sleep(nsleeptime);
	}
	__super::Close();
	return 0;
}
//----------------------------------------------------------------------------