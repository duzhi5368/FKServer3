/**
*	created:		2013-4-7   0:06
*	filename: 		FKReflect
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#ifdef USE_RTTI
#include <typeinfo>
#endif
//------------------------------------------------------------------------
#pragma warning (disable:4100)
//------------------------------------------------------------------------
#include "FKType.h"
#include "FKClass.h"
#include "FKField.h"
#include "FKMethod.h"
#include "FKTypeDecl.h"
#include "../FKBaseDefine.h"
#include "../FKSyncObjLock.h"
#include "../FKXmlParser.h"
//------------------------------------------------------------------------
const int RTTI_CLASS_HASH_SIZE = 1013;
//------------------------------------------------------------------------
class RTTIRepository:public CIntLock
{ 
public:
	RTTIClassDescriptor* getFirstClass() { 
		return classes;
	}

	RTTIClassDescriptor*               findClass(char const* pclassname,bool bocasestr=false);
	RTTIClassDescriptor*               findClassByAliasName(char const* pAliasName,bool bocasestr=false);
#ifdef USE_RTTI
	RTTIClassDescriptor*               findClass(class type_info const& tinfo,bool bocasestr=false) { 
			return findClass(tinfo.getName(),bocasestr);
	}
#endif

	static RTTIRepository* getInstance(){ 
		if (theRepository==NULL){
			theRepository=new RTTIRepository;
		}
		return theRepository;
	}

	static RTTIRepository* instance_readonly(){
		return theRepository;
	}

	static void delInstance(){
		if (theRepository){
			delete theRepository;
			theRepository=NULL;
		}
	}

	bool addClass(RTTIClassDescriptor* cls); 

	virtual bool load(char const* filePath);

	RTTIRepository():CIntLock()
	{
		ZeroMemory(hashTable,sizeof(hashTable));
		ZeroMemory(hashAliasTable,sizeof(hashAliasTable));
		classes=NULL;
	}

protected:
	static RTTIRepository* theRepository;
	RTTIClassDescriptor*  classes;
	RTTIClassDescriptor*  hashTable[RTTI_CLASS_HASH_SIZE];   
	RTTIClassDescriptor*  hashAliasTable[RTTI_CLASS_HASH_SIZE];   
};
//------------------------------------------------------------------------
typedef		RTTIRepository		RttiManage;
typedef		RTTIRepository		RttiM;
//------------------------------------------------------------------------
#define RTTI_FIELDNAME(pf,buf,objname)					if (objname==NULL || objname[0]==0){	\
	strcpy_s(buf,sizeof(buf)-1,pf->getAliasName());		}		\
else{ sprintf_s(buf,sizeof(buf)-1,"%s.%s",objname,pf->getAliasName()); }
//------------------------------------------------------------------------
typedef bool (WINAPI* fnfieldfilter)(void* p,RTTIClassDescriptor* pclass,TiXmlElement* node,RTTIFieldDescriptor* pfield,const char* objname);
bool SaveClassToXml(void* p,RTTIClassDescriptor* pclass,TiXmlElement* node,fnfieldfilter filter=NULL,const char* objname=NULL);
bool InitClassFromXml(void* p,RTTIClassDescriptor* pclass,TiXmlElement* node,fnfieldfilter filter=NULL,const char* objname=NULL);
//------------------------------------------------------------------------