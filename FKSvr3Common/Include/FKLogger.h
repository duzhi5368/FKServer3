/**
*	created:		2013-4-6   23:07
*	filename: 		FKLogger
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKSyncObjLock.h"
#include <string>
#include "FKBaseDefine.h"
#include "FKThread.h"
#include "FKOutput.h"
#include "STLTemplate/FKSyncList.h"
//------------------------------------------------------------------------
#pragma warning(disable:4244) 
//------------------------------------------------------------------------
#define MSGBUF_MAX	64*1024
//------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------
class zLogger
{
public:
	enum eLoggerLevel
	{
		eOFF=0,
		eFORCE=0,
		eFATAL=1,
		eERROR=2,
		eALARM=2,
		eWARN=3,
		eIFFY=3,
		eINFO=4,
		eTRACE=4,
		eDEBUG=5,
		eGBUG=5,
		eALL=250,
	};
public:
	struct zLevel 
	{
		const char* name;
		const BYTE writelevel;
		const BYTE showlevel;
		const bool realtimewrite;
		DWORD showcolor;

		zLevel(const char* sname,BYTE btwritelevel,BYTE btshowlevel,bool borealtimewrite,DWORD dwshowcolor=0)
			:name(sname),writelevel(btwritelevel),showcolor(dwshowcolor),
			showlevel(btshowlevel),realtimewrite(borealtimewrite){
		}
	};

	typedef void(__stdcall *pShowLogFunc)(zLevel& level,const char* logtime,const char* pszMsg);

	static zLevel zOFF;
	static zLevel zFORCE;
	static zLevel zFATAL;
	static zLevel zERROR;
	static zLevel zALARM;
	static zLevel zWARN;
	static zLevel zIFFY;
	static zLevel zINFO;
	static zLevel zTRACE;
	static zLevel zDEBUG;
	static zLevel zGBUG;
public:
	zLogger(const std::string & name="CLD_Logger");
	~zLogger();
public:
	virtual void ShowLog(zLevel& level,const char* logtime,const char* pszMsg);

	const std::string & getName();
	void setName(const std::string & setName);

	virtual void setLevel(const std::string & writelevel,const std::string & showlvl="6");
	virtual void setLevel(int writelevel,int showlvl=6);
	virtual bool SetLocalFileBasePath(const std::string &basefilepath);

	void SetZoneID(int nZoneID);
	void SetBoFor360(bool boYesNo);

	bool log(zLevel& level,const char * pattern, ...);

	bool forceLog(zLevel& level,const char * pattern, ...);
	bool forceLogFile(zLevel& level,const char * pattern, ...);

	bool realtimeLog(zLevel& level,const char * pattern, ...);

	bool debug(const char * pattern, ...);
	bool error(const char * pattern, ...);
	bool error_out(const char * pattern);
	bool info(const char * pattern, ...);
	bool lualog( zLevel& level, const char * pattern, ...);
	bool fatal(const char * pattern, ...);
	bool warn(const char * pattern, ...);
	bool alarm(const char * pattern, ...);
	bool iffy(const char * pattern, ...);
	bool trace(const char * pattern, ...);
	bool gbug(const char * pattern, ...);

	virtual void SetShowLogFunc(pShowLogFunc pfunc)
	{
		m_ShowLogFunc=pfunc;
	};

	int showlvl(){return m_nShowLvl;}
	int writelvl(){return m_nWriteLvl;}

	bool logbylevel(zLevel& level,const char *tempmessage);
protected:
	friend class stUpdatLogFileThread;

	virtual void fixlogpath(std::string &basepath);
	virtual bool WriteLog(char* pmsg,int nlen);
	virtual bool AddMsg2buf(char* pmsg,int nlen);
	virtual bool UpdatLogFile();
	virtual bool CheckLogPath();

	virtual int strlevltoint(const std::string & level);
private:
	pShowLogFunc m_ShowLogFunc;

	char m_msgbuf[MSGBUF_MAX];	
	int m_ncurpos;

	int m_nShowLvl;
	int m_nWriteLvl;
	int m_nlogbytes;
	int m_nZoneID;
	bool m_boFor360;
	std::string m_name;
	std::string m_basefilepath;
	COutput m_logout;
	CIntLock n_logoutlock;
	CInpLock n_logInpLock;
	long m_nLogidx;

	static CSyncVector<zLogger*> m_loggers;
public:
	static bool save();
};
//------------------------------------------------------------------------
extern zLogger g_logger;
extern zLogger g_luaLogger;
extern zLogger logger360;
extern zLogger g_FameLogger;
//------------------------------------------------------------------------