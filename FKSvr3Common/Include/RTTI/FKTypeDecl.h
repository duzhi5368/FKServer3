/**
*	created:		2013-4-7   0:09
*	filename: 		FKTypeDecl
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "../FKBaseDefine.h"
#include <typeinfo>
#include <memory>
//------------------------------------------------------------------------
#pragma warning (disable:4189)
#pragma warning (disable:4512)
#pragma warning (disable:4291)	
//------------------------------------------------------------------------
#define RTTI_FIELD_N(x, flags, aname) \
	*new RTTIFieldDescriptor(#x, (char*)&tempthis->x-(char*)tempthis, sizeof(tempthis->x), flags, RTTITypeOf(tempthis->x),#aname)

#define RTTI_ARRAY_N(x, flags, aname) \
	*new RTTIFieldDescriptor(#x, (char*)&tempthis->x-(char*)tempthis, sizeof(tempthis->x), flags, \
	new RTTIArrayType(RTTITypeOf(*tempthis->x), sizeof(tempthis->x)/sizeof(*tempthis->x)),#aname)

#define RTTI_BASE_CLASS_N(BC, flags, aname)  \
	*new RTTIFieldDescriptor(#BC, (char*)((BC*)tempthis) - (char*)tempthis, sizeof(BC), flags, \
	new RTTIDerivedType(__RTTITypeOfPtr((BC*)NULL)),#BC)

#define RTTI_FIELD(x, flags)				RTTI_FIELD_N(x,flags,x)
#define RTTI_ARRAY(x, flags)				RTTI_ARRAY_N(x, flags,x) 
#define RTTI_BASE_CLASS(BC, flags)			RTTI_BASE_CLASS_N(BC, flags,BC)  

#define RTTI_DESCRIBE(T,components,methods) \
	static RTTIClassDescriptor T##RTTIDescriptorClassObj; \
	__inline static RTTIClassDescriptor* RTTI() { \
	return &##T##RTTIDescriptorClassObj; \
	} \
	__inline static RTTIClassDescriptor* RTTIGetClass() { \
	return &##T##RTTIDescriptorClassObj; \
	} \
	template<class CT> \
	bool ClassIs() { \
	return RTTIGetClass()==__RTTITypeOfPtr((CT*)NULL);\
	} \
	bool ClassNameIs(char* cname) { \
	char buf[256]={0}; \
	RTTIClassDescriptor* cls=RTTIGetClass(); \
	if (cls) \
	{ \
	cls->getTypeName(buf); \
	return strcmp(buf,cname)==0; \
	} \
	return false; \
	} \
	RTTIClassDescriptor* RTTIFindClass() 	\
	{ 	\
	RTTIRepository* repo = RTTIRepository::getInstance(); 	\
	return repo?repo->findClass(typeid(*this).name()):NULL;		\
	} 	\
	RTTIFieldDescriptor* RTTIDescribeFields() { \
	\
	T* tempthis=((T*)0x0000ffff); 	\
	return (components); \
	} \
	typedef T self; \
	__inline static RTTIMethodDescriptor* RTTIDescribeMethods() { \
	return (methods); \
	}  \
	template<class CT> \
	bool IsKindOf() 	\
	{  	\
	RTTIClassDescriptor* kindclass=__RTTITypeOfPtr((CT*)NULL); 	\
	RTTIClassDescriptor* thisclass=RTTIGetClass();  \
	if (thisclass){  \
	return thisclass->IsKindOf(kindclass);  \
	}  \
	return false;  \
	}  \

#define RTTI_DESCRIBE_V(T,components,methods) \
	static RTTIClassDescriptor T##RTTIDescriptorClassObj; \
	__inline static RTTIClassDescriptor* RTTI() { \
	return &##T##RTTIDescriptorClassObj; \
	} \
	virtual RTTIClassDescriptor* RTTIGetClass() 	\
	{ 	\
	return &##T##RTTIDescriptorClassObj;	\
	} 	\
	template<class CT> \
	bool ClassIs() { \
	return RTTIGetClass()==__RTTITypeOfPtr((CT*)NULL);\
	} \
	bool ClassNameIs(char* cname) { \
	char buf[256]={0}; \
	RTTIClassDescriptor* cls=RTTIGetClass(); \
	if (cls) \
	{ \
	cls->getTypeName(buf); \
	return strcmp(buf,cname)==0; \
	} \
	return false; \
	} \
	RTTIClassDescriptor* RTTIFindClass() 	\
	{ 	\
	return RTTIGetClass(); \
	} 	\
	RTTIFieldDescriptor* RTTIDescribeFields() { \
	\
	T* tempthis=((T*)0x0000ffff); 	\
	return (components); \
	} \
	typedef T self; \
	__inline static RTTIMethodDescriptor* RTTIDescribeMethods() { \
	return (methods); \
	}  \
	template<class CT> \
	bool IsKindOf() 	\
	{  	\
	RTTIClassDescriptor* kindclass=__RTTITypeOfPtr((CT*)NULL); 	\
	RTTIClassDescriptor* thisclass=RTTIGetClass();  \
	if (thisclass){  \
	return thisclass->IsKindOf(kindclass);  \
	}  \
	return false;  \
	}  \

#define RTTI_BASE_REGISTER_N(T,TNAME,flags,sortfieldtype)	\
	static RTTIFieldDescriptor* RTTIDescribeFieldsOf##T() { \
	return ((T*)(NULL))->RTTIDescribeFields(); \
	} \
	static RTTIMethodDescriptor* RTTIDescribeMethodsOf##T() { \
	return ((T*)(NULL))->RTTIDescribeMethods(); \
	} \
	RTTIClassDescriptor T::##T##RTTIDescriptorClassObj(#T, sizeof(T), &RTTIDescribeFieldsOf##T, \
	&RTTIDescribeMethodsOf##T,flags,sortfieldtype,#TNAME); \
	template<> \
	RTTIClassDescriptor* RTTIClassDescriptorHelper<T>::getClassDescriptor() {   \
	return T::RTTI(); \
	}

template<class __T>
class RTTIClassDescriptorHelper { 
public:
	static RTTIClassDescriptor* getClassDescriptor();
};

template<class __P>
inline RTTIClassDescriptor* __RTTITypeOfPtr(__P const*) { 
	return RTTIClassDescriptorHelper<__P>::getClassDescriptor();
}

template<class __T>
inline RTTIType* RTTITypeOf(__T&) { 
	return RTTIClassDescriptorHelper<__T>::getClassDescriptor();
}

#define RTTI_DESCRIBE_STRUCT(T, components)				RTTI_DESCRIBE(T, &##components ,NULL)	
#define RTTI_DESCRIBE_STRUCT_V(T, components)			RTTI_DESCRIBE_V(T, &##components ,NULL)	

#define RTTI_DESCRIBE_CLASS(T, components,methods)		RTTI_DESCRIBE(T, &##components ,&##methods)	
#define RTTI_DESCRIBE_CLASS_V(T, components,methods)	RTTI_DESCRIBE_V(T, &##components ,&##methods)	

#define RTTI_REGISTER_STRUCT(T)							RTTI_BASE_REGISTER_N(T,T,0,eSortNone)
#define RTTI_REGISTER_STRUCT_N(T,TNAME)					RTTI_BASE_REGISTER_N(T,TNAME,0,eSortNone)
#define RTTI_REGISTER_CLASS								RTTI_REGISTER_STRUCT
#define RTTI_REGISTER_CLASS_N							RTTI_REGISTER_STRUCT_N

#define RTTI_BASE_REGISTER_STRUCT(T)					RTTI_BASE_REGISTER_N(T,T,0,eSortNone)
#define RTTI_BASE_REGISTER_STRUCT_N(T,TNAME)			RTTI_BASE_REGISTER_N(T,TNAME,0,eSortNone)
#define RTTI_BASE_REGISTER_CLASS						RTTI_BASE_REGISTER_STRUCT
#define RTTI_BASE_REGISTER_CLASS_N						RTTI_BASE_REGISTER_STRUCT_N


template<>
inline RTTIType* RTTITypeOf(char&) { 
	return &RTTIType::charType;
}
template<>
inline RTTIType* RTTITypeOf(unsigned char&) {
	return &RTTIType::ucharType;
}
template<>
inline RTTIType* RTTITypeOf(signed char&) {
	return &RTTIType::scharType;
}

template<>
inline RTTIType* RTTITypeOf(short&) {
	return &RTTIType::shortType;
}
template<>
inline RTTIType* RTTITypeOf(unsigned short&) { 
	return &RTTIType::ushortType;
}
template<>
inline RTTIType* RTTITypeOf(int&) {
	return &RTTIType::intType;
}
template<>
inline RTTIType* RTTITypeOf(unsigned int&) { 
	return &RTTIType::uintType;
}
template<>
inline RTTIType* RTTITypeOf(long&) { 
	return &RTTIType::longType;
}
template<>
inline RTTIType* RTTITypeOf(unsigned long&) { 
	return &RTTIType::ulongType;
}
template<>
inline RTTIType* RTTITypeOf(__int64&) { 
	return &RTTIType::i64Type;
}
template<>
inline RTTIType* RTTITypeOf(unsigned __int64&) { 
	return &RTTIType::ui64Type;
}
template<>
inline RTTIType* RTTITypeOf(float&) { 
	return &RTTIType::floatType;
}
template<>
inline RTTIType* RTTITypeOf(double&) { 
	return &RTTIType::doubleType;
}
template<>
inline RTTIType* RTTITypeOf(bool&) { 
	return &RTTIType::boolType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(char*) { 
	return (RTTIClassDescriptor*)&RTTIType::charType;
}
inline RTTIType* RTTITypeOfPtr(char*) { 
	return &RTTIType::charType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(unsigned char*) { 
	return (RTTIClassDescriptor*)&RTTIType::ucharType;
}
inline RTTIType* RTTITypeOfPtr(unsigned char*) {
	return &RTTIType::ucharType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(signed char*) { 
	return (RTTIClassDescriptor*)&RTTIType::scharType;
}
inline RTTIType* RTTITypeOfPtr(signed char*) {
	return &RTTIType::scharType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(short*) { 
	return (RTTIClassDescriptor*)&RTTIType::shortType;
}
inline RTTIType* RTTITypeOfPtr(short*) {
	return &RTTIType::shortType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(unsigned short*) { 
	return (RTTIClassDescriptor*)&RTTIType::ushortType;
}
inline RTTIType* RTTITypeOfPtr(unsigned short*) { 
	return &RTTIType::ushortType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(int*) { 
	return (RTTIClassDescriptor*)&RTTIType::intType;
}
inline RTTIType* RTTITypeOfPtr(int*) {
	return &RTTIType::intType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(unsigned int*) { 
	return (RTTIClassDescriptor*)&RTTIType::uintType;
}
inline RTTIType* RTTITypeOfPtr(unsigned int*) { 
	return &RTTIType::uintType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(long*) { 
	return (RTTIClassDescriptor*)&RTTIType::longType;
}
inline RTTIType* RTTITypeOfPtr(long*) { 
	return &RTTIType::longType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(unsigned long*) { 
	return (RTTIClassDescriptor*)&RTTIType::ulongType;
}
inline RTTIType* RTTITypeOfPtr(unsigned long*) { 
	return &RTTIType::ulongType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(__int64*) { 
	return (RTTIClassDescriptor*)&RTTIType::i64Type;
}
inline RTTIType* RTTITypeOfPtr(__int64*) { 
	return &RTTIType::i64Type;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(unsigned __int64*) { 
	return (RTTIClassDescriptor*)&RTTIType::ui64Type;
}
inline RTTIType* RTTITypeOfPtr(unsigned __int64*) { 
	return &RTTIType::ui64Type;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(float*) { 
	return (RTTIClassDescriptor*)&RTTIType::floatType;
}
inline RTTIType* RTTITypeOfPtr(float*) { 
	return &RTTIType::floatType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(double*) { 
	return (RTTIClassDescriptor*)&RTTIType::doubleType;
}
inline RTTIType* RTTITypeOfPtr(double*) { 
	return &RTTIType::doubleType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(bool*) { 
	return (RTTIClassDescriptor*)&RTTIType::boolType;
}
inline RTTIType* RTTITypeOfPtr(bool*) { 
	return &RTTIType::boolType;
}

inline RTTIClassDescriptor* __RTTITypeOfPtr(void*) { 
	return (RTTIClassDescriptor*)&RTTIType::voidType;
}
inline RTTIType* RTTITypeOfPtr(void*) { 
	return &RTTIType::voidType;
}


template<class __P>
inline RTTIType* RTTITypeOfPtr(__P const*const*) { 
	return new RTTIPtrType( __RTTITypeOfPtr( (__P*)NULL ) );
}