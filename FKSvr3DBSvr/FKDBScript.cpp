/**
*	created:		2013-4-9   15:37
*	filename: 		FKDBSvr
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKDBScript.h"
//------------------------------------------------------------------------
#if (LUA_VERSION_NUM >=501)
#else
	#ifndef LUA_VERSION_NUM
		#define LUA_VERSION_NUM		500	
	#endif
#endif
//------------------------------------------------------------------------
CScriptSystem::CScriptSystem()
{
	FUNCTION_BEGIN;
	m_LuaVM=NULL;
}
//------------------------------------------------------------------------
CScriptSystem::~CScriptSystem(){
	FUNCTION_BEGIN;
	if ( m_LuaVM && m_LuaVM->IsExistFunction("UnInstallScript") ){
		m_LuaVM->VCall("UnInstallScript");
	}
	SAFE_DELETE(m_LuaVM);
}
//------------------------------------------------------------------------
bool CScriptSystem::InitScript(char* pszScriptFileName,DWORD initstate)
{
	FUNCTION_BEGIN;
	if(initstate==eScript_uninit){
		if ( m_LuaVM && m_LuaVM->IsExistFunction("UnInstallScript") ){
			m_LuaVM->VCall("UnInstallScript",true);
		}
		SAFE_DELETE(m_LuaVM);
		return true;
	}else{
		if(m_LuaVM==NULL && initstate!=eScript_init){ initstate=eScript_init; }
		CLuaVM* tmpLuaVM=m_LuaVM;
		m_LuaVM=new CLuaVM;
		if (m_LuaVM){
			Bind(m_LuaVM);
			if ( !m_LuaVM->DoFile(pszScriptFileName) ){
				g_logger.error("lua ½Å±¾ %s ¼ÓÔØÊ§°Ü!",pszScriptFileName);
				SAFE_DELETE(m_LuaVM);
				m_LuaVM=tmpLuaVM;
				return false;
			}else{
				if ( tmpLuaVM && tmpLuaVM->IsExistFunction("UnInstallScript") ){
					tmpLuaVM->VCall("UnInstallScript",false);
				}
				if (m_LuaVM->IsExistFunction("InstallScript")){
					m_LuaVM->VCall("InstallScript",(initstate==eScript_init));
				}
			}
			SAFE_DELETE(tmpLuaVM);
		}else{
			g_logger.error("lua ½Å±¾ %s ¼ÓÔØÊ§°Ü!",pszScriptFileName);
			m_LuaVM=tmpLuaVM;
			return false;
		}
		return true;
	}
}
//------------------------------------------------------------------------
void scriptprint(const char* msg){
	if (!msg || msg[0]==0){return;}
	g_logger.forceLog(zLogger::zINFO,msg);
}
//------------------------------------------------------------------------
void CScriptSystem::Bind(CLuaVM* luavm)
{
	FUNCTION_BEGIN;
	using namespace luabind;
	lua_pushnumber(luavm->lua(), LUA_VERSION_NUM);
	lua_setglobal(luavm->lua(), "VERSION_NUM"); 
}
//------------------------------------------------------------------------
/*
PROPERTY_DEFINE(stSelectPlayerInfo,job,BYTE,o->siFeature.job);
PROPERTY_DEFINE(stSelectPlayerInfo,sex,BYTE,o->siFeature.sex);
PROPERTY_DEFINE_READONLY(stSelectPlayerInfo,szname,const char*,o->szName);

void CScriptSystem::Bind(CLuaVM* luavm)
{
	FUNCTION_BEGIN;
	using namespace luabind;

	module(luavm->lua())
		[
			class_<std::string>("String"),
			def("print",&scriptprint),		
			def("sectostr",&sec2str),

			class_<stSelectPlayerInfo>("newPlayerInfo")				
			.def_readwrite("mapid",&stSelectPlayerInfo::wmapid)			
			.def_readwrite("mapcountryid",&stSelectPlayerInfo::btmapcountryid)
			.def_readwrite("mapsublineid",&stSelectPlayerInfo::btmapsublineid)
			.def_readwrite("FamilyID",&stSelectPlayerInfo::familyID)
			.def_readwrite("level",&stSelectPlayerInfo::nlevel)
			.PROPERTY_RW("job",stSelectPlayerInfo,job)
			.PROPERTY_RW("sex",stSelectPlayerInfo,sex)

			.def_readonly("useronlyid",&stSelectPlayerInfo::dwUserOnlyId)
			.PROPERTY_READONLY("name",stSelectPlayerInfo,szname)	
		];

	lua_pushnumber(luavm->lua(), LUA_VERSION_NUM);
	lua_setglobal(luavm->lua(), "VERSION_NUM"); 
}
*/
//------------------------------------------------------------------------