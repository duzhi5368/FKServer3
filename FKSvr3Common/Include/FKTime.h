/**
*	created:		2013-4-6   23:04
*	filename: 		FKTime
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKBaseDefine.h"
#include "FKMisc.h"
#include <time.h>
#include <string>
#include "FKTimeEx.h"
#include "FKVsVer8Define.h"
//------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------
class zRTime
{
private:
	unsigned long long _msecs;
private:
	unsigned long long _now()
	{
		return GetTimeMsecs();
	}
	void nowByDelay(int delay)
	{
		_msecs = _now();
		addDelay(delay);
	}
public:
	zRTime(const int delay = 0)
	{
		nowByDelay(delay);
	}
	zRTime(const zRTime &rt)
	{
		_msecs = rt._msecs;
	}
	void now()
	{
		_msecs = _now();
	}
	unsigned long sec() const
	{
		return unsigned long(_msecs / 1000);
	}
	unsigned long msec() const
	{
		return unsigned long(_msecs % 1000);
	}
	unsigned long long msecs() const
	{
		return _msecs;
	}
	void setmsecs(unsigned long long data)
	{
		_msecs = data;
	}
	void addDelay(int delay)
	{
		_msecs += delay;
	}
	zRTime & operator= (const zRTime &rt)
	{
		_msecs = rt._msecs;
		return *this;
	}
	const zRTime & operator+ (const zRTime &rt)
	{
		_msecs += rt._msecs;
		return *this;
	}
	const zRTime & operator- (const zRTime &rt)
	{
		_msecs -= rt._msecs;
		return *this;
	}
	bool operator > (const zRTime &rt) const
	{
		return _msecs > rt._msecs;
	}
	bool operator >= (const zRTime &rt) const
	{
		return _msecs >= rt._msecs;
	}
	bool operator < (const zRTime &rt) const
	{
		return _msecs < rt._msecs;
	}
	bool operator <= (const zRTime &rt) const
	{
		return _msecs <= rt._msecs;
	}
	bool operator == (const zRTime &rt) const
	{
		return _msecs == rt._msecs;
	}

	unsigned long long elapse(const zRTime &rt) const
	{
		if (rt._msecs > _msecs)
			return (rt._msecs - _msecs);
		else
			return 0LL;
	}
	unsigned long long elapse() const
	{
		zRTime rt;
		if (rt._msecs > _msecs)
			return (rt._msecs - _msecs);
		else
			return 0LL;
	}
};
//------------------------------------------------------------------------
class zTime
{

public:
	zTime()
	{
		secs=GetTimeSec();
		getLocalTime(tv, secs);
	}
	zTime( unsigned int secs )
	{
		getLocalTime(tv, secs);
	}

	zTime(const zTime &ct)
	{
		secs = ct.secs;
		getLocalTime(tv, secs);
	}
	void now()
	{
		secs=GetTimeSec();
		getLocalTime(tv, secs);
	}
	time_t sec() const
	{
		return secs;
	}
	zTime & operator= (const zTime &rt)
	{
		secs = rt.secs;
		getLocalTime(tv, secs);
		return *this;
	}
	const zTime & operator+ (const zTime &rt)
	{
		secs += rt.secs;
		getLocalTime(tv, secs);
		return *this;
	}
	const zTime & operator- (const zTime &rt)
	{
		secs -= rt.secs;
		getLocalTime(tv, secs);
		return *this;
	}
	const zTime & operator-= (const time_t s)
	{
		secs -= s;
		getLocalTime(tv, secs);
		return *this;
	}

	const zTime & operator+= (const time_t s)
	{
		secs += s;
		getLocalTime(tv, secs);
		return *this;
	}

	bool operator > (const zTime &rt) const
	{
		return secs > rt.secs;
	}
	bool operator >= (const zTime &rt) const
	{
		return secs >= rt.secs;
	}
	bool operator < (const zTime &rt) const
	{
		return secs < rt.secs;
	}
	bool operator <= (const zTime &rt) const
	{
		return secs <= rt.secs;
	}
	bool operator == (const zTime &rt) const
	{
		return secs == rt.secs;
	}

	time_t elapse(const zTime &rt) const
	{
		if (rt.secs > secs)
			return (rt.secs - secs);
		else
			return 0;
	}

	time_t elapse() const
	{
		zTime rt;
		if (rt.secs > secs)
			return (rt.secs - secs);
		else
			return 0;
	}

	int getSec()
	{
		return tv.tm_sec;
	}


	int getMin()
	{
		return tv.tm_min;
	}

	int getHour()
	{
		return tv.tm_hour;
	}

	int getMDay()
	{
		return tv.tm_mday;
	}

	int getWDay()
	{
		return tv.tm_wday;
	}

	int GetYDay()
	{
		return tv.tm_yday;
	}

	int getMonth()
	{
		return tv.tm_mon+1;
	}

	int getYear()
	{
		return tv.tm_year+1900;
	}	

	int getnWeek()										//今天是一年内的第几周
	{
		int weeknow =tv.tm_wday;						//今天星期几
		int daydiff = (-1) * (weeknow+1);				//今日与上周末的天数差
		time_t lastweek = GetTimeSec() + daydiff*24*60*60;
		struct tm tv2;
		getLocalTime(tv2,lastweek);

		int days = tv2.tm_yday ;						//上周末是本年第几天
		int weeks = days/7;
		if(days%7 != 0)
		{
			weeks++;
		}												//此时，weeks为上周是本年的第几周
		return (weeks+1);
	}
	static std::string & getLocalTZ(std::string & s)
	{

		char sTimetz[256]={0};
		_tzset();

		sprintf_s(sTimetz,sizeof(sTimetz),"GMT%u\0",timezone/3600); 
		s=sTimetz; 
		return s;
	}
	static void getLocalTime(struct tm & tv1, time_t timValue)
	{
		timValue +=8*60*60;  
		tv1 = *gmtime(&timValue);
	}
private:

	time_t secs;
	struct tm tv;
};
//------------------------------------------------------------------------
class Timer
{
public:
	Timer(const float how_long , const int delay=0,const int nunits=1000) : _long((int)(how_long*nunits)), _timer(delay*nunits)
	{

	}
	Timer(const float how_long , const zRTime cur,const int nunits=1000) : _long((int)(how_long*nunits)), _timer(cur)
	{
		_timer.addDelay(_long);
	}
	void next(const zRTime &cur)
	{
		_timer=cur;
		_timer.addDelay(_long);
	} 


	bool operator() (const zRTime& current)
	{
		if (_timer <= current) 
		{
			_timer = current;
			_timer.addDelay(_long);
			return true;
		}
		return false;
	}
	bool istime() 
	{
		zRTime current;
		if (_timer <= current) 
		{
			_timer = current;
			_timer.addDelay(_long);
			return true;
		}
		return false;
	}

	unsigned long long msecs() const
	{
		return _timer.msecs();
	}
private:
	int _long;
	zRTime _timer;
};
//------------------------------------------------------------------------
class RandTimer
{
public:
	#define next_time(_long) (_long / 2 + _random(0, _long))

	RandTimer(const float how_long , const int delay=0,const int nunits=1000) : _long((int)(how_long*nunits)), _timer(delay*nunits)
	{
	}
	RandTimer(const float how_long , const zRTime cur,const int nunits=1000) : _long((int)(how_long*nunits)), _timer(cur)
	{
		_timer.addDelay(next_time(_long));
	}
	void next(const zRTime &cur)
	{
		_timer=cur;
		_timer.addDelay(next_time(_long));
	} 

	bool operator() (const zRTime& current)
	{
		if (_timer <= current)
		{
			_timer = current;
			_timer.addDelay(next_time(_long));
			return true;
		}
		return false;
	}
	bool istime() 
	{
		zRTime current;
		if (_timer <= current)
		{
			_timer = current;
			_timer.addDelay(next_time(_long));
			return true;
		}
		return false;
	}

	unsigned long long msecs() const
	{
		return _timer.msecs();
	}
private:
	int _long;
	zRTime _timer;
};
//------------------------------------------------------------------------