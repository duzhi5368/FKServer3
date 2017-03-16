/**
*	created:		2013-4-7   0:17
*	filename: 		FKField
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <string.h>
//------------------------------------------------------------------------
enum RTTIFieldFlags { 
	RTTI_FLD_INSTANCE  = 0x0001, 
	RTTI_FLD_STATIC    = 0x0002, 
	RTTI_FLD_CONST     = 0x0004, 
	RTTI_FLD_PUBLIC    = 0x0010, 
	RTTI_FLD_PROTECTED = 0x0020, 
	RTTI_FLD_PRIVATE   = 0x0040, 
	RTTI_FLD_VIRTUAL   = 0x0100, 
	RTTI_FLD_VOLATILE  = 0x0200, 
	RTTI_FLD_TRANSIENT = 0x0400
}; 
//------------------------------------------------------------------------
class RTTIFieldDescriptor
{ 
public:
	char const* getName() { 
		return name;
	}
	char const* getAliasName() { 
		return aliasname;
	}

	void fixPtrFieldName(char* fixname,int nBackLevel=0x7fff)
	{
		if (getType()->isPointer() && nBackLevel>=0)
		{
			nBackLevel=nBackLevel>getType()->getPtrLevel()?getType()->getPtrLevel():nBackLevel;
			memset(fixname,'*',nBackLevel);
		}
		int nlen=strlen(name);
		strncpy_s(&fixname[nBackLevel],nlen+1,name,nlen);
		fixname[nlen+nBackLevel]=0;
	}

	void setValue(void* obj, void* buf, int nmaxsize) { 
		memcpy( getPtr(obj), buf, safe_min(size,nmaxsize));
	}
	void getValue(void* obj, void* buf, int nmaxsize) { 
		memcpy(buf, getPtr(obj),  safe_min(size,nmaxsize));
	}

	RTTIClassDescriptor* getDeclaringClass() { 
		return declaringClass;
	}  

	int getOffset() { 
		return offs;
	}
	void* getPtr(void* p){
		if (!(flags & RTTI_FLD_STATIC)){
			return ((void*)(((BYTE*)p)+offs));
		}else{
			return ((void*)(offs));
		}
	}
	int getSize() { 
		return size;
	}
	RTTIType* getType() { 
		return type;
	}
	int getFlags() { 
		return flags;
	}	
	int getIndex() { 
		return index;
	}
public:
	RTTIFieldDescriptor(char const* name, int offs, int size, int flags, RTTIType* type,char const* aliasname=NULL) { 
		this->name = name;
		this->aliasname=aliasname;
		if (!this->aliasname){
			this->aliasname=name;
		}
		this->offs = offs;
		this->size = size;
		this->type = type;
		this->flags = flags;
		next = NULL; 
		chain = &next;
	}

	RTTIFieldDescriptor& operator, (RTTIFieldDescriptor& field) {
		*chain = &field;
		chain = &field.next;
		return *this;
	}

	~RTTIFieldDescriptor() { 
		type->destroy();
	}
protected:
	friend class RTTIType;
	friend class RTTIClassDescriptor;
	friend class RTTIBfdRepository;

	int         flags;
	int         index;
	RTTIType*   type;
	int         offs;
	int         size;
	char const* name;
	char const* aliasname;

	RTTIClassDescriptor*  declaringClass;

	RTTIFieldDescriptor*  next;
	RTTIFieldDescriptor** chain;
};
//------------------------------------------------------------------------