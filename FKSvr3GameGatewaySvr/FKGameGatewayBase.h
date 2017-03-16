/**
*	created:		2013-4-9   17:11
*	filename: 		FKGameGatewayBase
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKSvr3Common/FKCommonInclude.h"
//------------------------------------------------------------------------
class CGatewayConnManage:public CIntLock
{
public:
	struct CGatewayConn{
		void* obj;
		long sidx;	
		CGatewayConn(){ obj=NULL;sidx=INVALID_SOCKET;}
	};
	typedef std::vector< CGatewayConn >	 CGWCVector;
	typedef CGWCVector::iterator gwc_iter;
public:
	CGatewayConnManage();
	virtual ~CGatewayConnManage();

	int add(long sidx,void* po);
	bool modify(int nidx,long sidx,void* po);
	int remove(int nidx,long sidx);
	int removeall();

	int size(){return m_count;};

	gwc_iter begin(){ return m_v.begin(); }
	gwc_iter end(){ return m_v.end(); }

	template < class _To > bool get(int nidx,long sidx,_To& po){
		AILOCKT(*this);
		if (nidx>=0 && ((size_t)nidx)<m_v.size()){ if (m_v[nidx].sidx==sidx){ po=(_To)m_v[nidx].obj; return true; } }
		nidx=getindex(sidx);
		if (nidx>=0){ po=(_To)m_v[nidx].obj; return true; }
		return false;
	}
protected:
	int	getemptyindex();
	int getindex(long sidx);
private:
	CGWCVector m_v;
	std::queue<WORD> m_idxs;
	int m_count;
};
//------------------------------------------------------------------------
#define GW_PACKCHECKSUM			0x5533
//------------------------------------------------------------------------
#define GW_TESTCMD				0
#define GW_SUB_TEST				0
#define GW_SUB_TESTOK			1
#define GW_SUB_TESTDATA			10
#define GW_SUB_TESTDATAOK		11
#define GW_OPEN					20		
#define GW_OPENRET				21		
#define GW_CLOSE				30		
#define GW_KICK					40		
#define GW_DATA					50		
#define GW_DATA2USERS			51		
#define GW_SVRCMD				60		
#define GW_BUILDMSG				61
//------------------------------------------------------------------------
template < CMD_TYPE cmdvalue, SUBCMD_TYPE subcmdvalue >
struct stProxyPacketCmd:public stCmdBase<cmdvalue,subcmdvalue>
{
	const WORD numCheckSum;
	stProxyPacketCmd():numCheckSum(GW_PACKCHECKSUM){};
};
//------------------------------------------------------------------------
DEFINEUSERCMD1(stProxyTestBaseCmd,GW_TESTCMD);
DEFINEDATACMD1(stProxyTestCmd,stProxyTestBaseCmd);
//------------------------------------------------------------------------
typedef stProxyTestCmdEx2<DWORD ,long , GW_SUB_TEST>		stProxyTestSockCmd;
typedef stProxyTestCmdEx2<DWORD ,long , GW_SUB_TESTOK>		stProxyTestSockRetCmd;
typedef stProxyTestCmdEx3<DWORD ,long ,stZeroArray<char>, GW_SUB_TESTDATA>		stProxyTestDataCmd;
typedef stProxyTestCmdEx3<DWORD ,long ,stZeroArray<char>, GW_SUB_TESTDATAOK>		stProxyTestDataRetCmd;
//------------------------------------------------------------------------
struct stOpenCmd:public stProxyPacketCmd<GW_OPEN,0>
{
	long	sidx;
	SOCKET  socket_handle;
	in_addr clientip;
	WORD	wclientport;			
	in_addr	gateip;		
	WORD	wgateport;			
	__int64	addparam;			

	WORD	wgindex;		
	stZeroArray<char> pluscmd;
};
//------------------------------------------------------------------------
struct stCloseCmd:public stProxyPacketCmd<GW_CLOSE,0>
{
	long	sidx;
	WORD	wsvridx;		
	WORD	wgindex;
	stZeroArray<char> pluscmd;
};
//------------------------------------------------------------------------
struct stOpenRetCmd:public stProxyPacketCmd<GW_OPENRET,0>
{
	long	sidx;
	WORD	wsvridx;		
	WORD	wgindex;
	stZeroArray<char> pluscmd;
};
//------------------------------------------------------------------------
struct stKickCmd:public stProxyPacketCmd<GW_KICK,0>
{
	long	sidx;
	WORD	wsvridx;		
	WORD	wgindex;
	stZeroArray<char> pluscmd;
};
//------------------------------------------------------------------------
struct stProxyDataCmd:public stProxyPacketCmd<GW_DATA,0>
{
	long	sidx;
	WORD	wsvridx;		
	WORD	wgindex;
	stZeroArray<char> pluscmd;
};
//------------------------------------------------------------------------
struct stGameSvrBuildMsg : public stProxyPacketCmd<GW_BUILDMSG,0>
{
	stZeroArray<char> pluscmd;
};
//------------------------------------------------------------------------
struct stProxyData2UsersCmd:public stProxyPacketCmd<GW_DATA2USERS,0>
{
	struct stProxyUserIndex{
		long	sidx;
		WORD	wsvridx;		
		WORD	wgindex;
	};
	int ncmdlen;
	stZeroArray<stProxyUserIndex> broadcastusers;

	stProxyData2UsersCmd(){	ncmdlen=0; }

	char* getpluscmd(){
		return ((char*)(((char*)&broadcastusers)+broadcastusers.getarraysize()+sizeof(broadcastusers)));
	};

	bool setpluscmd(const char* pcmd,int ninlen){
		char* cmdbuf=getpluscmd();
		memcpy(cmdbuf,pcmd,ninlen);
		ncmdlen=ninlen;
		return true;
	}
};
//------------------------------------------------------------------------
struct stProxySvrCmd:public stProxyPacketCmd<GW_SVRCMD,0>
{
	stZeroArray<char> pluscmd;
};
//------------------------------------------------------------------------