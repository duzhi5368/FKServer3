/**
*	created:		2013-4-7   0:12
*	filename: 		FKClass
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <stddef.h>
#include <stdio.h>
#include "FKType.h"
#include "FKTypeDecl.h"
//------------------------------------------------------------------------
enum RTTIClassFlags { 
	RTTI_CLS_ABSTRACT  = 0x0001,
	RTTI_CLS_INTERNAL  = 0x0002,
	RTTI_CLS_TRANSIENT = 0x0004
};

enum eSortFieldType{
	eSortNone=0,
	eSortByOffSet=1,
	eSortByName=2,
};
//------------------------------------------------------------------------
class RTTIClassDescriptor : public RTTIType 
{ 
public:
	typedef RTTIFieldDescriptor* (*RTTIDescribeFieldsFunc)();
	typedef RTTIMethodDescriptor* (*RTTIDescribeMethodsFunc)();

	char* getTypeName(char* buf);

	RTTIClassDescriptor* getNextClass() 
	{ 
		RTTIClassDescriptor* retnext=next;
		while (retnext)
		{
			if (retnext->isClass())
			{
				return retnext;
			}
			retnext=retnext->next;
		}
		return retnext;
	}

	RTTIClassDescriptor* getNext() { 
		return next;
	}
	RTTIFieldDescriptor** getFields() { 
		return fields;
	}
	int getNumberOfFields() { 
		return nFields;
	}
	char const* getName() {
		return name;
	}
	char const* getAliasName() {
		return aliasname;
	}
	int  getSize() { 
		return size;
	}
	int  getFlags() { 
		return flags;
	}

	template<class __RT>
	bool newInstance(__RT*& retobj)
	{
		RTTIMethodDescriptor* pm=findMethodfunc< stInvokeCallHelper<int,__RT>::new0 >("new0",0,RTTI_MTH_CONSTRUCTOR);
		if (pm){
			bool boret=false;
			if (pm->invoke(boret,(stInvokeCallHelper<int,__RT>*)NULL,(void**)&retobj)){
				__RT* tmpobj=NULL;
				tmpobj=(__RT*)ConvertClassPtr((void*)retobj,this,__RTTITypeOfPtr((__RT*)NULL));
				if (tmpobj==NULL){ tmpobj=retobj; }
				retobj=tmpobj;
				return boret;
			}
		}else{
			char szbuff[512];
			g_logger.debug("找不到与函数原型 %s 匹配的构造函数!",RTTIFuncTypeGetDis((stInvokeCallHelper<int,__RT>::new0)NULL,szbuff,this,"new0" ) );
		}
		return false;
	}

	template<class __RT,class __P1>
	bool newInstance(__RT*& retobj,__P1 p1)
	{
		RTTIMethodDescriptor* pm=findMethodfunc< stInvokeCallHelper<int,__RT,__P1>::new1 >("new1",0,RTTI_MTH_CONSTRUCTOR);
		if (pm){
			bool boret=false;
			if (pm->invoke(boret,(stInvokeCallHelper<int,__RT,__P1>*)NULL,(void**)&retobj,p1)){
				__RT* tmpobj=NULL;
				tmpobj=(__RT*)ConvertClassPtr((void*)retobj,this,__RTTITypeOfPtr((__RT*)NULL));
				if (tmpobj==NULL){ tmpobj=retobj; }
				retobj=tmpobj;
				return boret;
			}
		}else{
			char szbuff[512];
			g_logger.debug("找不到与函数原型 %s 匹配的构造函数!",RTTIFuncTypeGetDis((stInvokeCallHelper<int,__RT,__P1>::new1)NULL,szbuff,this,"new1" ) );
		}
		return false;
	}

	template<class __RT,class __P1,class __P2>
	bool newInstance(__RT*& retobj,__P1 p1,__P2 p2)
	{
		RTTIMethodDescriptor* pm=findMethodfunc< stInvokeCallHelper<int,__RT,__P1,__P2>::new2 >("new2",0,RTTI_MTH_CONSTRUCTOR);
		if (pm){
			bool boret=false;
			if (pm->invoke(boret,(stInvokeCallHelper<int,__RT,__P1,__P2>*)NULL,(void**)&retobj,p1,p2)){
				__RT* tmpobj=NULL;
				tmpobj=(__RT*)ConvertClassPtr((void*)retobj,this,__RTTITypeOfPtr((__RT*)NULL));
				if (tmpobj==NULL){ tmpobj=retobj; }
				retobj=tmpobj;
				return boret;
			}
		}else{
			char szbuff[512];
			g_logger.debug("找不到与函数原型 %s 匹配的构造函数!",RTTIFuncTypeGetDis((stInvokeCallHelper<int,__RT,__P1,__P2>::new2)NULL,szbuff,this,"new2" ) );
		}
		return false;
	}

	template<class __RT,class __P1,class __P2,class __P3>
	bool newInstance(__RT*& retobj,__P1 p1,__P2 p2,__P3 p3)
	{
		RTTIMethodDescriptor* pm=findMethodfunc< stInvokeCallHelper<int,__RT,__P1,__P2,__P3>::new3 >("new3",0,RTTI_MTH_CONSTRUCTOR);
		if (pm){
			bool boret=false;
			if (pm->invoke(boret,(stInvokeCallHelper<int,__RT,__P1,__P2,__P3>*)NULL,(void**)&retobj,p1,p2,p3)){
				__RT* tmpobj=NULL;
				tmpobj=(__RT*)ConvertClassPtr((void*)retobj,this,__RTTITypeOfPtr((__RT*)NULL));
				if (tmpobj==NULL){ tmpobj=retobj; }
				retobj=tmpobj;
				return boret;
			}
		}else{
			char szbuff[512];
			g_logger.debug("找不到与函数原型 %s 匹配的构造函数!",RTTIFuncTypeGetDis((stInvokeCallHelper<int,__RT,__P1,__P2,__P3>::new3)NULL,szbuff,this,"new3" ) );
		}
		return false;
	}

	template<class __RT,class __P1,class __P2,class __P3,class __P4>
	bool newInstance(__RT*& retobj,__P1 p1,__P2 p2,__P3 p3,__P4 p4)
	{
		RTTIMethodDescriptor* pm=findMethodfunc< stInvokeCallHelper<int,__RT,__P1,__P2,__P3,__P4>::new4 >("new4",0,RTTI_MTH_CONSTRUCTOR);
		if (pm){
			bool boret=false;
			if (pm->invoke(boret,(stInvokeCallHelper<int,__RT,__P1,__P2,__P3,__P4>*)NULL,(void**)&retobj,p1,p2,p3,p4)){
				__RT* tmpobj=NULL;
				tmpobj=(__RT*)ConvertClassPtr((void*)retobj,this,__RTTITypeOfPtr((__RT*)NULL));
				if (tmpobj==NULL){ tmpobj=retobj; }
				retobj=tmpobj;
				return boret;
			}
		}else{
			char szbuff[512];
			g_logger.debug("找不到与函数原型 %s 匹配的构造函数!",RTTIFuncTypeGetDis((stInvokeCallHelper<int,__RT,__P1,__P2,__P3,__P4>::new4)NULL,szbuff,this,"new4" ) );
		}
		return false;
	}

	RTTIClassDescriptor** getBaseClasses() { 
		return baseClasses;
	}
	int getNumberOfBaseClasses() { 
		return nBaseClasses;
	}

	void* ConvertClassPtr(void* psrc,RTTIClassDescriptor* src_cls,RTTIClassDescriptor* dst_cls);
	void* ConvertClassPtr(void* p,int ibase);
	bool IsKindOf(RTTIClassDescriptor* kindclass,int* offset=NULL);

	RTTIMethodDescriptor** getMethods() { 
		return methods;
	}
	int getNumberOfMethods() { 
		return nMethods;
	}

	RTTIFieldDescriptor*  findField(char const* name,int nbegin=0,int flag=0);
	RTTIFieldDescriptor*  findFieldByAliasName(char const* name,int nbegin=0,int flag=0,bool bocasestr=false);
	RTTIMethodDescriptor* findMethod(char const* name,int nbegin=0,int flag=0);
	RTTIMethodDescriptor* findMethodByAliasName(char const* name,int nbegin=0,int flag=0,bool bocasestr=false);

	template<class __func_type>
	RTTIMethodDescriptor* findMethodfunc(char const* name,int nbegin=0,int flag=0){
		__func_type tmpfunc=(__func_type)NULL;
		RTTIMethodDescriptor* pm=findMethod(name,nbegin,flag);
		while(pm!=NULL){
			if ( RTTIFuncTypeCheckIdx(tmpfunc)==pm->getType()->get_check_idx_r() ){
				return pm;
			}
			pm=findMethod(name,pm->index+1,flag);
		}
		return NULL;
	};

	template<class __proc_type>
	RTTIMethodDescriptor* findMethodproc(char const* name,int nbegin=0,int flag=0){
		__proc_type tmpproc=(__proc_type)NULL;
		RTTIMethodDescriptor* pm=findMethod(name,nbegin,flag);
		while(pm!=NULL){
			if ( RTTIProcTypeCheckIdx(tmpproc)==pm->getType()->get_check_idx() ){
				return pm;
			}
			pm=findMethod(name,pm->index+1,flag);
		}
		return NULL;
	};

	RTTIClassDescriptor(char const* name, int size, 
		RTTIDescribeFieldsFunc  describeFieldsFunc,
		RTTIDescribeMethodsFunc describeMethodsFunc, 
		int flags,int sortfieldtype=eSortNone,char const* aliasname=NULL,
		unsigned short int userdefine=RTTI_STRUCT); 
	RTTIClassDescriptor(char const* name, int size, int flags,int sortfieldtype=eSortNone,
		char const* aliasname=NULL,unsigned short int userdefine=RTTI_STRUCT);
	~RTTIClassDescriptor();

protected:
	friend class RTTIRepository;
	friend class RTTIBfdRepository;

	RTTIClassDescriptor*    next;
	RTTIClassDescriptor*    collisionChain;
	RTTIClassDescriptor*    collisionAliasChain;

	RTTIMethodDescriptor*   methodList;
	RTTIMethodDescriptor**  methods;
	int                     nMethods;

	RTTIFieldDescriptor*    fieldList;
	RTTIFieldDescriptor**   fields;
	int                     nFields;

	int                     flags;
	int                     size;

	bool                    initialized;

	char const*             name;
	char const*				aliasname;
	unsigned                hashCode;
	unsigned                hashAliasCode;

	int                     nBaseClasses;    
	RTTIClassDescriptor**   baseClasses;
	RTTIFieldDescriptor**	baseClassesFields;

	void buildClassDescriptor(int sortfieldtype);
};
//------------------------------------------------------------------------