/**
*	created:		2013-4-7   21:21
*	filename: 		FKDBConnPool
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#define MAX_INFOSTR_SIZE	128
//------------------------------------------------------------------------
#include "../Dump/FKDumpErrorBase.h"
#include <string>
#include "../FKNoncopyable.h"
#include "../FKTime.h"
#include "../FKSyncObjLock.h"
#include "../FKLogger.h"
#include "../zlib/zlib.h"
#include <sstream>
#include <hash_map>
//------------------------------------------------------------------------
#ifdef _DEBUG
#pragma comment( lib,"../FKSvr3Common/Lib/zlib_debug.lib")
#else
#pragma comment( lib,"../FKSvr3Common/Lib/zlib.lib")
#endif
//------------------------------------------------------------------------
using namespace std;
#ifdef _NOT_USE_STLPORT
	using namespace stdext;
#endif
//------------------------------------------------------------------------
#define _DBCOL_NOT_READ_		1
#define _DBCOL_NOT_WRITE_		2
#define _DBCOL_NOT_CHECK_NAME_	4
#define _DBCOL_WRITENAMEBACK_	8
//------------------------------------------------------------------------
struct dbCol
{
	const char *name;	
	WORD type;
	BYTE state;	
	BYTE reserva;
	unsigned int type_size;
	unsigned int data_offset;
	unsigned char * data_addr;
	WORD dbcolsize;
	char szNameBuffer[64];

	static const dbCol* findbyName(const dbCol* pfirstdbcol,const char* szName){
		if (pfirstdbcol && szName){
			while(pfirstdbcol->name){
				if (strcmp(pfirstdbcol->name,szName)==0){
					return pfirstdbcol;
				}
				pfirstdbcol++;
			}
		}
		return NULL;
	}
	__inline bool canRead() const{
		return ((state & _DBCOL_NOT_READ_)==0);
	}
	__inline bool canWrite() const{
		return ((state & _DBCOL_NOT_WRITE_)==0);
	}
	__inline bool checkName() const{
		return ((state & _DBCOL_NOT_CHECK_NAME_)==0);
	}
	bool writeNameBack(const char* szname);
	void clone(const dbCol* pcol);
} ;
//------------------------------------------------------------------------
struct dbColProxy
{
	BYTE buffer[sizeof(dbCol)];
	dbColProxy(){
		ZEROOBJ(this);
	}
	dbCol* getdbCol(){
		return ((dbCol*)&buffer);
	}
};
//------------------------------------------------------------------------
#define _DBCOL_SIZE_OFFSET_FULL_(name,dbtype,type,member,state,dbcolsize)	name,CSqlClientHandle::##dbtype,state,0,sizeof( ((type*)NULL)->member ), (DWORD)(&((type*)NULL)->member),NULL,dbcolsize
#define _DBCOL_SIZE_OFFSET_(name,dbtype,type,member)	_DBCOL_SIZE_OFFSET_FULL_(name,dbtype,type,member,0,sizeof(dbCol))

#define _DBC_SO_		_DBCOL_SIZE_OFFSET_
#define _DBC_SOF_		_DBCOL_SIZE_OFFSET_FULL_

#define _DBCOL_SIZE_ADDR_FULL_(name,dbtype,member,state,dbcolsize)		name,CSqlClientHandle::##dbtype,state,0,sizeof(member),0,(unsigned char *)&(member),dbcolsize
#define _DBCOL_SIZE_ADDR_(name,dbtype,member)		_DBCOL_SIZE_ADDR_FULL_(name,dbtype,member,0,sizeof(dbCol))

#define _DBC_SA_		_DBCOL_SIZE_ADDR_
#define _DBC_SAF_		_DBCOL_SIZE_ADDR_FULL_

#define _DBC_NULL_		NULL,0,0,0,0,0,NULL,sizeof(dbCol)

#define _DBC_NULL_MAXOFFSET_(type)		NULL,0,0,0,sizeof(type),0,NULL,sizeof(dbCol)
#define _DBC_MO_NULL_	_DBC_NULL_MAXOFFSET_
//------------------------------------------------------------------------
class Record;
class FieldSet;
class RecordSet;
//------------------------------------------------------------------------
/**
 * \brief 哈希代码函数类型定义
 * 用户可以根据自己的需要写自己的哈希函数，以便对相对应用户定义的数据库进行操作。
 */
