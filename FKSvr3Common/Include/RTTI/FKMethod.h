/**
*	created:		2013-4-7   0:20
*	filename: 		FKMethod
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKTypeDecl.h"
//------------------------------------------------------------------------
enum RTTIMethodFlags { 
	RTTI_MTH_INSTANCE  = RTTI_FLD_INSTANCE, 
	RTTI_MTH_STATIC    = RTTI_FLD_STATIC, 
	RTTI_MTH_CONST     = RTTI_FLD_CONST, 
	RTTI_MTH_PUBLIC    = RTTI_FLD_PUBLIC, 
	RTTI_MTH_PROTECTED = RTTI_FLD_PROTECTED, 
	RTTI_MTH_PRIVATE   = RTTI_FLD_PRIVATE, 
	RTTI_MTH_VIRTUAL   = RTTI_FLD_VIRTUAL, 
	RTTI_MTH_CONSTRUCTOR = 0x0200, 
	RTTI_MTH_ABSTRACT    = 0x0400,
	RTTI_MTH_OVERLOADED  = 0x0800,
}; 
//------------------------------------------------------------------------
class RTTIMethodDescriptor 
{ 
public:
	char const* getName() { 
		return name;
	}
	char const* getAliasName() { 
		return aliasname;
	}
	RTTIMethodType* getType() { 
		return type;
	}
	int getFlags() { 
		return flags;
	}
	char* getMethodDeclaration(char* buf) { 
		return type->getMethodDeclaration(buf, aliasname);
	}


	template<class __C>
	bool vinvoke(__C* obj)
	{ 
		if (type->returnType->getTag()!=RTTIType::RTTI_VOID || ((int)RTTIProcType0<__C>::s_get_check_idx())!=type->get_check_idx()){
			char szbuff[512];char szbuff1[512];
			RTTIProcType0<__C> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeProc0-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		((RTTIProcType0<__C>*)(type))->invoke(obj);
		return true;
	};

	template<class __C,class __P1>
	bool vinvoke(__C* obj,__P1 p1)
	{ 
		if (type->returnType->getTag()!=RTTIType::RTTI_VOID || ((int)RTTIProcType1<__C,__P1>::m_class_check_idx)!=type->get_check_idx()){
			char szbuff[512];char szbuff1[512];
			RTTIProcType1<__C,__P1> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeProc1-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		((RTTIProcType1<__C,__P1>*)(type))->invoke(obj,p1);
		return true;
	};

	template<class __C,class __P1,class __P2>
	bool vinvoke(__C* obj,__P1 p1,__P2 p2)
	{ 
		if (type->returnType->getTag()!=RTTIType::RTTI_VOID || ((int)RTTIProcType2<__C,__P1,__P2>::s_get_check_idx())!=type->get_check_idx()){
			char szbuff[512];char szbuff1[512];
			RTTIProcType2<__C,__P1,__P2> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeProc2-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		((RTTIProcType2<__C,__P1,__P2>*)(type))->invoke(obj,p1,p2);
		return true;
	};

	template<class __C,class __P1,class __P2,class __P3>
	bool vinvoke(__C* obj,__P1 p1,__P2 p2,__P3 p3)
	{ 
		if (type->returnType->getTag()!=RTTIType::RTTI_VOID || ((int)RTTIProcType3<__C,__P1,__P2,__P3>::s_get_check_idx())!=type->get_check_idx()){
			char szbuff[512];char szbuff1[512];
			RTTIProcType3<__C,__P1,__P2,__P3> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeProc3-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		((RTTIProcType3<__C,__P1,__P2,__P3>*)(type))->invoke(obj,p1,p2,p3);
		return true;
	};

	template<class __C,class __P1,class __P2,class __P3,class __P4>
	bool vinvoke(__C* obj,__P1 p1,__P2 p2,__P3 p3,__P4 p4)
	{ 
		if (type->returnType->getTag()!=RTTIType::RTTI_VOID || ((int)RTTIProcType4<__C,__P1,__P2,__P3,__P4>::s_get_check_idx())!=type->get_check_idx()){
			char szbuff[512];char szbuff1[512];
			RTTIProcType4<__C,__P1,__P2,__P3,__P4> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeProc4-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		((RTTIProcType4<__C,__P1,__P2,__P3,__P4>*)(type))->invoke(obj,p1,p2,p3,p4);
		return true;
	};

	template<class __C,class __P1,class __P2,class __P3,class __P4,class __P5>
	bool vinvoke(__C* obj,__P1 p1,__P2 p2,__P3 p3,__P4 p4,__P5 p5)
	{ 
		if (type->returnType->getTag()!=RTTIType::RTTI_VOID || ((int)RTTIProcType5<__C,__P1,__P2,__P3,__P4,__P5>::s_get_check_idx())!=type->get_check_idx()){
			char szbuff[512];char szbuff1[512];
			RTTIProcType5<__C,__P1,__P2,__P3,__P4,__P5> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeProc5-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		((RTTIProcType5<__C,__P1,__P2,__P3,__P4,__P5>*)(type))->invoke(obj,p1,p2,p3,p4,p5);
		return true;
	};

	template<class __C,class __RT>
	bool invoke(__RT& rt,__C* obj)
	{ 
		if ( type->returnType->getTag()==RTTIType::RTTI_VOID || ((int)RTTIFuncType0<__RT,__C>::s_get_check_idx_r())!=type->get_check_idx_r()){
			char szbuff[512];char szbuff1[512];
			RTTIFuncType0<__RT,__C> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeFunc0-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		rt=((RTTIFuncType0<__RT,__C>*)(type))->invoke(obj);
		return true;
	};

	template<class __C,class __P1,class __RT>
	bool invoke(__RT& rt,__C* obj,__P1 p1)
	{ 
		if ( type->returnType->getTag()==RTTIType::RTTI_VOID || ((int)RTTIFuncType1<__RT,__C,__P1>::s_get_check_idx_r())!=type->get_check_idx_r()){
			char szbuff[512];char szbuff1[512];
			RTTIFuncType1<__RT,__C,__P1> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeFunc1-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		rt=((RTTIFuncType1<__RT,__C,__P1>*)(type))->invoke(obj,p1);
		return true;
	};

	template<class __C,class __P1,class __P2,class __RT>
	bool invoke(__RT& rt,__C* obj,__P1 p1,__P2 p2)
	{ 
		if ( type->returnType->getTag()==RTTIType::RTTI_VOID || ((int)RTTIFuncType2<__RT,__C,__P1,__P2>::s_get_check_idx_r())!=type->get_check_idx_r()){
			char szbuff[512];char szbuff1[512];
			RTTIFuncType2<__RT,__C,__P1,__P2> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeFunc2-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		rt=((RTTIFuncType2<__RT,__C,__P1,__P2>*)(type))->invoke(obj,p1,p2);
		return true;
	};

	template<class __C,class __P1,class __P2,class __P3,class __RT>
	bool invoke(__RT& rt,__C* obj,__P1 p1,__P2 p2,__P3 p3)
	{ 
		if ( type->returnType->getTag()==RTTIType::RTTI_VOID || ((int)RTTIFuncType3<__RT,__C,__P1,__P2,__P3>::s_get_check_idx_r())!=type->get_check_idx_r()){
			char szbuff[512];char szbuff1[512];
			RTTIFuncType3<__RT,__C,__P1,__P2,__P3> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeFunc3-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		rt=((RTTIFuncType3<__RT,__C,__P1,__P2,__P3>*)(type))->invoke(obj,p1,p2,p3);
		return true;
	};

	template<class __C,class __P1,class __P2,class __P3,class __P4,class __RT>
	bool invoke(__RT& rt,__C* obj,__P1 p1,__P2 p2,__P3 p3,__P4 p4)
	{ 
		if ( type->returnType->getTag()==RTTIType::RTTI_VOID || ((int)RTTIFuncType4<__RT,__C,__P1,__P2,__P3,__P4>::s_get_check_idx_r())!=type->get_check_idx_r()){
			char szbuff[512];char szbuff1[512];
			RTTIFuncType4<__RT,__C,__P1,__P2,__P3,__P4> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeFunc4-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		rt=((RTTIFuncType4<__RT,__C,__P1,__P2,__P3,__P4>*)(type))->invoke(obj,p1,p2,p3,p4);
		return true;
	};

	template<class __C,class __P1,class __P2,class __P3,class __P4,class __P5,class __RT>
	bool invoke(__RT& rt,__C* obj,__P1 p1,__P2 p2,__P3 p3,__P4 p4,__P5 p5)
	{ 
		if ( type->returnType->getTag()==RTTIType::RTTI_VOID || ((int)RTTIFuncType5<__RT,__C,__P1,__P2,__P3,__P4,__P5>::s_get_check_idx_r())!=type->get_check_idx_r()){
			char szbuff[512];char szbuff1[512];
			RTTIFuncType5<__RT,__C,__P1,__P2,__P3,__P4,__P5> tmpfunc(NULL,type->methodClass);
			g_logger.error("RTTIMethod.invokeFunc5-> %s<>%s  函数原型不匹配!",getMethodDeclaration(szbuff),tmpfunc.getMethodDeclaration(szbuff1,aliasname));
			return false;
		}
		rt=((RTTIFuncType5<__RT,__C,__P1,__P2,__P3,__P4,__P5>*)(type))->invoke(obj,p1,p2,p3,p4,p5);
		return true;
	};

	RTTIClassDescriptor* getDeclaringClass() { 
		return type->getClass();
	}

	RTTIMethodDescriptor(char const* name, int flags, RTTIMethodType* type,char const* aliasname=NULL) { 
		this->name = name;
		this->aliasname=aliasname;
		if (!this->aliasname){
			this->aliasname=name;
		}
		this->flags = flags;
		this->type = type;
		type->methodobj=this;
		next = NULL; 
		chain = &next;
	}

	RTTIMethodDescriptor& operator, (RTTIMethodDescriptor& method) {
		*chain = &method;
		chain = &method.next;
		return *this;
	}

	~RTTIMethodDescriptor() { 
		type->destroy();
	}


	int getIndex() { 
		return index;
	}
	bool isOverloaded(){
		return (flags & RTTI_MTH_OVERLOADED)!=0;
	}
	bool isVirtual(){
		return (flags & RTTI_MTH_VIRTUAL)!=0;
	}
	bool isAbstract(){
		return (flags & RTTI_MTH_ABSTRACT)!=0;
	}
	bool isStatic(){
		return (flags & RTTI_MTH_STATIC)!=0;
	}
protected:
	friend class RTTIType;
	friend class RTTIClassDescriptor;
	friend class RTTIBfdRepository;

	int             flags;
	int             index;
	RTTIMethodType* type;
	char const*     name;
	char const* aliasname;

	RTTIMethodDescriptor*  next;
	RTTIMethodDescriptor** chain;
};
//------------------------------------------------------------------------