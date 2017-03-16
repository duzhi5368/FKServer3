/**
*	created:		2013-4-8   13:17
*	filename: 		FKCfgParser
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <sstream>
#include <algorithm>
#include "FKBaseDefine.h"
#include "FKStringEx.h"
#include <hash_map>
#include <malloc.h>
#include "FKXmlParser.h"
//------------------------------------------------------------------------
#define	_LOCK_NAME_			"CConfigParse4589FDJJLAPWE903"
//------------------------------------------------------------------------
class CConfigParse
{
protected:
	CInpLock	m_lock;
protected:
	virtual bool read(const char* name,__int64& value,__int64 i64default,bool onlylocal)=0;
	virtual bool read(const char* name,char* szbuffer,int nsize,const char* szdefault,bool onlylocal)=0;

	virtual bool seek(void* global,void* local) = 0;
public:
	bool readstr(const char* name,char* szbuffer,int nsize,const char* szdefault,bool onlylocal=false){
		return read(name,szbuffer,nsize,szdefault,onlylocal);
	}
	template<class _Ty>	bool readvalue(const char* name,_Ty& _value,_Ty _default,bool onlylocal=false){
		__int64 i64value=0;
		bool boret=read(name,i64value,_default,onlylocal);
		_value=(_Ty)i64value;
		return boret;
	};
	template<class _Ty> _Ty readvalue(const char* name,_Ty _default,bool onlylocal=false){
		__int64 i64value=0;
		read(name,i64value,_default,onlylocal);
		return ((_Ty)i64value);
	};

	CConfigParse(){
		m_lock.open(_LOCK_NAME_);
		INFOLOCK(m_lock);
	}
	virtual ~CConfigParse(){
		UNINFOLOCK(m_lock);
		m_lock.close();
	}
};
//------------------------------------------------------------------------
class CXMLConfigParse:public CConfigParse
{
protected:
	zXMLParser m_parse;
	xmlNodePtr m_local;
	xmlNodePtr m_global;
	char m_szfilename[MAX_PATH];
	bool m_bodump;

	virtual bool read(const char* name,__int64& value,__int64 i64default,bool onlylocal){
		if (m_parse.getChildNodeNum(m_local,name,value)){
			return true;
		}else if(!onlylocal && m_parse.getChildNodeNum(m_global,name,value)){
			return true;
		}
		value=i64default;
		if (!onlylocal){ m_parse.newChildNode_Num(m_global,name,value);	}
		else{ m_parse.newChildNode_Num(m_local,name,value);	}
		m_bodump=true;
		return false;
	}
	virtual bool read(const char* name,char* szbuffer,int nsize,const char* szdefault,bool onlylocal){
		if (!szbuffer || !name){ return false; }
		if (m_parse.getChildNodeStr(m_local,name,szbuffer,nsize)){
			return true;
		}else if(!onlylocal && m_parse.getChildNodeStr(m_global,name,szbuffer,nsize)){
			return true;
		}else if(szdefault){
			strcpy_s(szbuffer,nsize,szdefault);
		}
		if (!onlylocal){ m_parse.newChildNode(m_global,name,szbuffer);	}
		else{ m_parse.newChildNode(m_local,name,szbuffer);	}
		m_bodump=true;
		return false;
	}
	virtual bool seek(void* global,void* local){
		m_local=(xmlNodePtr)local;
		m_global=(xmlNodePtr)global;
		return true;
	}
public:
	CXMLConfigParse(){
		m_bodump=false;m_szfilename[0]=0;
	}
	virtual ~CXMLConfigParse(){
		if (m_bodump && m_szfilename[0]!=0){ m_parse.dump(m_szfilename);m_bodump=false; }
	}
	bool InitConfig(const char* localname=NULL,const char* filename="./config.xml",const char* globalname="global"){
		char szlocalname[MAX_PATH]={0};
		if (localname==NULL){
			GetModuleFileName(NULL,szlocalname,sizeof(szlocalname)-1);
			strcpy_s(szlocalname,sizeof(szlocalname)-1,extractfiletitle(szlocalname));
			strlwr(szlocalname);
		}else{
			strcpy_s(szlocalname,sizeof(szlocalname)-1,localname);
			strlwr(szlocalname);
		}
		localname=szlocalname;
		strcpy_s(m_szfilename,sizeof(m_szfilename)-1,filename);
		if (!m_parse.initFile(filename)){
			m_parse.init();
			m_bodump=true;
		}

		xmlNodePtr cfgroot=m_parse.getRootNode("config");
		if (!cfgroot){	
			cfgroot=m_parse.newRootNode("config");
			if (cfgroot){
				m_parse.newNodeProp(cfgroot,"parse","tinyxml");
			}
		}
		xmlNodePtr global=NULL;
		xmlNodePtr local=NULL;
		if (cfgroot){
			global=m_parse.getChildNode(cfgroot,globalname);
			local=m_parse.getChildNode(cfgroot,localname);
		}
		if (!global){
			global=m_parse.newChildNode(cfgroot,globalname,NULL);
			m_bodump=true;
		}
		if (!local){
			local=m_parse.newChildNode(cfgroot,localname,NULL);
			m_bodump=true;
		}
		if (global!=NULL && local!=NULL){
			if (seek(global,local)){
				return true;
			}
		}
		m_bodump=false;
		return false;
	}
	zXMLParser& getparse(){ return m_parse; }
};
//------------------------------------------------------------------------