typedef unsigned int(* hashCodeFunc)(const void *data);
/**
* \brief 默认HashCode函数,始终返回0
* \param anyArg 任意参数
* \return 始终返回0 或则 指针指向的第一个字节的值
*/
unsigned int defaultHashCode(const void *anyArg);
/**
 * \brief 连接句柄,用户调用使用,只能从链接池中得到
 */
typedef unsigned int connHandleID;

class CLD_DBConnPool;
class CSqlClientHandle;
//------------------------------------------------------------------------
class CLD_dbColMaker
{
protected:
	std::vector< dbCol > m_dbcols;
public:
	CLD_dbColMaker();
	bool put(const char *n,int t,unsigned int ts,unsigned int ndo,unsigned char * da,BYTE st=0);
	const dbCol* getdbcol();
	void clear(){ m_dbcols.clear(); }
	size_t size(){ return m_dbcols.size(); }
};
//------------------------------------------------------------------------
struct stAutoSqlClient
{
private:
	CSqlClientHandle* m_Sql;
	CLD_DBConnPool* m_pool;
public:
	stAutoSqlClient(unsigned int nhashcode){m_Sql=NULL;m_pool=NULL;}
	stAutoSqlClient(CLD_DBConnPool* pool,unsigned int nhashcode);
	stAutoSqlClient(CLD_DBConnPool& pool,unsigned int nhashcode);
	virtual ~stAutoSqlClient();

	__inline CSqlClientHandle* client() {return m_Sql;}
};
//------------------------------------------------------------------------
class stUrlInfo
{	
public:
	typedef  CSqlClientHandle*(* pCreateSqlClientHandle)(stUrlInfo* ui);

	enum eSqlType
	{
		eTypeError=0,
		
		eMsSql=2,
		eMsMdb=3,
		eMsXls=4,
	};
	struct stSqlTypeInfo
	{
		char* head;
		eSqlType type;
		BYTE maxhandles;
		pCreateSqlClientHandle pnew;
	};
	static stSqlTypeInfo zSqlTypes[];

	const unsigned int hashcode;
	const std::string url;
	const bool supportTransactions;

	stUrlInfo()
		: hashcode(0),url(),urlerror(false),supportTransactions(false),sqltypeinfo(NULL),maxHandleBuf(0) {};

	stUrlInfo(const unsigned int hashcode,const std::string &parurl,const bool supportTransactions,BYTE maxHandle=0)
		: hashcode(hashcode),url(parurl),urlerror(false),supportTransactions(supportTransactions),sqltypeinfo(NULL),maxHandleBuf(maxHandle){
		parseURLString();
		if (maxHandleBuf<=0){
			if (sqltypeinfo){
				maxHandleBuf=sqltypeinfo->maxhandles;
			}else{
				maxHandleBuf=1;
			}
		}
	}
	stUrlInfo(stUrlInfo* ui)
		: hashcode(ui->hashcode),url(ui->url),urlerror(ui->urlerror),supportTransactions(ui->supportTransactions),sqltypeinfo(NULL){
		strcpy_s(host,sizeof(host),ui->gethost());
		strcpy_s(user,sizeof(user),ui->getuser());
		strcpy_s(passwd,sizeof(passwd),ui->getpasswd());
		strcpy_s(dbConnParam,sizeof(dbConnParam),ui->getparam());
		port=ui->getport();
		strcpy_s(dbName,sizeof(dbName),ui->getdbName());
		sqltypeinfo=ui->gettypeinfo();
		maxHandleBuf=ui->getmaxhandlebuf();
		if (maxHandleBuf<=0)
		{
			if (sqltypeinfo)
			{
				maxHandleBuf=sqltypeinfo->maxhandles;
			}
			else
			{
				maxHandleBuf=1;
			}
		}
	}

	CSqlClientHandle* NewSqlClientHandle(){
		if (this!=NULL && sqltypeinfo && sqltypeinfo->pnew){
			return sqltypeinfo->pnew(this);
		}
		return NULL;
	}
	char* gethost()	{return host;}
	char* getuser(){return user;	}
	char* getpasswd(){	return passwd;}
	char* getparam(){	return dbConnParam;}
	unsigned int getport(){return port;}
	char* getdbName(){return dbName;}
	stSqlTypeInfo* gettypeinfo(){return sqltypeinfo;}
	bool geturlerror(){return urlerror;}
	int getmaxhandlebuf(){ return (int)maxHandleBuf;}

