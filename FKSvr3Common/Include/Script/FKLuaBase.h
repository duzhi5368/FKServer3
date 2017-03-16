/**
*	created:		2013-4-9   15:26
*	filename: 		FKLuaBase
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#pragma warning(disable : 4127)			
#pragma warning(disable : 4100)			
//------------------------------------------------------------------------
extern "C"
{
#include "../Lua/lua.h"
#include "../Lua/lauxlib.h"
#include "../Lua/lualib.h"
}
//------------------------------------------------------------------------
#include "../luabind/luabind.hpp"
#include "../luabind/class.hpp"
#include "../luabind/function.hpp"
#include "../luabind/object.hpp"
#include "../luabind/operator.hpp"
#include "../FKBaseDefine.h"
#include "../FKLogger.h"
#include "../STLTemplate/FKSyncList.h"
#include <malloc.h> 
//------------------------------------------------------------------------
#define LUABIND_TRY_ERRORMSG(s,L)		const char* s=NULL; try{	__noop;

#define LUA_GET_ERRORMSG(s,L)			s=(const char*)lua_tostring(L, -1);if(s==NULL){s="";};lua_pop(L, 1);
#define LUABIND_LOG_CALL_ERROR(cs,fns,es,s)		g_logger.error( "(%s)Luabind::callfunc: %s () -> err: %s ( %s )",cs,fns,es,s);

#define LUABIND_CATCH_ERRORMSG(s,L,fns)		}catch (luabind::cast_failed& e){ LUA_GET_ERRORMSG(s,L);LUABIND_LOG_CALL_ERROR("cast_failed",fns,e.what(),s) }	\
	catch (luabind::error& e){ LUA_GET_ERRORMSG(s,L);LUABIND_LOG_CALL_ERROR("error",fns,e.what(),s) }		\
	catch (const char* e){ LUA_GET_ERRORMSG(s,L);LUABIND_LOG_CALL_ERROR("char*",fns,e,s) }		\
	catch (...){ LUA_GET_ERRORMSG(s,L);LUABIND_LOG_CALL_ERROR("...",fns,"",s) }		\

#define _BUILDLUASTR_(callstr,maxparam,szfuncname,pchar_param,param_count)		CEasyStrParse parse;int nsrclen=strlen(callstr)+32;		\
	STACK_ALLOCA( char*,tmpstr,nsrclen );strcpy_s(tmpstr,nsrclen-1 ,callstr);parse.SetParseStr(tmpstr,"(), \x9","\"\"\'\'",'"');		\
	int nparam_parse_count=parse.ParamCount();param_count=0;		\
	if (nparam_parse_count<=((maxparam)+1) && nparam_parse_count>=1){		\
	strcpy_s(&szfuncname[0],sizeof(szfuncname)-1,parse[0]);		\
	if (szfuncname[0]!=0){		\
	int j=0;char* pstr=NULL;int nslen=0;		\
	for(int i=1;i<nparam_parse_count;i++){		\
	pstr=parse[i];j=i-1;		\
	if(pstr[0]=='"'){		\
	nslen=strlen(pstr);		\
	if(nslen<2 || pstr[nslen-1]!='"'){ param_count=-1;break; }		\
	pstr[nslen-1]=0;pstr++;	pchar_param[j]=pstr;param_count++;		\
	}else if(pstr[0]=='\''){		\
	nslen=strlen(pstr);		\
	if(nslen<2 || pstr[nslen-1]!='\''){param_count=-1;break; }		\
	pstr[nslen-1]=0;pstr++;pchar_param[j]=pstr;param_count++;		\
	}else{pchar_param[j]=pstr;param_count++;}}}else{ param_count=-1; }; }else{ param_count=-1; };		\

#define _VCALL_BUILDLUASTR_(funcname,paramcount,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)		if(IsExistFunction(funcname)){		\
	LUABIND_TRY_ERRORMSG(errmsg,m_luaState); switch (paramcount){		\
	case	0:{ luabind::call_function< void >(lua(),funcname); return true; }break;	\
	case	1:{ luabind::call_function< void >(lua(),funcname,p1); return true; }break;	\
	case	2:{ luabind::call_function< void >(lua(),funcname,p1,p2); return true; }break;	\
	case	3:{ luabind::call_function< void >(lua(),funcname,p1,p2,p3); return true; }break;	\
	case	4:{ luabind::call_function< void >(lua(),funcname,p1,p2,p3,p4); return true; }break;	\
	case	5:{ luabind::call_function< void >(lua(),funcname,p1,p2,p3,p4,p5); return true; }break;	\
	case	6:{ luabind::call_function< void >(lua(),funcname,p1,p2,p3,p4,p5,p6); return true; }break;	\
	case	7:{ luabind::call_function< void >(lua(),funcname,p1,p2,p3,p4,p5,p6,p7); return true; }break;	\
	case	8:{ luabind::call_function< void >(lua(),funcname,p1,p2,p3,p4,p5,p6,p7,p8); return true; }break;	\
	case	9:{ luabind::call_function< void >(lua(),funcname,p1,p2,p3,p4,p5,p6,p7,p8,p9); return true; }break;	\
	case	10:{ luabind::call_function< void >(lua(),funcname,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10); return true; }break;};	\
	LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname); };	\

#define _CALL_BUILDLUASTR_( R ,funcname,paramcount,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10)		if(IsExistFunction(funcname)){	\
	LUABIND_TRY_ERRORMSG(errmsg,m_luaState); switch (paramcount){		\
	case	0:{ return luabind::call_function< R >(lua(),funcname);}break;	\
	case	1:{ return luabind::call_function< R >(lua(),funcname,p1);}break;	\
	case	2:{ return luabind::call_function< R >(lua(),funcname,p1,p2);}break;	\
	case	3:{ return luabind::call_function< R >(lua(),funcname,p1,p2,p3);}break;	\
	case	4:{ return luabind::call_function< R >(lua(),funcname,p1,p2,p3,p4);}break;	\
	case	5:{ return luabind::call_function< R >(lua(),funcname,p1,p2,p3,p4,p5);}break;	\
	case	6:{ return luabind::call_function< R >(lua(),funcname,p1,p2,p3,p4,p5,p6);}break;	\
	case	7:{ return luabind::call_function< R >(lua(),funcname,p1,p2,p3,p4,p5,p6,p7);}break;	\
	case	8:{ return luabind::call_function< R >(lua(),funcname,p1,p2,p3,p4,p5,p6,p7,p8);}break;	\
	case	9:{ return luabind::call_function< R >(lua(),funcname,p1,p2,p3,p4,p5,p6,p7,p8,p9);}break;	\
	case	10:{ return luabind::call_function< R >(lua(),funcname,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);}break;};		\
	LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname); };	\

//------------------------------------------------------------------------
class CLuaVM{
protected:
	lua_State *m_luaState;
	int m_ErrFn;
	int m_nParseStatus;
private:
	static DWORD dwThreadsId;
public:
	CLuaVM(bool bOpenStdLib = true);
	virtual ~CLuaVM();

	void OpenStdLib();
	void closelua();
	__inline lua_State* GetHandle(){	return m_luaState;}
	__inline lua_State* lua() const { return m_luaState; }
	__inline int ErrorFn(){	return m_ErrFn; }

	bool Do();

	virtual bool LoadFileToBuffer(lua_State *L, const char *filename,char* szbuffer,int &maxlen,bool& loadlocal);

	bool DoFile(const char* filename);
	bool DoString(const char* buffer);
	bool DoBuffer(const char* buffer, size_t size);

	bool IsExistFunction(const char* name){
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		lua_getglobal(m_luaState, name);
		if(lua_type(m_luaState,-1) == LUA_TFUNCTION){
			lua_pop(m_luaState,1);
			return true;
		}else{
			lua_pop(m_luaState,1);
			return false;
		}
	}

	template<typename R>
	R Call(const char* funcname,R defVal)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1>
	R Call(const char* funcname,R defVal, P1 p1)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2, typename P3>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2, P3 p3)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2,p3);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2,p3,p4);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2,p3,p4,p5);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2,p3,p4,p5,p6);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2,p3,p4,p5,p6,p7);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2,p3,p4,p5,p6,p7,p8);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2,p3,p4,p5,p6,p7,p8,p9);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}
	template<typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
	R Call(const char* funcname,R defVal, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			return luabind::call_function<R>(m_luaState,funcname,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return defVal;
	}


	bool VCall(const char* funcname){
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}

	template<typename P1>
	bool VCall(const char* funcname,P1 p1)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2>
	bool VCall(const char* funcname, P1 p1, P2 p2)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2, typename P3>
	bool VCall(const char* funcname, P1 p1, P2 p2, P3 p3)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2,p3);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2, typename P3, typename P4>
	bool VCall(const char* funcname, P1 p1, P2 p2, P3 p3, P4 p4)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2,p3,p4);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2, typename P3, typename P4, typename P5>
	bool VCall(const char* funcname, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2,p3,p4,p5);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	bool VCall(const char* funcname, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2,p3,p4,p5,p6);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	bool VCall(const char* funcname, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2,p3,p4,p5,p6,p7);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	bool VCall(const char* funcname, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2,p3,p4,p5,p6,p7,p8);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
	bool VCall(const char* funcname, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2,p3,p4,p5,p6,p7,p8,p9);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}
	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
	bool VCall(const char* funcname, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10)
	{
		if(IsExistFunction(funcname)){
			LUABIND_TRY_ERRORMSG(errmsg,m_luaState);
			luabind::call_function<void>(m_luaState,funcname,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10);
			return true;
			LUABIND_CATCH_ERRORMSG(errmsg,m_luaState,funcname);
		}
		return false;
	}

	template<typename R>
	R Call_LuaStr(const char* callstr,R defVal)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=0;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_CALL_BUILDLUASTR_( R ,szfuncname,(param_count+tpc),pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5],pchar_param[6],pchar_param[7],pchar_param[8],pchar_param[9]);
		}else{
			g_logger.error("Call_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return defVal;
	}
	template<typename R,typename P1>
	R Call_LuaStr(const char* callstr,R defVal,P1 p1)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=1;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_CALL_BUILDLUASTR_( R ,szfuncname,(param_count+tpc),p1,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5],pchar_param[6],pchar_param[7],pchar_param[8]);
		}else{
			g_logger.error("Call_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return defVal;
	}
	template<typename R,typename P1,typename P2>
	R Call_LuaStr(const char* callstr,R defVal,P1 p1,P2 p2)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=2;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_CALL_BUILDLUASTR_( R ,szfuncname,(param_count+tpc),p1,p2,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5],pchar_param[6],pchar_param[7]);			
		}else{
			g_logger.error("Call_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return defVal;
	}
	template<typename R,typename P1,typename P2,typename P3>
	R Call_LuaStr(const char* callstr,R defVal,P1 p1,P2 p2,P3 p3)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=3;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_CALL_BUILDLUASTR_( R ,szfuncname,(param_count+tpc),p1,p2,p3,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5],pchar_param[6]);
		}else{
			g_logger.error("Call_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return defVal;
	}
	template<typename R,typename P1,typename P2,typename P3,typename P4>
	R Call_LuaStr(const char* callstr,R defVal,P1 p1,P2 p2,P3 p3,P4 p4)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=4;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_CALL_BUILDLUASTR_( R ,szfuncname,(param_count+tpc),p1,p2,p3,p4,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5]);
		}else{
			g_logger.error("Call_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return defVal;
	}
	template<typename R,typename P1,typename P2,typename P3,typename P4,typename P5>
	R Call_LuaStr(const char* callstr,R defVal,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=5;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_CALL_BUILDLUASTR_( R ,szfuncname,(param_count+tpc),p1,p2,p3,p4,p5,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4]);
		}else{
			g_logger.error("Call_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return defVal;
	}

	bool VCall_LuaStr(const char* callstr);

	template<typename P1>
	bool VCall_LuaStr(const char* callstr,P1 p1)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=1;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_VCALL_BUILDLUASTR_(szfuncname,(param_count+tpc),p1,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5],pchar_param[6],pchar_param[7],pchar_param[8]);
		}else{
			g_logger.error("VCall_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return false;
	}
	template<typename P1,typename P2>
	bool VCall_LuaStr(const char* callstr,P1 p1,P2 p2)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=2;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_VCALL_BUILDLUASTR_(szfuncname,(param_count+tpc),p1,p2,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5],pchar_param[6],pchar_param[7]);
		}else{
			g_logger.error("VCall_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return false;
	}
	template<typename P1,typename P2,typename P3>
	bool VCall_LuaStr(const char* callstr,P1 p1,P2 p2,P3 p3)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=3;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_VCALL_BUILDLUASTR_(szfuncname,(param_count+tpc),p1,p2,p3,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5],pchar_param[6]);
		}else{
			g_logger.error("VCall_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return false;
	}
	template<typename P1,typename P2,typename P3,typename P4>
	bool VCall_LuaStr(const char* callstr,P1 p1,P2 p2,P3 p3,P4 p4)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=4;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_VCALL_BUILDLUASTR_(szfuncname,(param_count+tpc),p1,p2,p3,p4,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4],
				pchar_param[5]);
		}else{
			g_logger.error("VCall_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return false;
	}
	template<typename P1,typename P2,typename P3,typename P4,typename P5>
	bool VCall_LuaStr(const char* callstr,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
	{
		if (dwThreadsId!=GetCurrentThreadId()){
			g_logger.forceLog( zLogger::zFATAL, vformat("线程不同 原线程ID[%d] 当前线程ID[%d]",dwThreadsId,GetCurrentThreadId()));
			dwThreadsId=GetCurrentThreadId();
		}

		char* pchar_param[10];
		char szfuncname[256]={0};
		int param_count=0;
		const int tpc=5;

		_BUILDLUASTR_(callstr,count_of(pchar_param)-tpc,szfuncname,pchar_param,param_count);
		if (param_count>=0){
			_VCALL_BUILDLUASTR_(szfuncname,(param_count+tpc),p1,p2,p3,p4,p5,pchar_param[0],pchar_param[1],pchar_param[2],pchar_param[3],pchar_param[4]);
		}else{
			g_logger.error("VCall_LuaStr(%s) 参数字符串解析失败!",callstr);
		}
		return false;
	}
};
//------------------------------------------------------------------------
#define PROPERTY_DEFINE_READONLY(classname,varname,vartype,retcode )	struct classname##_##varname##_property{		\
	static vartype classname##_##varname##_property_read(const classname * o){ return retcode; };}; 	\

#define PROPERTY_DEFINE(classname,varname,vartype,retcode )	struct classname##_##varname##_property{		\
	static vartype classname##_##varname##_property_read(const classname * o){ return retcode; };	\
	static void classname##_##varname##_property_write(classname * o,vartype v){ retcode=v; };	};	\

#define STATIC_PROPERTY_DEFINE_READONLY(classname,varname,vartype)	PROPERTY_DEFINE_READONLY(classname,varname,vartype,classname##::##varname);
#define STATIC_PROPERTY_DEFINE(classname,varname,vartype)	PROPERTY_DEFINE(classname,varname,vartype,classname##::##varname); 

#define PROPERTY_READONLY(propertyname,classname,varname)	property( propertyname ,&##classname##_##varname##_property::##classname##_##varname##_property_read)
#define PROPERTY_RW(propertyname,classname,varname)	property( propertyname ,&##classname##_##varname##_property::##classname##_##varname##_property_read , &##classname##_##varname##_property::##classname##_##varname##_property_write)

#define CMP_REF_PTR_DEFINE(T)		bool operator==(const T& se1,const T* se2){ return ((&se1)==se2); }
#define CMP_REF_REF_DEFINE(T)		bool operator==(const T& se1,const T& se2){ return ((&se1)==(&se2)); }
//------------------------------------------------------------------------