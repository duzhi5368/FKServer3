/**
*	created:		2013-4-6   23:18
*	filename: 		FKLogger
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKLogger.h"
#include "../Include/FKTime.h"
#include <time.h>
#include "../Include/Dump/FKDumpErrorBase.h"
#include "../Include/FKBaseDefine.h"
//------------------------------------------------------------------------
class stUpdatLogFileThread : public CThread
{
public:
	stUpdatLogFileThread():CThread(true,NULL){}
	~stUpdatLogFileThread(){

		save();
		Resume();
		Terminate();
		Waitfor();		

		INFOLOCK(zLogger::m_loggers);
		zLogger::m_loggers.clear();
		UNINFOLOCK(zLogger::m_loggers);
	}
	virtual int Run(void *pvParam);
	static bool save();
};
//------------------------------------------------------------------------
stUpdatLogFileThread UpdatLogFileThread;
//------------------------------------------------------------------------
zLogger::zLogger(const std::string &name)
{
	m_ShowLogFunc=NULL;
	m_nShowLvl=2;		
	m_nWriteLvl=-1;
	m_nLogidx=0;
	m_ncurpos=0;
	m_nlogbytes=0x7fffffff;
	m_msgbuf[m_ncurpos]=0;
	m_basefilepath="";
	m_name="";
	if (name!=""){
		m_basefilepath="./log/";
		m_basefilepath += name;
		m_basefilepath += "/";
		setName(name);
	}
	m_nZoneID=0;
	m_boFor360=false;
	AILOCKT(m_loggers);
	m_loggers.push_back(this);
}
//------------------------------------------------------------------------
zLogger::~zLogger(){
	AILOCKT(m_loggers);
	m_loggers.erase(remove(m_loggers.begin(),m_loggers.end(),this),m_loggers.end());
	m_ShowLogFunc=NULL;
}
//------------------------------------------------------------------------
void zLogger::ShowLog(zLevel& level,const char* logtime,const char* pszMsg){
	FUNCTION_BEGIN;
	if (pszMsg && m_ShowLogFunc){
		m_ShowLogFunc(level,logtime,(char *)pszMsg);
	}
}
//------------------------------------------------------------------------
const std::string & zLogger::getName(){
	return m_name;
}
//------------------------------------------------------------------------
void zLogger::setName(const std::string & setName){
	m_name=setName;
}
//------------------------------------------------------------------------
int zLogger::strlevltoint(const std::string & level)
{
	if (stricmp("off",level.c_str())==0)
		return zLogger::eOFF;
	else if (stricmp("fatal",level.c_str())==0)
		return zLogger::eFATAL;
	else if (stricmp("alarm",level.c_str())==0)
		return zLogger::eALARM;
	else if (stricmp("error",level.c_str())==0)
		return zLogger::eERROR;
	else if (stricmp("iffy",level.c_str())==0)
		return zLogger::eIFFY;
	else if (stricmp("warn",level.c_str())==0 || stricmp("warning",level.c_str())==0)
		return zLogger::eWARN;
	else if (stricmp("trace",level.c_str())==0)
		return zLogger::eTRACE;
	else if (stricmp("info",level.c_str())==0)
		return zLogger::eINFO;
	else if (stricmp("gbug",level.c_str())==0)
		return zLogger::eGBUG;
	else if (stricmp("debug",level.c_str())==0)
		return zLogger::eDEBUG;
	else if (stricmp("all",level.c_str())==0 || stricmp("always",level.c_str())==0)
		return zLogger::eALL;
	else
	{
		return atoi(level.c_str());
	}
}
//------------------------------------------------------------------------
void zLogger::setLevel(int writelevel,int showlvl)
{
	FUNCTION_BEGIN;
	m_nWriteLvl=writelevel;
	m_nShowLvl=showlvl;
}
//------------------------------------------------------------------------
void zLogger::SetZoneID(int nZoneID)
{
	if (nZoneID!=0)
	{
		m_nZoneID=nZoneID;
	}
}
//------------------------------------------------------------------------
void zLogger::SetBoFor360(bool boYesNo)
{
	m_boFor360=boYesNo;
}
//------------------------------------------------------------------------
void zLogger::setLevel(const std::string & writelevel,const std::string & showlvl)
{
	FUNCTION_BEGIN;
	m_nWriteLvl=strlevltoint(writelevel);
	m_nShowLvl=strlevltoint(showlvl);
}
//------------------------------------------------------------------------
void zLogger::fixlogpath(std::string &basepath)
{
	char szSvridxPath[MAX_PATH]={0};
	strcpy_s(szSvridxPath,sizeof(szSvridxPath),basepath.c_str());
	int nPathLen=strlen(szSvridxPath);
	if (nPathLen>0)
	{
		replaceFrontlashPath(szSvridxPath);
		if (szSvridxPath[nPathLen-1]=='\\')
		{
			szSvridxPath[nPathLen-1]='\0';
			nPathLen--;
		}
		basepath=szSvridxPath;
	}
}
//------------------------------------------------------------------------
bool zLogger::SetLocalFileBasePath(const std::string &basefilepath)
{
	FUNCTION_BEGIN;
	m_nlogbytes=0x7fffffff;
	m_basefilepath=basefilepath;
	fixlogpath(m_basefilepath);
	UpdatLogFileThread.Start(false,NULL);
	return CheckLogPath();
}
//------------------------------------------------------------------------
const int TEMPBUFSIZE=1024*10;
//------------------------------------------------------------------------
#define  _ZLOGGER_TIMEFORMAT_	"%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d"
#define  _ZLOGGER_TIMELEN_		20
//------------------------------------------------------------------------
bool zLogger::logbylevel(zLevel& level,const char *tempmessage)
{
	FUNCTION_BEGIN;
	if (tempmessage && (level.showlevel<=m_nShowLvl || level.writelevel<=m_nWriteLvl)){
		char message[TEMPBUFSIZE]={0};
		timetostr(time(NULL),message,_ZLOGGER_TIMELEN_,_ZLOGGER_TIMEFORMAT_);
		message[_ZLOGGER_TIMELEN_-1]=0;
		sprintf_s(&message[_ZLOGGER_TIMELEN_],sizeof(message)-32, "%.8s (%u:%d:%d) %s\r\n",level.name ,m_nLogidx,::GetCurrentProcessId(),::GetCurrentThreadId(),tempmessage);
		if (level.showlevel<=m_nShowLvl){
			ShowLog(level,message,&message[_ZLOGGER_TIMELEN_]);
		}
		if (level.writelevel<=m_nWriteLvl){
			message[_ZLOGGER_TIMELEN_-1]='\x20';
			if (level.realtimewrite){
				WriteLog(message,strlen(message));
			}else{
				AddMsg2buf(message,strlen(message));
			}
		}
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::log(zLevel& level,const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (level.showlevel<=m_nShowLvl || level.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(level,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::realtimeLog(zLevel& level,const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (level.showlevel<=m_nShowLvl || m_nWriteLvl>=0)){
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	

		char message[TEMPBUFSIZE]={0};
		timetostr(time(NULL),message,_ZLOGGER_TIMELEN_,_ZLOGGER_TIMEFORMAT_);
		message[_ZLOGGER_TIMELEN_-1]=0;
		sprintf_s(&message[_ZLOGGER_TIMELEN_],sizeof(message)-32, "%.8s (%u:%d:%d) %s\r\n",level.name ,m_nLogidx,::GetCurrentProcessId(),::GetCurrentThreadId(),tempmessage);
		if (level.showlevel<=m_nShowLvl){
			ShowLog(level,message,&message[_ZLOGGER_TIMELEN_]);
		}
		if (m_nWriteLvl>=0){
			message[_ZLOGGER_TIMELEN_-1]='\x20';
			WriteLog(message,strlen(message));
		}
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::forceLog(zLevel& level,const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (m_nShowLvl>=0 || m_nWriteLvl>=0))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	

		char message[TEMPBUFSIZE]={0};
		timetostr(time(NULL),message,_ZLOGGER_TIMELEN_,_ZLOGGER_TIMEFORMAT_);
		message[_ZLOGGER_TIMELEN_-1]=0;
		sprintf_s(&message[_ZLOGGER_TIMELEN_],sizeof(message)-32, "%.5s (force:%u:%d:%d) %s\r\n",level.name,m_nLogidx,::GetCurrentProcessId(),::GetCurrentThreadId(),tempmessage);
		if (m_nShowLvl>=0){
			ShowLog(zFORCE,message,&message[_ZLOGGER_TIMELEN_]);
		}
		if (m_nWriteLvl>=0){
			message[_ZLOGGER_TIMELEN_-1]='\x20';
			if (level.realtimewrite){
				WriteLog(message,strlen(message));
			}else{
				AddMsg2buf(message,strlen(message));
			}
		}
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::debug(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zDEBUG.showlevel<=m_nShowLvl || zDEBUG.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zDEBUG,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::error(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zERROR.showlevel<=m_nShowLvl || zERROR.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zERROR,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::error_out(const char * pattern)
{
	FUNCTION_BEGIN;
	if (pattern && (zERROR.showlevel<=m_nShowLvl || zERROR.writelevel<=m_nWriteLvl))
	{
		logbylevel(zERROR,pattern);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::info(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zINFO.showlevel<=m_nShowLvl || zINFO.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zINFO,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::fatal(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zFATAL.showlevel<=m_nShowLvl || zFATAL.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zFATAL,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::warn(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zWARN.showlevel<=m_nShowLvl || zWARN.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zWARN,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::alarm(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zALARM.showlevel<=m_nShowLvl || zALARM.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zALARM,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::iffy(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zIFFY.showlevel<=m_nShowLvl || zIFFY.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zIFFY,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::trace(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zTRACE.showlevel<=m_nShowLvl || zTRACE.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zTRACE,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::gbug(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	if (pattern && (zGBUG.showlevel<=m_nShowLvl || zGBUG.writelevel<=m_nWriteLvl))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	
		logbylevel(zGBUG,tempmessage);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::WriteLog(char* pmsg,int nlen)
{
	FUNCTION_BEGIN;

	AddMsg2buf(pmsg,nlen);
	UpdatLogFile();
	return true;
}
//------------------------------------------------------------------------
bool zLogger::CheckLogPath()
{
	FUNCTION_BEGIN;

	if (m_nlogbytes>=(MSGBUF_MAX*2-1024))
	{
		char filepath[MAX_PATH]={0};
		strcpy_s(filepath,sizeof(filepath),m_basefilepath.c_str());
		int nPathLen=strlen(filepath);
		if (nPathLen<=0){ return false;}
		replaceFrontlashPath(filepath);

		if (!(filepath[nPathLen-1]=='\\')){
			filepath[nPathLen]='\\';
			nPathLen++;
		}
		if (!m_boFor360)
		{
			time_t ti = time(NULL);
			tm* t = localtime(&ti);
			sprintf_s(&filepath[nPathLen],sizeof(filepath)-nPathLen,"%.4d%.2d%.2d\\\0",t->tm_year+1900, t->tm_mon+1, t->tm_mday);

			char szfile[MAX_PATH]={0};
			sprintf_s(szfile,sizeof(szfile),"%s%.2d%.2d%.2d.log\0",filepath,t->tm_hour, t->tm_min,t->tm_sec);

			m_logout.SetFileName(szfile);
			FileSystem::createPath(filepath);
			m_nlogbytes=0;
		}
		else
		{
			time_t ti = time(NULL);
			tm* t = localtime(&ti);
			char szfile[MAX_PATH]={0};
			sprintf_s(szfile,sizeof(szfile),"%s%s_S%d_%.2d%.2d%.2d.log\0",filepath,"xyol",m_nZoneID,t->tm_year+1900, t->tm_mon+1, t->tm_mday);
			char sztime[MAX_PATH]={0};
			sprintf_s(sztime,sizeof(sztime),"%.2d%.2d%.2d",t->tm_year+1900, t->tm_mon+1, t->tm_mday);
			m_logout.Set360FileTime(sztime);
			m_logout.SetFileName(szfile);
			FileSystem::createPath(filepath);
			m_nlogbytes=0;
		}

		return true;
	}else if (!FileSystem::IsFileExist(m_logout.GetFileName())){
		FileSystem::createPath(extractfilepath(m_logout.GetFileName()));
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::UpdatLogFile()
{
	FUNCTION_BEGIN;
	AILOCKP(n_logInpLock);
	if (m_ncurpos>0)
	{
		CheckLogPath();
		m_nlogbytes += m_ncurpos;
		if (!m_boFor360)
		{
			m_logout.WriteString(m_msgbuf);
		}		
		else
		{
		}
		InterlockedIncrement(&m_nLogidx);

		m_ncurpos=0;
		m_msgbuf[m_ncurpos]=0;
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::AddMsg2buf(char* pmsg,int nlen)
{
	FUNCTION_BEGIN;

	if (nlen<1024*8)
	{
		AILOCKP(n_logInpLock);
		int n=m_ncurpos+nlen;
		if (n>=(MSGBUF_MAX-1024))
		{
			UpdatLogFile();
			n=m_ncurpos+nlen;
			if (n>=(MSGBUF_MAX-1024))
			{
				char message[TEMPBUFSIZE]={0};
				timetostr(time(NULL),message,_ZLOGGER_TIMELEN_,_ZLOGGER_TIMEFORMAT_);
				message[_ZLOGGER_TIMELEN_-1]=0;
				sprintf_s(&message[_ZLOGGER_TIMELEN_],sizeof(message)-32, "[ Error:日志缓冲区不足(%u : %u : %u) ] %s\r\n",m_ncurpos,nlen,m_nlogbytes,pmsg);

				zLogger::zLevel tempFATAL("FATAL",zLogger::eFATAL,zLogger::eFATAL,true);
				ShowLog(tempFATAL,message,&message[_ZLOGGER_TIMELEN_]);
				message[_ZLOGGER_TIMELEN_-1]='\x20';
				m_logout.WriteString(message);

				return false;
			}
		}
		memcpy(&m_msgbuf[m_ncurpos],pmsg,nlen);
		m_ncurpos+=nlen;
		m_msgbuf[m_ncurpos]=0;
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::save(){return stUpdatLogFileThread::save();}
//------------------------------------------------------------------------
bool zLogger::lualog( zLevel& level, const char * pattern, ... )
{
	if (pattern && (m_nShowLvl>=0 || m_nWriteLvl>=0))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	

		char message[TEMPBUFSIZE]={0};
		timetostr(time(NULL),message,_ZLOGGER_TIMELEN_,_ZLOGGER_TIMEFORMAT_);
		message[_ZLOGGER_TIMELEN_-1]=0;
		sprintf_s(&message[_ZLOGGER_TIMELEN_],sizeof(message)-32, "%.5s (force:%u:%d:%d) %s\r\n",level.name,m_nLogidx,::GetCurrentProcessId(),::GetCurrentThreadId(),tempmessage);

		if (m_nWriteLvl>=0)
		{
			message[_ZLOGGER_TIMELEN_-1]='\x20';
			WriteLog(message,strlen(message));
		}
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool zLogger::forceLogFile( zLevel& level,const char * pattern, ... )
{
	if (pattern && (m_nShowLvl>=0 || m_nWriteLvl>=0))
	{
		char tempmessage[TEMPBUFSIZE]={0};
		va_list ap;	
		va_start(ap, pattern);		
		_safe_vsnprintf(tempmessage, (sizeof(tempmessage)) - 32, pattern, ap);	
		va_end(ap);	

		char message[TEMPBUFSIZE]={0};
		timetostr(time(NULL),message,_ZLOGGER_TIMELEN_,_ZLOGGER_TIMEFORMAT_);
		message[_ZLOGGER_TIMELEN_-1]=0;
		sprintf_s(&message[_ZLOGGER_TIMELEN_],sizeof(message)-32, "%.5s (force:%u:%d:%d) %s\r\n",level.name,m_nLogidx,::GetCurrentProcessId(),::GetCurrentThreadId(),tempmessage);
		if (m_nShowLvl>=0){
			ShowLog(zFORCE,message,&message[_ZLOGGER_TIMELEN_]);
		}
		if (m_nWriteLvl>=0){
			message[_ZLOGGER_TIMELEN_-1]='\x20';
			if (level.realtimewrite){
				WriteLog(message,strlen(message));
			}else{
				AddMsg2buf(message,strlen(message));
			}
		}
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
bool stUpdatLogFileThread::save()
{
	CSyncVector<zLogger*> m_templist;

	INFOLOCK(zLogger::m_loggers);
	CSyncVector<zLogger*>::iterator it;
	for (it=zLogger::m_loggers.begin();it!= zLogger::m_loggers.end();it++){
		if ((*it)!=NULL){ m_templist.push_back(*it);};
	}
	UNINFOLOCK(zLogger::m_loggers);

	for (it=m_templist.begin();it!= m_templist.end();it++){
		(*it)->UpdatLogFile();
	}
	return true;
}
//------------------------------------------------------------------------
int stUpdatLogFileThread::Run(void *pvParam){
	time_t dwRunTime=0;
	SetPriority(THREAD_PRIORITY_IDLE);
	CSyncVector<zLogger*> m_templist;
	while (!IsTerminated()){
		INFOLOCK(zLogger::m_loggers);
		CSyncVector<zLogger*>::iterator it;

		if (zLogger::m_loggers.size()>0){
			if (time(NULL)>dwRunTime){
				dwRunTime=time(NULL)+3;
				for (it=zLogger::m_loggers.begin();it!= zLogger::m_loggers.end();it++){

					if (_random_d(10)<=2){if ((*it)!=NULL){ m_templist.push_back(*it);};}
				}
			}
		}
		UNINFOLOCK(zLogger::m_loggers);
		if (m_templist.size()>0){
			for (it=m_templist.begin();it!= m_templist.end();it++){
				(*it)->UpdatLogFile();
			}
		}
		m_templist.clear();
		Sleep(1000);
	}

	save();
	return 0;
}
//------------------------------------------------------------------------