	eSqlType gettype(){return sqltypeinfo==NULL?eTypeError:sqltypeinfo->type;}
	
	static stSqlTypeInfo* gettypeinfo(eSqlType type);
protected:
	stSqlTypeInfo* sqltypeinfo;
	char host[MAX_INFOSTR_SIZE*2];
	char user[MAX_INFOSTR_SIZE];
	char passwd[MAX_INFOSTR_SIZE];
	unsigned int port;
	char dbName[MAX_INFOSTR_SIZE*4];
	char dbConnParam[MAX_INFOSTR_SIZE*8];
	bool urlerror;
	BYTE maxHandleBuf;
protected:
	const char* parseurlhead(const char* connstr);
	void parseURLString();
};
//------------------------------------------------------------------------
typedef  stUrlInfo   UrlInfo;
//------------------------------------------------------------------------
class CSqlClientHandle : private zNoncopyable
{
public:
	enum handleState
	{
		SQLCLIENT_HANDLE_INVALID  = 1,   
		SQLCLIENT_HANDLE_VALID    = 2,   
		SQLCLIENT_HANDLE_USED     = 3,   
	};

	enum eSD2U_RC					
	{
		eDataSqlUserRetTypeError=-2,				
		eDataSqlUserRetOther=-1,
		eDataSqlUserRetOk=0,
		eDataSqlUserRetSizeError=1,			
	};

	enum eDB_CLDTYPE
	{
		DB_TYPEBEGIN,

		DB_BYTE,		 
		DB_WORD,		 
		DB_DWORD,		 
		DB_QWORD,		 
		DB_FLOAT,
		DB_DOUBLE,
		DB_RAWSTR,
		DB_STR,			 
		DB_DATETIME,
		DB_BIN,			   
		DB_ZIP,			   
		DB_BIN2,		   
		DB_ZIP2,			 

		DB_TYPEEND,
	};

	struct stenumDataBaseCallback{
		virtual void enumtables(const char* tableName)=0;
		virtual void enumfields(const char* tableName,const char* fieldName,int fieldType)=0;
	};
public:
	HANDLE getedThread;
	zTime useTime;
	std::string my_sql;
protected:
	friend class CLD_DBConnPool;

	stUrlInfo url;
	unsigned int getedCount;
	zTime lifeTime;
	handleState state; 
private:
	const connHandleID id;		
	static long HandleID_generator;
	static CIntLock mHandleIDlock;
protected:
	virtual bool initsqlconn()=0;  
	virtual void finalsqlconn()=0; 
	virtual bool checksqlconn()=0; 

	bool initconn();
	void finalconn();

	virtual bool enumFields(stenumDataBaseCallback& callback,char* tableName)=0;
public:
	virtual bool enumTables(stenumDataBaseCallback& callback)=0;
	virtual bool setTransactions(bool supportTransactions)=0;
	virtual bool commit()=0;
	virtual bool rollback()=0;
    virtual int execSql(const char *sql,unsigned int sqllen = 0)=0;
	virtual int getinsertid()=0;
	virtual int getaffectedrows()=0;
	virtual int execSelectSqlEx(const char *sql,const dbCol *column,unsigned char*& data,unsigned int maxbuflen=0 ,unsigned int sqllen=0 )=0;
	virtual int execInsert(const char *tableName,const dbCol *column,const unsigned char *data,
		const char* noexists_where=NULL,const char* noexists_table=NULL,FieldSet* table=NULL)=0;
        virtual int execDelete(const char *tableName, const char *where)=0;
	virtual int execUpdate(const char *tableName,const dbCol *column,
		const unsigned char *data, const char *where,FieldSet* table=NULL)=0;
	virtual char * escapeStr(const char *src,char *dest,unsigned int srcsize=0)=0;
	virtual std::string& escapeStr(const std::string &src,std::string &dest)=0;
	virtual void updateDatatimeCol(const char* tableName, const char *colName, const char *where=NULL)=0;
	virtual int getCount(const char* tableName, const char *where=NULL)=0;
public:
	CSqlClientHandle(stUrlInfo* ui)
		:id((connHandleID)(HandleID_generator+1)),url(ui), lifeTime(), useTime(){
		FUNCTION_BEGIN;
		INFOLOCK(mHandleIDlock);
		++HandleID_generator;
		*(connHandleID*)(&id)=(connHandleID)HandleID_generator;
		UNINFOLOCK(mHandleIDlock);

		state=CSqlClientHandle::SQLCLIENT_HANDLE_INVALID;
		getedCount=0;
		getedThread=0;
	}
	virtual ~CSqlClientHandle(){
		//不能调用虚函数
		//finalHandle();
	}
	__inline const stUrlInfo::eSqlType getType(){return url.gettype();}
	__inline const connHandleID &getID() const	{return id;}
	__inline const unsigned int hashcode() const{return url.hashcode;}
	__inline bool isSupportTransactions() const	{return url.supportTransactions;}
	__inline const stUrlInfo& geturl(){ return url;}
	__inline handleState getstate(){return state;};
	virtual bool initHandle();
	virtual void finalHandle();

