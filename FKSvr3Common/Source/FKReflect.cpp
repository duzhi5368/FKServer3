/**
*	created:		2013-4-7   0:27
*	filename: 		FKReflect
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/RTTI/FKReflect.h"
#include "../Include/FKVsVer8Define.h"
//------------------------------------------------------------------------
RTTIRepository* RTTIRepository::theRepository=NULL;
//------------------------------------------------------------------------
struct stAutoinitRTTIRepository
{
	stAutoinitRTTIRepository()
	{
		RTTIRepository::getInstance();
	}
	~stAutoinitRTTIRepository()
	{
		RTTIRepository::delInstance();
	}
};
//------------------------------------------------------------------------
stAutoinitRTTIRepository	AutoinitRTTIRepository;
//------------------------------------------------------------------------
inline unsigned hashFunction(const char* name)
{ 
	unsigned h = 0;
	while (*name != '\0') { 
		h = ((h << 8) ^ (*name++ & 0xFF)) | (h >> 24);
	}
	return h;
}
//------------------------------------------------------------------------
bool RTTIRepository::addClass(RTTIClassDescriptor* cls) 
{ 
	AILOCKT(*this);

	unsigned hname = hashFunction(cls->name);
	for (RTTIClassDescriptor* cp = hashTable[hname % RTTI_CLASS_HASH_SIZE]; cp != NULL; cp = cp->collisionChain){ 
		if (cp->hashCode == hname && strcmp(cp->name, cls->name) == 0){
			return false;
		}
	}	

	unsigned haliasname = hashFunction(cls->aliasname);
	for (RTTIClassDescriptor* cp = hashAliasTable[haliasname % RTTI_CLASS_HASH_SIZE]; cp != NULL; cp = cp->collisionAliasChain){ 
		if (cp->hashAliasCode == haliasname && strcmp(cp->aliasname, cls->aliasname) == 0){
			return false;
		}
	}

	cls->next = classes;
	classes = cls;

	cls->collisionChain = hashTable[hname % RTTI_CLASS_HASH_SIZE];    
	hashTable[hname % RTTI_CLASS_HASH_SIZE] = cls;
	cls->hashCode = hname;

	cls->collisionAliasChain = hashAliasTable[haliasname % RTTI_CLASS_HASH_SIZE];    
	hashAliasTable[haliasname % RTTI_CLASS_HASH_SIZE] = cls;
	cls->hashAliasCode = haliasname;
	return true;
}
//------------------------------------------------------------------------
RTTIClassDescriptor* RTTIRepository::findClassByAliasName(char const* pAliasName,bool bocasestr){
	unsigned h = hashFunction(pAliasName);
	AILOCKT(*this);
	for (RTTIClassDescriptor* cls = hashAliasTable[h % RTTI_CLASS_HASH_SIZE]; cls != NULL; cls = cls->collisionAliasChain) { 
		if (cls->hashAliasCode == h && (strcmp(cls->aliasname, pAliasName) == 0 || (bocasestr && stricmp(cls->aliasname, pAliasName) == 0))) { 
			return cls;
		}
	}
	return NULL;
}
//------------------------------------------------------------------------
RTTIClassDescriptor* RTTIRepository::findClass(char const* pclassname,bool bocasestr)
{
	bool bospace=false;
	char* name=(char*)strrchr(pclassname,0x20);
	if (name==NULL){
		name=(char*)pclassname;		
	}else{
		bospace=true;
		name++;
	}

	unsigned h = hashFunction(name);
	AILOCKT(*this);
	for (RTTIClassDescriptor* cls = hashTable[h % RTTI_CLASS_HASH_SIZE]; cls != NULL; cls = cls->collisionChain) { 
		if (cls->hashCode == h && (strcmp(cls->name, name) == 0 || (bocasestr && stricmp(cls->name, name) == 0))) { 
			return cls;
		}
	}

	name=(char*)strrchr(pclassname,':');
	if (name){
		name++;
		unsigned h = hashFunction(name);
		for (RTTIClassDescriptor* cls = hashTable[h % RTTI_CLASS_HASH_SIZE]; cls != NULL; cls = cls->collisionChain) { 
			if (cls->hashCode == h && (strcmp(cls->name, name) == 0 || (bocasestr && stricmp(cls->name, name) == 0))) { 
				return cls;
			}
		}
	}else if (bospace){

		name=(char*)pclassname;	
		unsigned h = hashFunction(name);
		for (RTTIClassDescriptor* cls = hashTable[h % RTTI_CLASS_HASH_SIZE]; cls != NULL; cls = cls->collisionChain) { 
			if (cls->hashCode == h && (strcmp(cls->name, name) == 0 || (bocasestr && stricmp(cls->name, name) == 0))) { 
				return cls;
			}
		}
	}
	return NULL;
}
//------------------------------------------------------------------------
bool RTTIRepository::load(char const* filePath)
{
	return false;
}
//------------------------------------------------------------------------
bool SaveClassToXml(void* p,RTTIClassDescriptor* pclass,TiXmlElement* node,fnfieldfilter filter,const char* objname){
	RTTIClassDescriptor** pbases=pclass->getBaseClasses();
	int nbasecount=pclass->getNumberOfBaseClasses();
	if (pbases && nbasecount){
		for (int i=0;i<nbasecount;i++){
			SaveClassToXml(pclass->ConvertClassPtr(p,i),pbases[i],node,filter,objname);
		}
	}
	RTTIFieldDescriptor** pfields=pclass->getFields();
	int nfieldcount=pclass->getNumberOfFields();
	if (pfields && nfieldcount){
		char buf[1024];
		char szbuf[1024*16];
		char szname[1024];
		for (int i=0;i<nfieldcount;i++){
			RTTIFieldDescriptor* pcurfield=pfields[i];
			if (pcurfield && (filter==NULL || (filter && filter(p,pclass,node,pcurfield,objname))) ){
				ZeroMemory(buf,32);
				pcurfield->getValue(p,&buf,sizeof(buf));
				if (pcurfield->getType()->isInt()){
					if ( (*((__int64*)&buf))>0x007fffff ){
						sprintf_s(szbuf,sizeof(szbuf)-1,"0x%I64x", *((__int64*)&buf) );
					}else{
						sprintf_s(szbuf,sizeof(szbuf)-1,"%I64u", *((__int64*)&buf) );
					}
				}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_FLOAT){
					sprintf_s(szbuf,sizeof(szbuf)-1,"%f", *((float*)&buf) );
				}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_DOUBLE){
					sprintf_s(szbuf,sizeof(szbuf)-1,"%f", *((double*)&buf) );	
				}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_ARRAY){
					RTTIArrayType* parray=(RTTIArrayType*)pcurfield->getType();
					if (parray->getElementType() && parray->getElementType()->getTag()==RTTIType::RTTI_CHAR){
						sprintf_s(szbuf,sizeof(szbuf)-1,"%s",(pcurfield->getPtr(p)));
					}else{
						sprintf_s(szbuf,sizeof(szbuf)-1,"ArrayCannotSave");
					}
				}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_PTR){
					sprintf_s(szbuf,sizeof(szbuf)-1,"PtrCannotSave");
				}else{
					sprintf_s(szbuf,sizeof(szbuf)-1,"RttiNotFound");
				}
				RTTI_FIELDNAME(pcurfield,szname,objname);
				node->SetAttribute(szname,szbuf);
			}
		}
	}
	return true;
}
//------------------------------------------------------------------------
bool InitClassFromXml(void* p,RTTIClassDescriptor* pclass,TiXmlElement* node,fnfieldfilter filter,const char* objname){
	RTTIClassDescriptor** pbases=pclass->getBaseClasses();
	int nbasecount=pclass->getNumberOfBaseClasses();
	if (pbases && nbasecount){
		for (int i=0;i<nbasecount;i++){
			InitClassFromXml(pclass->ConvertClassPtr(p,i),pbases[i],node,filter,objname);
		}
	}
	RTTIFieldDescriptor** pfields=pclass->getFields();
	int nfieldcount=pclass->getNumberOfFields();
	if (pfields && nfieldcount){
		char buf[1024];
		char szname[1024];
		for (int i=0;i<nfieldcount;i++){
			RTTIFieldDescriptor* pcurfield=pfields[i];
			if (pcurfield && (filter==NULL || (filter && filter(p,pclass,node,pcurfield,objname))) ){
				const char* szvalue=NULL;
				RTTI_FIELDNAME(pcurfield,szname,objname);
				szvalue=node->Attribute(szname);
				if (szvalue){
					if (pcurfield->getType()->isInt()){
						if (strlen(szvalue)>2 && szvalue[0]=='0' && (szvalue[1]=='x' || szvalue[1]=='X')){
							*((__int64*)&buf)=strtoull(&szvalue[2],(char **)NULL,16);
						}else{
							*((__int64*)&buf)=strtoull(szvalue,(char **)NULL,10);
						}
					}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_FLOAT){
						*((float*)&buf)=(float)atof(szvalue);
					}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_DOUBLE){
						*((double*)&buf)=atof(szvalue);
					}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_ARRAY){
						RTTIArrayType* parray=(RTTIArrayType*)pcurfield->getType();
						if (!(parray->getElementType() && parray->getElementType()->getTag()==RTTIType::RTTI_CHAR)){

							szvalue=NULL;
						}else{
							strcpy_s(buf,sizeof(buf)-1,szvalue);
						}
					}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_PTR){
						szvalue=NULL;
					}else{
						szvalue=NULL;
					}
					if (szvalue){
						pcurfield->setValue(p,buf,sizeof(buf));
					}
				}
			}
		}
	}
	return true;
}
//------------------------------------------------------------------------