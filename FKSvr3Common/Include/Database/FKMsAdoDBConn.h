/**
*	created:		2013-4-7   21:36
*	filename: 		FKMsAdoDBConn
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKDBConnPool.h"
#include <OleDBErr.h>
#include <Ole2.h>
//------------------------------------------------------------------------
#pragma warning(disable:4239)		
#pragma warning(disable:4146)
//------------------------------------------------------------------------
#import "../../Lib/MsADO15.DLL" rename_namespace("ADOCG") rename("EOF","EndOfFile")
#import "../../Lib/MsADOx.dll"  rename_namespace("ADOX")  rename("EOF","adoxEOF")   
//------------------------------------------------------------------------
typedef	_com_error CComError;
//------------------------------------------------------------------------
class CAdoSqlClientHandle : public CSqlClientHandle
{
protected:
	ADOX::_CatalogPtr		m_pCatalog;
	ADOCG::_RecordsetPtr	m_pRecordset;
	ADOCG::_ConnectionPtr	m_pConnection; 
	int						m_naffectedrows;
	// 执行任意sql语句后初始化  m_qinid=-1 和 m_bocangetqinid=false
	unsigned int			m_qinid;
	bool					m_bocangetqinid;
protected:
	virtual bool initsqlconn();
	virtual void finalsqlconn();
	virtual bool checksqlconn();	

	virtual bool enumFields(stenumDataBaseCallback& callback,char* tableName);
public:
	virtual bool enumTables(stenumDataBaseCallback& callback);

	virtual bool setTransactions(bool supportTransactions);
	virtual bool commit();
	virtual bool rollback();

	virtual int execSql(const char *sql,unsigned int sqllen=0);
	virtual int getinsertid();
	virtual int getaffectedrows(){return m_naffectedrows;};
	virtual int execSelectSqlEx(const char *sql,const dbCol *column,unsigned char*& data,unsigned int maxbuflen=0 ,unsigned int sqllen=0 );
	virtual int execInsert(const char *tableName,const dbCol *column,const unsigned char *data,
		const char* noexists_where=NULL,const char* noexists_table=NULL,FieldSet* table=NULL);
	virtual int execDelete(const char *tableName, const char *where);
	virtual int execUpdate(const char *tableName,const dbCol *column,
		const unsigned char *data, const char *where,FieldSet* table=NULL);

	virtual char * escapeStr(const char *src,char *dest,unsigned int srcsize=0);
	virtual std::string& escapeStr(const std::string &src,std::string &dest);
	virtual void updateDatatimeCol(const char* tableName, const char *colName, const char *where=NULL);
	virtual int getCount(const char* tableName, const char *where=NULL);

	static CSqlClientHandle* newInstance(stUrlInfo* ui){
		return (CSqlClientHandle*)(new CAdoSqlClientHandle(ui));
	}
	CAdoSqlClientHandle(stUrlInfo* ui):CSqlClientHandle(ui){
		m_pConnection=NULL;
		m_pCatalog=NULL;
		m_pRecordset=NULL;
		m_naffectedrows=0;
		m_qinid=(unsigned int)-1;
		m_bocangetqinid=false;
	}
	const char* formatcomerrormsg(CComError & ADOError);

	virtual const char* getconnstr();

	eSD2U_RC sqldata2user(int _etype,_variant_t &sqldata,const unsigned int dwsqlLen,char* userbuf,
		const unsigned int dwuserlen,unsigned int offset);
	eSD2U_RC userdata2sql(int _etype,std::ostringstream& sqlout,const unsigned char * userbuf,
		const unsigned int dwuserlen,unsigned int offset);

	bool makeRecord2Sql(std::ostringstream& query_string,FieldSet* table,Record* rec,bool makename,
		const char* linkstr=" , ");

	bool makeRecord2Sql(std::ostringstream& sqlstr,FieldSet* table,Record* rec,
		const char* linkstr=" , ",const char* namelinkvalue=" = ");

	bool ischs(const char* pstr);
protected:
	bool escapebin2str(const char *src,unsigned int srcsize,char *dest);
};
//------------------------------------------------------------------------
typedef CAdoSqlClientHandle		CAdoMsSqlClient;
//------------------------------------------------------------------------
class CAccessMdbClient : public CAdoSqlClientHandle
{
public:
	CAccessMdbClient(stUrlInfo* ui):CAdoSqlClientHandle(ui){
	}
	static CSqlClientHandle* newInstance(stUrlInfo* ui){
		return (CSqlClientHandle*)(new CAccessMdbClient(ui));
	}
public:
	virtual const char* getconnstr();
};
//------------------------------------------------------------------------