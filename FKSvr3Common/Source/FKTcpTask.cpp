/**
*	created:		2013-4-7   22:37
*	filename: 		FKTcpTask
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/Network/FKTcpTask.h"
#include "../Include/FKLogger.h"
//------------------------------------------------------------------------
CLD_IocpClientSocket * CMIocpTcpAccepters::CSubAccepter::CreateIocpClient(SOCKET s)
{
	if (m_Service)
	{
		return m_Service->CreateIocpClient(this,s);
	}
	return NULL;
}
//------------------------------------------------------------------------
void CMIocpTcpAccepters::CSubAccepter::OnIocpClientConnect(CLD_Socket* Socket)
{
	if (m_Service)
	{
		return m_Service->OnIocpClientConnect(Socket);
	}
}
//------------------------------------------------------------------------
void CMIocpTcpAccepters::CSubAccepter::OnClientDisconnect(CLD_Socket* Socket){
	if (m_Service){return m_Service->OnClientDisconnect(Socket);}
}
//------------------------------------------------------------------------
void CMIocpTcpAccepters::CSubAccepter::OnClientError(CLD_Socket* Socket,CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg){
	if (m_Service){return m_Service->OnClientError(Socket,ErrEvent,nErrCode,sErrMsg);}
}
//------------------------------------------------------------------------
void CMIocpTcpAccepters::CSubAccepter::OnClientRead(CLD_Socket* Socket){
	FUNCTION_BEGIN;
	if (m_Service){ m_Service->OnClientRead(Socket);}
}
//------------------------------------------------------------------------
CMIocpTcpAccepters::CSubAccepter* CMIocpTcpAccepters::CreateAccepter(CLD_IocpHandle* iocphandle,CMIocpTcpAccepters* pService,
																	 const std::string &name,int type,int idx)
{
	return new CSubAccepter(iocphandle,this,name,type,idx);
}
//------------------------------------------------------------------------
int CMIocpTcpAccepters::put(CLD_IocpHandle* iocphandle,const unsigned short port,
							const char* name,int type,int idx,const char* ip,
							CMIocpTcpAccepters::CSubAccepter** pAccepter)
{
	FUNCTION_BEGIN;
	if (iocphandle && port>0)
	{
		AILOCKT(mapper);
		CSubAccepter* p=CreateAccepter(iocphandle,this,name,type,idx);
		if (p){
			if (pAccepter){	*pAccepter=p;}
			if(p->Open(ip,port)){mapper.insert(Sock2Port_value_type(port,p));return 0;}
			else{p->Close();delete p;}
		}
	}
	return -2;
}
//------------------------------------------------------------------------
int CMIocpTcpAccepters::remove(const unsigned short port)
{
	int nret=0;
	AILOCKT(mapper);
	Sock2Port_iterator it =mapper.find(port);
	if (it!=mapper.end()){
		CSubAccepter* p=it->second;
		if (p){
			p->Close();
			delete p;
			mapper.erase(it);
			nret++;
		}
	}
	return nret;
}
//------------------------------------------------------------------------
int CMIocpTcpAccepters::closeall()
{
	AILOCKT(mapper);
	int nret=mapper.size();
	for(Sock2Port_iterator it = mapper.begin(); it != mapper.end(); it++)
	{
		CSubAccepter* p=it->second;
		if (p){p->Close();}
	}
	return nret;
}
//------------------------------------------------------------------------
int CMIocpTcpAccepters::clear()
{
	AILOCKT(mapper);
	int nret=mapper.size();
	for(Sock2Port_iterator it = mapper.begin(); it != mapper.end(); it++)
	{
		CSubAccepter* p=it->second;
		if (p){p->Close();delete p;}
	}
	mapper.clear();
	return nret;
}
//------------------------------------------------------------------------
CLD_Accepter* CMIocpTcpAccepters::getservice(const unsigned short port)
{
	AILOCKT(mapper);
	Sock2Port_iterator it =mapper.find(port);
	if (it!=mapper.end()){
		CSubAccepter* p=it->second;
		return p;
	}
	return NULL;
}
//------------------------------------------------------------------------
CLD_Accepter* CMIocpTcpAccepters::getservice(unsigned short type,unsigned short idx){
	AILOCKT(mapper);

	for(Sock2Port_iterator it = mapper.begin(); it != mapper.end(); it++){
		CSubAccepter* p=it->second;
		if (p && p->gettype()==type && p->getidx()==idx){return p;}
	}
	return NULL;
}
//------------------------------------------------------------------------
bool CMTcpConnters::put(CLD_Connecter *pConnecter,const char* ip,short int nport,const char* name,unsigned short type,unsigned short idx){
	if (pConnecter){
		stConnecterInfo info;
		info.actiontime=time(NULL)+m_actiontime;
		info.idx=idx;
		info.type=type;
		info.connecterdis=name;
		info.connip=ip;
		info.nport=nport;
		AILOCKT(mapper);
		if (mapper.find(pConnecter)!=mapper.end()){return false;}
		mapper.insert(Connecter2time_value_type(pConnecter,info));
		m_nextactiontime=0;
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
CLD_Connecter* CMTcpConnters::getconnecter(unsigned short type,unsigned short idx,stConnecterInfo*& info){
	AILOCKT(mapper);

	for(Connecter2time_iterator it = mapper.begin(); it != mapper.end(); it++){
		info=&it->second;
		if (info->type==type && info->idx==idx){return it->first;}
	}
	return NULL;
}
//------------------------------------------------------------------------
int CMTcpConnters::closeall(){
	AILOCKT(mapper);
	int nret=mapper.size();
	for(Connecter2time_iterator it = mapper.begin(); it != mapper.end(); it++){
		CLD_Connecter* pConnecter=it->first;
		if (pConnecter){pConnecter->Close();}
	}
	return nret;
}
//------------------------------------------------------------------------
int CMTcpConnters::clear(){
	AILOCKT(mapper);
	int nret=mapper.size();
	for(Connecter2time_iterator it = mapper.begin(); it != mapper.end(); it++){
		CLD_Connecter* pConnecter=it->first;
		if (pConnecter){
			pConnecter->Close();
			delete pConnecter;
			pConnecter=NULL;
		}
	}
	mapper.clear();
	return nret;
}
//------------------------------------------------------------------------
void CMTcpConnters::timeAction(bool boforcenow){
	DWORD now=time(NULL);
	if (boforcenow){ now=(safe_max(m_nextactiontime,now)+m_actiontime+1); }
	if (now>=m_nextactiontime){

		m_nextactiontime=now+safe_max<DWORD>((m_actiontime/4),2);
		AILOCKT(mapper);
		for(Connecter2time_iterator it = mapper.begin(); it != mapper.end(); it++){
			if (now>=it->second.actiontime){
				CLD_Connecter* pConnecter=it->first;
				if (pConnecter && !pConnecter->IsConnected() && pConnecter->SocketCanRecycle()){
					if (pConnecter){
						g_logger.debug("³¢ÊÔÁ¬½Ó %s:%d ( %s:%d )",it->second.connecterdis.c_str(),it->second.idx,it->second.connip.c_str(),it->second.nport);
						pConnecter->Open(it->second.connip.c_str(),it->second.nport);
					}
				}
				it->second.actiontime=now+m_actiontime;
			}
		}
	}
}
//------------------------------------------------------------------------
bool CMIocpTcpConnters::put(CLD_Connecter *pConnecter,const char* ip,short int nport,const char* name,unsigned short type,unsigned short idx){
	if (dynamic_cast<CLD_IocpConnecter*>(pConnecter)!=NULL){
		return CMTcpConnters::put(pConnecter,ip,nport,name,type,idx);
	}
	return false;
}
//------------------------------------------------------------------------
int CMIocpTcpConnters::closeall(){
	return clear();
}
//------------------------------------------------------------------------
int CMIocpTcpConnters::clear(){
	INFOLOCK(mapper);
	int nret=mapper.size();
	UNINFOLOCK(mapper);

	CLD_IocpConnecter* pConnecter=NULL;
	while (true){
		pConnecter=NULL;
		INFOLOCK(mapper);
		if (mapper.begin()!=mapper.end()){
			pConnecter=((CLD_IocpConnecter*)mapper.begin()->first);
			mapper.erase(mapper.begin());
		}
		UNINFOLOCK(mapper);
		if (pConnecter){


			pConnecter->GetIocpObj().RecycleMe();
			pConnecter=NULL;
		}
		INFOLOCK(mapper);
		if (mapper.begin()==mapper.end()){UNINFOLOCK(mapper);return nret;}
		UNINFOLOCK(mapper);
	}
	return nret;
}
//------------------------------------------------------------------------