/**
*	created:		2013-4-7   22:35
*	filename: 		FKTcpTask
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKBaseDefine.h"
#include "FKIocp.h"
#include <string>
#include "../FKNoncopyable.h"
#include <map>
//------------------------------------------------------------------------
//用户层的管理器方便使用
class CMIocpTcpAccepters : private zNoncopyable
{
public:
	class CSubAccepter:public CLD_IocpAccepter
	{
	public:
		friend class CLD_IocpObj;
		CSubAccepter(CLD_IocpHandle* iocphandle,CMIocpTcpAccepters* pService=NULL,const std::string &name="",int type=0,int idx=0)
			:m_Service(pService),svrdis(name),ntype((unsigned short)type),nindex((unsigned short)idx),CLD_IocpAccepter(*iocphandle){};

		virtual CLD_IocpClientSocket * CreateIocpClient(SOCKET s);

		virtual void OnIocpClientConnect(CLD_Socket* Socket);
		virtual void OnClientDisconnect(CLD_Socket* Socket);
		virtual void OnClientError(CLD_Socket* Socket,CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg);
		virtual void OnClientRead(CLD_Socket* Socket);

		unsigned short gettype(){ return ntype;}
		unsigned short getidx(){ return nindex;}
		const char* getdis(){ return svrdis.c_str();}
		CMIocpTcpAccepters* getservice(){ return m_Service;}
	protected:
		CMIocpTcpAccepters* m_Service;
		unsigned short ntype;
		unsigned short nindex;
		std::string svrdis;
	};
public:
	typedef std::CSyncMap< unsigned short,CSubAccepter*> Sock2Port;
	typedef Sock2Port::tbase::value_type Sock2Port_value_type;
	typedef Sock2Port::tbase::iterator Sock2Port_iterator;
	typedef Sock2Port_iterator	Sock2IpPort_iterator;

	CMIocpTcpAccepters(const std::string &name):m_name(name){};

	//必须自己调用clear清理所有对象
	virtual ~CMIocpTcpAccepters(){};

	int put(CLD_IocpHandle* iocphandle,const unsigned short port,const char* name="",int type=0,int idx=0,const char* ip="0.0.0.0",CMIocpTcpAccepters::CSubAccepter** pAccepter=NULL);
	int remove(const unsigned short port);

	virtual int clear();
	virtual int closeall();

	CLD_Accepter* getservice(const unsigned short port);
	CLD_Accepter* getservice(unsigned short type,unsigned short idx);

	std::string& getname(){return m_name;};
	Sock2Port& getall(){return mapper;};
	Sock2Port_iterator begin(){return mapper.begin();}
	Sock2Port_iterator end(){return mapper.end();}

	virtual CSubAccepter* CreateAccepter(CLD_IocpHandle* iocphandle,CMIocpTcpAccepters* pService=NULL,const std::string &name="",int type=0,int idx=0);
	virtual CLD_IocpClientSocket * CreateIocpClient(CSubAccepter* pAccepter ,SOCKET s)=0;

	virtual void OnIocpClientConnect(CLD_Socket* Socket){};
	virtual void OnClientDisconnect(CLD_Socket* Socket){};
	virtual void OnClientError(CLD_Socket* Socket,CLD_TErrorEvent ErrEvent,OUT int &nErrCode,char * sErrMsg){};
	virtual void OnClientRead(CLD_Socket* Socket){};
protected:
	std::string m_name;
	Sock2Port mapper;
};
//------------------------------------------------------------------------
class CMTcpConnters : private zNoncopyable
{
public:
	struct stConnecterInfo{
		DWORD actiontime;
		unsigned short type;
		unsigned short idx;
		std::string connip;
		unsigned short nport;
		std::string connecterdis;
	};
	typedef std::CSyncMap< CLD_Connecter *,stConnecterInfo> Connecter2time;
	typedef Connecter2time::tbase::value_type Connecter2time_value_type;
	typedef Connecter2time::tbase::iterator Connecter2time_iterator;

	CMTcpConnters():m_actiontime(10),m_nextactiontime(0){};
	virtual ~CMTcpConnters(){};

	virtual bool put(CLD_Connecter *pConnecter,const char* ip,short int nport,const char* name,unsigned short type,unsigned short idx);
	virtual int clear();
	virtual int closeall();

	virtual void timeAction(bool boforcenow=false);
	Connecter2time& getall(){return mapper;};
	Connecter2time_iterator begin(){return mapper.begin();}
	Connecter2time_iterator end(){return mapper.end();}

	CLD_Connecter* getconnecter(unsigned short type,unsigned short idx,stConnecterInfo*& info);

	void setActiontime(int ntime){m_actiontime=ntime;}
protected:
	Connecter2time mapper;
	DWORD m_actiontime;
	DWORD m_nextactiontime;
};
//------------------------------------------------------------------------
class CMIocpTcpConnters : public CMTcpConnters
{
public:
	virtual ~CMIocpTcpConnters(){};
	virtual bool put(CLD_Connecter *pConnecter,const char* ip,short int nport,const char* name,unsigned short type,unsigned short idx);
	//下面2个函数要放在IOCP关闭前调用 与IOCP绑定的对象在IOCP关闭时会被释放
	//不同的关闭和释放可以重载...
	virtual int clear();
	virtual int closeall();
};
//------------------------------------------------------------------------  