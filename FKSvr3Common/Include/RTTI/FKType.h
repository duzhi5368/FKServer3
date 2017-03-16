/**
*	created:		2013-4-7   0:04
*	filename: 		FKType
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <string.h>
//------------------------------------------------------------------------
class RTTIClassDescriptor;
class RTTIFieldDescriptor;
//------------------------------------------------------------------------
#define RTTI_MAX_PARAMETERS  5
//------------------------------------------------------------------------
class RTTIType 
{ 
public:
	int  getTag() 
	{ 
		return tag;
	}

	virtual char* getTypeName(char* buf);

	static RTTIType voidType;

	static RTTIType charType;
	static RTTIType ucharType;
	static RTTIType scharType;

	static RTTIType shortType;
	static RTTIType ushortType;
	static RTTIType intType;
	static RTTIType uintType;
	static RTTIType longType;
	static RTTIType ulongType;
	static RTTIType i64Type;
	static RTTIType ui64Type;
	static RTTIType floatType;
	static RTTIType doubleType;
	static RTTIType boolType;
	static RTTIType unknownType;

	enum TypeTag { 
		RTTI_UNKNOWN, 
		RTTI_VOID, 

		RTTI_BYTE,
		RTTI_CHAR, 
		RTTI_UCHAR, 
		RTTI_SCHAR, 
		RTTI_WCHAR,
		RTTI_SHORT, 
		RTTI_USHORT, 
		RTTI_INT, 
		RTTI_UINT, 
		RTTI_LONG, 
		RTTI_ULONG, 
		RTTI_I64, 
		RTTI_UI64, 
		RTTI_ENUM,		

		RTTI_BOOL, 

		RTTI_FLOAT, 
		RTTI_DOUBLE,  

		RTTI_ARRAY, 
		RTTI_STRUCT, 
		RTTI_PTR,  
		RTTI_DERIVED,
		RTTI_METHOD 
	};


	bool isBuiltin() { 

		return ( tag <= RTTI_DOUBLE ) && ( tag!=RTTI_ENUM );
	}

	bool isScalar() { 

		return tag > RTTI_VOID && tag <= RTTI_DOUBLE;
	}
	bool isInt(){

		return tag > RTTI_VOID && tag <= RTTI_BOOL;
	}
	bool isDouble(){

		return tag >= RTTI_FLOAT && tag <= RTTI_DOUBLE;
	}

	bool isArray() { 
		return tag == RTTI_ARRAY;
	}


	bool isPointer() { 
		return tag == RTTI_PTR;
	}

	virtual int getPtrLevel()
	{
		return m_nPtrLevel;
	}

	virtual RTTIType* getPtrDataType(int nBackLevel=0x7fff)
	{
		return this;
	}

	bool isClass() { 
		return tag == RTTI_STRUCT;
	}


	bool isBaseClass() { 
		return tag == RTTI_DERIVED;
	}


	~RTTIType();

protected:
	friend class RTTIClassDescriptor;
	friend class RTTIFieldDescriptor;
	friend class RTTIMethodDescriptor;
	friend class RTTIPtrType;
	friend class RTTIMethodType;
	friend class RTTIArrayType;

	unsigned short int   tag;
	unsigned short int   m_nPtrLevel;
	void destroy() {
		if ( !isBuiltin() && tag!=RTTI_STRUCT ) { 

			delete this;
		}
	}

	RTTIType(unsigned short int tag) { 
		this->tag = tag;
		m_nPtrLevel=0;
	}

	RTTIType(unsigned short int tag,unsigned short int usersettag) { 
		this->tag = usersettag;
		m_nPtrLevel=0;
	}
};
//------------------------------------------------------------------------
class RTTIPtrType : public RTTIType {     
public:
	RTTIPtrType(RTTIType* ptrType) : RTTIType(RTTI_PTR) {
		this->ptrType = ptrType;

		RTTIType* tempptrType=this;
		m_nPtrLevel=0;
		while (tempptrType->isPointer()) {
			m_nPtrLevel++;
			tempptrType=((RTTIPtrType*)tempptrType)->ptrType;
		}
	}
	char* getTypeName(char* buf);

	~RTTIPtrType(){
		if (m_nPtrLevel>0){
			ptrType->destroy();
		}
	}

	virtual RTTIType* getPtrDataType(int nBackLevel=0x7fff){
		RTTIType* tempptrType=this;
		if (nBackLevel>0){
			nBackLevel=nBackLevel>m_nPtrLevel?m_nPtrLevel:nBackLevel;
			while (nBackLevel>0){
				tempptrType=((RTTIPtrType*)tempptrType)->ptrType;
				nBackLevel--;
			}
		}
		return tempptrType;
	}
protected:
	RTTIType* ptrType;
};
//------------------------------------------------------------------------
class RTTIArrayType : public RTTIType {     
public:
	RTTIArrayType(RTTIType* elemType, int nElems) : RTTIType(RTTI_ARRAY) {
		this->elemType = elemType;
		this->nElems = nElems;
	}
	~RTTIArrayType(){
		elemType->destroy();
	}
	char* getTypeName(char* buf);

	int  getArraySize() { 
		return nElems;
	}

	RTTIType* getElementType() { 
		return elemType;
	}

protected:
	RTTIType* elemType;
	int       nElems;
};
//------------------------------------------------------------------------
class RTTIDerivedType : public RTTIType {     
public:
	RTTIDerivedType(RTTIClassDescriptor* baseClass) : RTTIType(RTTI_DERIVED) {
		this->baseClass = baseClass;
	}

	RTTIClassDescriptor* getBaseClass() { 
		return baseClass;
	}

	char* getTypeName(char* buf);

protected:
	RTTIClassDescriptor* baseClass;
};
//------------------------------------------------------------------------
class RTTIMethodType : public RTTIType { 
public:
	char* getTypeName(char* buf);

	char* getMethodDeclaration(char* buf, char const* name);

	static  __inline int s_get_check_idx(){ return (int)(0); };
	static  __inline int s_get_check_idx_rc(){ return (int)(0); };
	static  __inline int s_get_check_idx_r(){ return (int)(0); };
	static  __inline int s_get_check_idx_c(){ return (int)(0); };

	virtual  int get_check_idx(){ return (int)(0); };
	virtual  int get_check_idx_rc(){ return (int)(0); };
	virtual  int get_check_idx_r(){ return (int)(0); };
	virtual  int get_check_idx_c(){ return (int)(0); };

	RTTIClassDescriptor* getClass() { 
		return methodClass;
	}

	RTTIType* getReturnType() { 
		return returnType;
	}

	RTTIType** getParameterTypes() { 
		return paramTypes;
	}

	int getNumberOfParameters() { 
		return nParams;
	}

	RTTIMethodType() : RTTIType(RTTI_METHOD) {
		returnType=NULL;
		methodobj=NULL;
		memset(paramTypes,0,sizeof(paramTypes));
	}
	~RTTIMethodType() { 

		for(int i=0;i<RTTI_MAX_PARAMETERS;i++){
			if (paramTypes[i]){
				paramTypes[i]->destroy();
				paramTypes[i]=NULL;
			}
		}
		if (returnType){
			returnType->destroy();
		}
	}
protected:
	friend class RTTIMethodDescriptor;
	friend class RTTIBfdRepository;

	RTTIType*				returnType;
	int						nParams;
	RTTIType*				paramTypes[RTTI_MAX_PARAMETERS];
	RTTIClassDescriptor*	methodClass;
	RTTIMethodDescriptor*	methodobj;
};
//------------------------------------------------------------------------