	virtual bool setHandle();
	virtual void unsetHandle();

	int sprintf_exec(const char * pattern, ...);
	int execSelectSql(const char *sql ,const dbCol *column,
		unsigned char* data,unsigned int maxbuflen=0 ,unsigned int sqllen=0);

	static unsigned int decodebin(char* pbin,const unsigned int nlen,
		char* dbuff,const unsigned int nmaxlen);
	static unsigned int encodebin(const char* bindata,const unsigned int nlen,
		char* dbuff,const unsigned int nmaxlen);

	static unsigned int getColSize(const dbCol* column,unsigned char* data=NULL);
	static unsigned int getColInfo(const dbCol* column,int& colcount,int& bincolcount,unsigned char* data=NULL);
	static const char *getTypeString(int type);
	static void dumpCol(const dbCol *column);
};
//------------------------------------------------------------------------
class RTTIClassDescriptor;
//------------------------------------------------------------------------
class  CRttiDbDataLoader
{
public:
	static bool MakedbCol(CLD_dbColMaker& maker,RTTIClassDescriptor* pclass,int srcdatasize);

	template < class srcdata,class _param >
		static int dbLoad( CSqlClientHandle* sqlc,CLD_dbColMaker& maker,
		const char* sqlstr,int nsrcdatacount,_param param,
		int srcdatasize=sizeof(srcdata) )
	{
		RTTIClassDescriptor* pclass=__RTTITypeOfPtr((srcdata*)NULL);
		if ( nsrcdatacount>0 && pclass && pclass->isClass() && MakedbCol(maker,pclass,srcdatasize) && maker.size()>0 )
		{
			maker.put(0,0,srcdatasize,0,0);
			STACK_ALLOCA(char*, srcdatabuffer, ((nsrcdatacount+1)*srcdatasize));
			if (srcdatabuffer)
			{
				int nret=sqlc->execSelectSql(sqlstr,maker.getdbcol(), (unsigned char*)srcdatabuffer,((nsrcdatacount+1)*srcdatasize) );
				if (nret>0)
				{
					for(int i=0;i<nret;i++)
					{
						if (!srcdata::refresh( (srcdata*)&srcdatabuffer[srcdatasize*i],param ))
						{
							return i;
						}
					}
					return nret;
				}
			}
		}
		return 0;
	}

	template < class srcdata,class _param >
		static int dbSave( CSqlClientHandle* sqlc,srcdata* savebuf,CLD_dbColMaker& maker,
		const char* tblname,const char* where,_param param,int srcdatasize=sizeof(srcdata) )
	{
		RTTIClassDescriptor* pclass=__RTTITypeOfPtr((srcdata*)NULL);
		if ( pclass && pclass->isClass() && MakedbCol(maker,pclass) && maker.size()>0 ){
			maker.put(0,0,srcdatasize,0,0);
			if (savebuf){
				int nret=sqlc->execUpdate(tblname, maker.getdbcol(), (unsigned char *) savebuf, where);
				if (nret > 0) {
					return nret;
				}else{
					nret=sqlc->execInsert(tblname, maker.getdbcol(), (unsigned char *) savebuf);
					if (nret > 0) {
						return nret;
					}
				}
			}
		}
		return 0;
	}
};
//------------------------------------------------------------------------
//id相同的会覆盖前面的url
#ifdef _NOT_USE_STLPORT
	typedef stdext::hash_multimap<unsigned int,CSqlClientHandle *> handlesPool;
	typedef stdext::hash_map<unsigned int,stUrlInfo> urlsPool;
	typedef stdext::hash_map<connHandleID,CSqlClientHandle *> handlesIDMap;
