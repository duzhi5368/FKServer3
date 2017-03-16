/**
*	created:		2013-4-7   21:34
*	filename: 		FKDBConnPool
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/Database/FKDBConnPool.h"
#include "../Include/Database/FKMsAdoDBConn.h"
#include "../Include/FKLogger.h"
#include "../Include/RTTI/FKReflect.h"
//------------------------------------------------------------------------
bool dbCol::writeNameBack(const char* szname)
{
	if (  ((state & _DBCOL_NOT_CHECK_NAME_)!=0 && (state & _DBCOL_WRITENAMEBACK_)!=0 ) ){
		name=szNameBuffer;
		strcpy_s(szNameBuffer,sizeof(szNameBuffer)-1,szname);
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
void dbCol::clone(const dbCol* pcol){
	CopyMemory(this,pcol,sizeof(*this));
	if (pcol->name){
		szNameBuffer[0]=0;
		strcpy_s(szNameBuffer,sizeof(szNameBuffer)-1,pcol->name); 
		name=szNameBuffer;
	}else{ 
		szNameBuffer[0]=0;name=NULL; 
	}
}
//------------------------------------------------------------------------
unsigned int defaultHashCode(const void *anyArg){
	return anyArg==NULL?0:((int)*((BYTE*)anyArg));
}
//------------------------------------------------------------------------
stUrlInfo::stSqlTypeInfo stUrlInfo::zSqlTypes[]=	{
	{"mssql://",stUrlInfo::eMsSql,16,CAdoSqlClientHandle::newInstance},
	{"msmdb://",stUrlInfo::eMsMdb,16,CAccessMdbClient::newInstance},

	{NULL,stUrlInfo::eTypeError,NULL},
};
//------------------------------------------------------------------------
CLD_dbColMaker::CLD_dbColMaker(){
	m_dbcols.reserve(50);
}
//------------------------------------------------------------------------
bool CLD_dbColMaker::put(const char *n,int t,unsigned int ts,unsigned int ndo,unsigned char * da,BYTE st){
	static dbCol s_tmp={ _DBC_NULL_ };
	dbCol tmp={n,t,st,0,ts,ndo,da};
	if (m_dbcols.size()==0){
		m_dbcols.push_back(tmp);	
	}else{
		m_dbcols.back()=tmp;
	}
	m_dbcols.push_back(s_tmp);	
	return true;	
}
//------------------------------------------------------------------------
const dbCol* CLD_dbColMaker::getdbcol(){
	if (m_dbcols.size()>0){
		return &m_dbcols[0];
	}else{
		return NULL;
	}
}
//------------------------------------------------------------------------
CSqlClientHandle::eDB_CLDTYPE ConvertRtti2Dbtype(RTTIFieldDescriptor* pfield){
	switch (pfield->getType()->getTag())
	{
	case RTTIType::RTTI_UNKNOWN:
		return CSqlClientHandle::DB_TYPEEND;
	case RTTIType::RTTI_VOID:
		return CSqlClientHandle::DB_TYPEEND;
	case RTTIType::RTTI_CHAR:
		return CSqlClientHandle::DB_BYTE;
	case RTTIType::RTTI_UCHAR:
		return CSqlClientHandle::DB_BYTE;
	case RTTIType::RTTI_SCHAR:
		return CSqlClientHandle::DB_BYTE;
	case RTTIType::RTTI_SHORT:
		return CSqlClientHandle::DB_WORD;
	case RTTIType::RTTI_USHORT:
		return CSqlClientHandle::DB_WORD;
	case RTTIType::RTTI_INT:
		return CSqlClientHandle::DB_DWORD;
	case RTTIType::RTTI_UINT:
		return CSqlClientHandle::DB_DWORD;
	case RTTIType::RTTI_LONG:
		return CSqlClientHandle::DB_DWORD;
	case RTTIType::RTTI_ULONG:
		return CSqlClientHandle::DB_DWORD;
	case RTTIType::RTTI_I64:
		return CSqlClientHandle::DB_QWORD;
	case RTTIType::RTTI_UI64:
		return CSqlClientHandle::DB_QWORD;
	case RTTIType::RTTI_ENUM:
		{
			switch (pfield->getSize())
			{
			case 1:return CSqlClientHandle::DB_BYTE;
			case 2:return CSqlClientHandle::DB_WORD;
			case 4:return CSqlClientHandle::DB_DWORD;
			default:return CSqlClientHandle::DB_TYPEEND; 
			}
		}
		break;
	case RTTIType::RTTI_FLOAT:
		return CSqlClientHandle::DB_FLOAT;
	case RTTIType::RTTI_DOUBLE:
		return CSqlClientHandle::DB_DOUBLE;
	case RTTIType::RTTI_BOOL:
		{
			switch (pfield->getSize())
			{
			case 1:return CSqlClientHandle::DB_BYTE;
			case 2:return CSqlClientHandle::DB_WORD;
			case 4:return CSqlClientHandle::DB_DWORD;
			default:return CSqlClientHandle::DB_TYPEEND; 
			}
		}
	case RTTIType::RTTI_ARRAY:
		{
			RTTIArrayType* parray=((RTTIArrayType*)pfield->getType());
			switch ( parray->getElementType()->getTag() )
			{
			case RTTIType::RTTI_CHAR: 
			case RTTIType::RTTI_SCHAR: 
				{
					return CSqlClientHandle::DB_STR;
				}
			case RTTIType::RTTI_UCHAR: 
				{
					return CSqlClientHandle::DB_BIN;
				}
			}
		}
	default:
		{
			return CSqlClientHandle::DB_TYPEEND; 
		}

	}
}
//------------------------------------------------------------------------
bool CRttiDbDataLoader::MakedbCol(CLD_dbColMaker& maker,RTTIClassDescriptor* pclass,int srcdatasize){
	if (pclass){
		RTTIClassDescriptor** pbases=pclass->getBaseClasses();
		int nbasecount=pclass->getNumberOfBaseClasses();
		if (pbases && nbasecount){
			for (int i=0;i<nbasecount;i++){
				if (pbases[i]){
					if (!MakedbCol(maker,pbases[i],srcdatasize)){
						return false;
					}
				}
			}
		}
		RTTIFieldDescriptor** pfields=pclass->getFields();
		int nfieldcount=pclass->getNumberOfFields();
		if (pfields && nfieldcount){
			for (int i=0;i<nfieldcount;i++){
				RTTIFieldDescriptor* pcurfield=pfields[i];
				if (pcurfield){
					CSqlClientHandle::eDB_CLDTYPE dt=ConvertRtti2Dbtype(pcurfield);
					if (dt>CSqlClientHandle::DB_TYPEBEGIN && dt<CSqlClientHandle::DB_TYPEEND){
						if (pcurfield->getFlags() & RTTI_FLD_STATIC){
							maker.put( pcurfield->getAliasName(),dt,pcurfield->getSize(),NULL,(unsigned char *)pcurfield->getPtr(NULL) );
						}else{
							maker.put(pcurfield->getAliasName(),dt,pcurfield->getSize(),pcurfield->getOffset(),NULL);
						}
					}else if (pcurfield->getType()->getTag()==RTTIType::RTTI_STRUCT){
						if (!MakedbCol(maker,(RTTIClassDescriptor*)pcurfield->getType(),srcdatasize)){
							return false;
						}
					}else{
						return false;
					}
				}else{
					return false;
				}
			}
		}
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
stUrlInfo::stSqlTypeInfo* stUrlInfo::gettypeinfo(eSqlType type)
{
	FUNCTION_BEGIN;
	int i=0;
	while (zSqlTypes[i].head!=NULL && zSqlTypes[i].pnew!=NULL){
		if (zSqlTypes[i].type==type){
			return &zSqlTypes[i];
		}
		i++;
	}
	return NULL;
}
//------------------------------------------------------------------------
const char* stUrlInfo::parseurlhead(const char* connstr){
	FUNCTION_BEGIN;
	int i=0;
	while (zSqlTypes[i].head!=NULL && zSqlTypes[i].pnew!=NULL){
		if (0 == strnicmp(connstr, zSqlTypes[i].head, strlen(zSqlTypes[i].head))){
			sqltypeinfo=&zSqlTypes[i];
			return (connstr+strlen(zSqlTypes[i].head));
		}
		i++;
	}
	return NULL;
}
//------------------------------------------------------------------------
void stUrlInfo::parseURLString()
{
	FUNCTION_BEGIN;
	urlerror=false;
	ZeroMemory(host,sizeof(host));
	ZeroMemory(user,sizeof(user));
	ZeroMemory(passwd,sizeof(passwd));
	port=3306;
	ZeroMemory(dbName,sizeof(dbName));
	ZeroMemory(dbConnParam,sizeof(dbConnParam));

	char strPort[64];

	const char *connString = parseurlhead(url.c_str());
	if (connString){
		int  j=0, k=0;
		size_t i=0;
		while(i < strlen(connString)){
			switch(j)
			{
			case 0:
				if (connString[i] == ':'){user[k] = '\0'; j++; k = 0;}
				else{user[k++] = connString[i];}
				break;
			case 1:
				if (connString[i] == '@'){passwd[k] = '\0'; j++; k = 0;}
				else{passwd[k++] = connString[i];}
				break;
			case 2:
				if (connString[i] == ':'){host[k] = '\0'; j++; k = 0;}
				else{host[k++] = connString[i];}
				break;
			case 3:
				if (connString[i] == '/'){strPort[k] = '\0'; j++; k = 0;}
				else{strPort[k++] = connString[i];}
				break;
			case 4:
				{
					CEasyStrParse parse;
					parse.SetParseStrEx(&connString[i],"\"/","\"\"", '"');
					if ( parse.ParamCount()>0 && parse.ParamCount()<=2 ){
						strcpy_s(dbName,sizeof(dbName)-1,parse[0]);
						if (parse.ParamCount()>1){
							strcpy_s(dbConnParam,sizeof(dbConnParam)-1,parse[1]);
						}
					}else{
						urlerror=true;
					}
					i=strlen(connString);
				}
				break;
			default:
				break;
			}
			i++;
		}
		if (j!=4){urlerror=true;}
		port=atoi(strPort);
		return;
	}
	urlerror=true;
}
//------------------------------------------------------------------------
stAutoSqlClient::stAutoSqlClient(CLD_DBConnPool* pool,unsigned int nhashcode){
	m_Sql=NULL;
	m_pool=NULL;
	if (pool){
		m_pool=pool;
		m_Sql=m_pool->getSqlClient(nhashcode);
	}	
}
//------------------------------------------------------------------------
stAutoSqlClient::stAutoSqlClient(CLD_DBConnPool& pool,unsigned int nhashcode){
	m_pool=&pool;
	m_Sql=m_pool->getSqlClient(nhashcode);
}
//------------------------------------------------------------------------
stAutoSqlClient::~stAutoSqlClient(){
	if (m_pool && m_Sql){
		m_pool->putSqlClient(m_Sql);
	}
	m_Sql=NULL;
	m_pool=NULL;
}
//------------------------------------------------------------------------
long CSqlClientHandle::HandleID_generator = 0;
CIntLock CSqlClientHandle::mHandleIDlock;
//------------------------------------------------------------------------
unsigned int CSqlClientHandle::decodebin(char* pbin,const unsigned int nlen,char* dbuff,const unsigned int nmaxlen)
{
	FUNCTION_BEGIN;
	unsigned int j=0;
	unsigned int i=0;
	while (i<nlen){
		if (j>=nmaxlen){return j;}
		if (i+1<nlen){
			if (pbin[i]=='\\' && (pbin[i+1]=='\\'|| pbin[i+1]=='0')){
				if (pbin[i+1]=='0'){dbuff[j]=0;	}
				else{dbuff[j]='\\';	}
				j++;
				i+=2;
				continue;
			}
		}
		dbuff[j]=pbin[i];
		j++;
		i++;	
	}
	return j;
}
//------------------------------------------------------------------------
unsigned int CSqlClientHandle::encodebin(const char* bindata,const unsigned int nlen,char* dbuff,const unsigned int nmaxlen)
{
	FUNCTION_BEGIN;
	unsigned int j=0;
	for (unsigned int i=0;i<nlen;i++){
		if (j>=nmaxlen){return j;}
		if (bindata[i]==0 || bindata[i]=='\\'){
			if ((j+1)>=nmaxlen){return j;}
			dbuff[j]='\\';
			if (bindata[i]=='\\'){dbuff[j+1]='\\';}
			else{dbuff[j+1]='0';}
			j+=2;
			continue;
		}
		dbuff[j]=bindata[i];
		j++;
	}
	return j;
}
//------------------------------------------------------------------------
unsigned int CSqlClientHandle::getColInfo(const dbCol* column,int& colcount,int& bincolcount,unsigned char* data)
{
	FUNCTION_BEGIN;
	unsigned int retval = 0;
	if(column==NULL) return retval;
	const dbCol *temp;
	temp = column; 
	colcount=0;
	bincolcount=0;
	unsigned int maxoffset=0;
	unsigned int maxoffset_size=0;
	unsigned int ncursize=0;
	while(temp->name){
		if (temp->type==DB_BIN2 || temp->type==DB_ZIP2){
			bincolcount++;
			if (temp->data_addr){ncursize=0;  }
			else if (data)	{ncursize= (sizeof(DWORD)+(*(DWORD*)(data+temp->data_offset)));}
			else {ncursize= sizeof(DWORD);};
		}else{
			if (temp->data_addr){ ncursize=0; }
			else{ ncursize= temp->type_size; }
		}
		retval+=ncursize;
		maxoffset=safe_max(maxoffset,temp->data_offset);
		maxoffset_size=safe_max(maxoffset_size,temp->data_offset+ncursize);
		colcount++;
		temp++; 
	}
	retval=safe_max(retval,maxoffset_size); 
	if(temp->type_size>0){
		retval=safe_max(retval,temp->type_size);
	}
	return retval;
}
//------------------------------------------------------------------------
unsigned int CSqlClientHandle::getColSize(const dbCol* column,unsigned char* data)
{
	int colcount=0;
	int bincolcount=0;
	return getColInfo(column,colcount,bincolcount,data);
}
//------------------------------------------------------------------------
const char *CSqlClientHandle::getTypeString(int type)
{ 
	FUNCTION_BEGIN;
	char *retval = "DB_NONE";

	switch(type)
	{
	case DB_BYTE:
		retval = "DB_BYTE";
		break;  
	case DB_WORD:
		retval = "DB_WORD";
		break;  
	case DB_DWORD:
		retval = "DB_DWORD";
		break;  
	case DB_QWORD:
		retval = "DB_QWORD";
		break;
	case DB_FLOAT:
		retval = "DB_FLOAT";
		break;
	case DB_DOUBLE:
		retval = "DB_DOUBLE";
		break;
	case DB_RAWSTR :
		retval = "DB_RAWSTR";
		break;
	case DB_STR:
		retval = "DB_STR";
		break;
	case DB_DATETIME:
		retval = "DB_DATETIME";
		break;
	case DB_BIN:
		retval = "DB_BIN";
		break;
	case DB_ZIP:
		retval = "DB_ZIP";
		break;
	case DB_BIN2:
		retval = "DB_BIN2";
		break;
	case DB_ZIP2:
		retval = "DB_ZIP2";
		break;
	default:
		retval = "UNKNOW";
		break;
	}
	return retval;
}
//------------------------------------------------------------------------
void CSqlClientHandle::dumpCol(const dbCol *column)
{
	FUNCTION_BEGIN;
	const dbCol *temp;
	temp = column;
	while(temp->name){
		g_logger.info("{  %s,CLD_DBConnPool::%s,%d,%d,NULL},", temp->name ,getTypeString(temp->type),temp->type_size,temp->data_offset);
		temp++;
	}
}
//------------------------------------------------------------------------
bool CSqlClientHandle::initconn()
{
	FUNCTION_BEGIN;
	g_logger.forceLog( zLogger::zFATAL, "[%s]", __FUNC_LINE__  );

	finalconn();
	if (initsqlconn()){
		state=CSqlClientHandle::SQLCLIENT_HANDLE_VALID;
		lifeTime.now();
		getedCount=0;
		my_sql="";
		return true;
	}
	return false;	 
}
//------------------------------------------------------------------------
void CSqlClientHandle::finalconn(){
	FUNCTION_BEGIN;
	finalsqlconn();
	state=CSqlClientHandle::SQLCLIENT_HANDLE_INVALID;
	getedCount=0;
#ifdef _WIN32
	getedThread=(HANDLE)0;
#else
	getedThread=(pthread_t)0;
#endif
	my_sql="";
}
//------------------------------------------------------------------------
bool CSqlClientHandle::initHandle()
{
	FUNCTION_BEGIN;
	if(!initconn()){
		finalconn();
		return false;
	}
	return true;
}
//------------------------------------------------------------------------
void CSqlClientHandle::finalHandle()
{
	finalconn();
}
//------------------------------------------------------------------------
bool CSqlClientHandle::setHandle()
{
	FUNCTION_BEGIN;
	if(getedCount>3600 || lifeTime.elapse()>1800 || !checksqlconn())
	{
		if(!initconn())
		{
			finalconn();
			return false;
		}
	}
	state=CSqlClientHandle::SQLCLIENT_HANDLE_USED;
	getedCount++;
	useTime.now();
#ifdef _WIN32
	getedThread = (HANDLE)GetCurrentThreadId();
#else
	getedThread=pthread_self();
#endif
	return true;
}
//------------------------------------------------------------------------
void CSqlClientHandle::unsetHandle()
{
	FUNCTION_BEGIN;
	state=CSqlClientHandle::SQLCLIENT_HANDLE_VALID;
	useTime.now();
	getedThread=0;
}
//------------------------------------------------------------------------
int CSqlClientHandle::sprintf_exec(const char * pattern, ...)
{
	FUNCTION_BEGIN;
	char szTemp[1024*8]={0};
	va_list ap;	
	va_start(ap, pattern);		
	_vsnprintf(szTemp, (sizeof(szTemp)) - 1, pattern, ap);	
	va_end(ap);	
	return execSql(szTemp,strlen(szTemp));
}
//------------------------------------------------------------------------
int CSqlClientHandle::execSelectSql(const char *sql,const dbCol *column, unsigned char* data,unsigned int maxbuflen,unsigned int sqllen)
{
	if (data!=NULL)
	{
		unsigned char* ptempdata=data;
		return execSelectSqlEx(sql,column,ptempdata,maxbuflen,sqllen);
	}
	return (unsigned int) -1;
}
//------------------------------------------------------------------------
CSqlClientHandle* CLD_DBConnPool::getHandleByHashcode(unsigned int hashcode){
	FUNCTION_BEGIN;
	int nTryCount=0;
	while(true){
		CSqlClientHandle* invalidHandle=NULL;
		INFOLOCK(mlock);
		std::pair<handlesPool::iterator,handlesPool::iterator> hps = handles.equal_range(hashcode);
		for(handlesPool::iterator it = hps.first; it != hps.second; ++it){
			CSqlClientHandle* tempHandle=(*it).second;
			switch(tempHandle->getstate())
			{
			case CSqlClientHandle::SQLCLIENT_HANDLE_INVALID:

				if(invalidHandle==NULL)
					invalidHandle=tempHandle;
				break;
			case CSqlClientHandle::SQLCLIENT_HANDLE_VALID:

				if(tempHandle->setHandle()){
					UNINFOLOCK(mlock);
					return tempHandle;
				}
				break;
			case CSqlClientHandle::SQLCLIENT_HANDLE_USED:

				if(tempHandle->useTime.elapse()>10){

					g_logger.warn("The handle(%u / %d / %d) timeout %lus by thread %u",
						tempHandle->getID(),tempHandle->getedCount,handles.count(hashcode),tempHandle->useTime.elapse(),tempHandle->getedThread);
					g_logger.warn("The handle sql is : %s" , tempHandle->my_sql.c_str());

					tempHandle->useTime.now();
				}
				break;
			}
		}
		stUrlInfo* purl=NULL;
		if(urls.find(hashcode)==urls.end() || urls[hashcode].url.size()==0){
			UNINFOLOCK(mlock);
			return NULL;
		}else{
			purl=&urls[hashcode];
		}
		if(invalidHandle!=NULL){
			if(invalidHandle->initHandle()){
				if(invalidHandle->setHandle()){
					UNINFOLOCK(mlock);
					return invalidHandle;
				}
			}
		}else if((int)handles.count(hashcode) < purl->getmaxhandlebuf()){

			CSqlClientHandle *handle=purl->NewSqlClientHandle();
			if (NULL==handle){
				UNINFOLOCK(mlock);
				g_logger.fatal("not enough memory to allocate handle");
				return handle;
			}
			if(handle->initHandle()){
				handles.insert(handlesPool::value_type(hashcode, handle));
				idmaps.insert(handlesIDMap::value_type(handle->getID(), handle));
				if(handle->setHandle()){
					UNINFOLOCK(mlock);
					return handle;
				}
			}
		}
		UNINFOLOCK(mlock);
		Sleep(50);

		if (purl && nTryCount>((int)(16-purl->getmaxhandlebuf()))) {
			g_logger.warn("[Try:%d:%d] Sleep(50)*%d with getSqlClientByHash",nTryCount,handles.count(hashcode),16-purl->getmaxhandlebuf());
			return NULL;
		};
		nTryCount++;
	}
}
//------------------------------------------------------------------------
CSqlClientHandle* CLD_DBConnPool::getHandleByID(connHandleID handleID){
	FUNCTION_BEGIN;
	INFOLOCK(mlock);
	if (!idmaps.empty()){
		handlesIDMap::iterator it = idmaps.find(handleID);
		if (it != idmaps.end()){
			UNINFOLOCK(mlock);
			return (*it).second;
		}
	}
	UNINFOLOCK(mlock);
	return NULL;
}
//------------------------------------------------------------------------