#else
	typedef std::hash_multimap<unsigned int,CSqlClientHandle *> handlesPool;
	typedef std::hash_map<unsigned int,stUrlInfo> urlsPool;
	typedef std::hash_map<connHandleID,CSqlClientHandle *> handlesIDMap;
#endif
//------------------------------------------------------------------------
#define GETAUTOSQL(sqlrettype,sql,pool,nhashcode)	stAutoSqlClient ac##sql(pool,nhashcode);sqlrettype sql=dynamic_cast<sqlrettype>(ac##sql.client());
#define GETAUTOSQL_LOOP(sqlrettype,sql,pool,nhashcode,l,s)	stAutoSqlClient ac##sql(pool,nhashcode);sqlrettype sql=dynamic_cast<sqlrettype>(ac##sql.client());
//------------------------------------------------------------------------
class CLD_DBConnPool
{
private:
	CIntLock mlock;
	handlesPool handles;
	urlsPool urls;
	handlesIDMap idmaps;
public:
	CLD_DBConnPool(){
	}

	~CLD_DBConnPool(){
		clear();
	}

	bool putURL(unsigned int hashcode,stUrlInfo::eSqlType type,const char *host,
		unsigned int port,const char *dbname,const char *user,
		const char *pass,bool supportTransactions,BYTE maxhandle){
		FUNCTION_BEGIN;
		stUrlInfo::stSqlTypeInfo* typeinfo=stUrlInfo::gettypeinfo(type);
		if (typeinfo){
			char buffer[1024]={0};
			
			sprintf_s(buffer,sizeof(buffer),"%s%s:%s@%s:%u/%s",typeinfo->head,user,pass,host,port,dbname);
			stUrlInfo ui(hashcode,std::string(buffer),supportTransactions,maxhandle);	
			return putURL(&ui);
		}
		return false;		
	}

	bool putURL(stUrlInfo* ui){
		if (ui){
			 CSqlClientHandle* psqlc=getSqlClient(ui->hashcode);
			if (psqlc!=NULL){
				putSqlClient(psqlc);
				if (ui->url!=psqlc->geturl().url){
					g_logger.error("发现ID重复的数据库连接,增加数据库连接失败！");
					return false;
				}
				return true;
			}
			CSqlClientHandle *handle=ui->NewSqlClientHandle();
			if (handle==NULL)
				return false;
			if(handle->initHandle()){
				INFOLOCK(mlock);
				handles.insert(handlesPool::value_type(ui->hashcode, handle));
				urls.insert(urlsPool::value_type(ui->hashcode, ui));
				idmaps.insert(handlesIDMap::value_type(handle->getID(), handle));
				UNINFOLOCK(mlock);
				return true;
			}else{
				SAFE_DELETE(handle);
				return false;
			}
		}
		return false;
	}

	bool putURL(unsigned int hashcode,const char *url,bool supportTransactions,BYTE maxhandle,bool bokeep=false){
		FUNCTION_BEGIN;
		stUrlInfo ui(hashcode,url,supportTransactions,maxhandle);			
		return putURL(&ui);
	}

	__inline CSqlClientHandle* getSqlClient(unsigned int hashcode){
		return getHandleByHashcode(hashcode);
	}

	void putSqlClient(CSqlClientHandle* handle){
		FUNCTION_BEGIN;
		if(handle!=NULL){
			handle->unsetHandle();
		}
	}

	CSqlClientHandle* getHandleByID(connHandleID handleID);

	void clear(){
		INFOLOCK(mlock);
		if (!handles.empty()){
			for(handlesPool::iterator it = handles.begin(); it != handles.end(); ++it){
				CSqlClientHandle *tempHandle=(*it).second;
				if (tempHandle){	tempHandle->finalHandle();};
				SAFE_DELETE(tempHandle);
			}
		}
		handles.clear();
		urls.clear();
		idmaps.clear();
		UNINFOLOCK(mlock);
	}
protected:
	CSqlClientHandle* getHandleByHashcode(unsigned int hashcode);
};
//------------------------------------------------------------------------