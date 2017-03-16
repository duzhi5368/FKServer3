/**
*	created:		2013-4-7   21:41
*	filename: 		FKMsAdoDBConn
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/Database/FKMsAdoDBConn.h"
#include "../Include/FKStringEx.h"
//------------------------------------------------------------------------
#define _ADO_DISCONN_ERR			0x80004005
#define _ADO_MSSQL_CONN_STR			"Provider=SQLOLEDB.1;Persist Security Info=False;Initial Catalog=%s;Data Source=%s"
#define _ADO_MDB_CONN_STR			"Provider=MSDASQL.1;Persist Security Info=False;Extended Properties=\"DBQ=%s;DRIVER={Microsoft Access Driver (*.mdb)}\""
#define	SELECT_IDENTITY				"SET NOCOUNT ON;SELECT @@IDENTITY AS RETIID;"
#define	SELECT_IDENTITY_COUNTON		"SET NOCOUNT OFF;SELECT @@IDENTITY AS RETIID;"
//------------------------------------------------------------------------
using namespace ADOCG;
//------------------------------------------------------------------------
const char* CAdoSqlClientHandle::formatcomerrormsg(CComError & ADOError)
{
	try
	{
		_bstr_t bstrSource(ADOError.Source());
		_bstr_t bstrDescription(ADOError.Description());
		const char* pret= vformat(/*CAFCDataBase*/"CADO DataBase(%s:%d:[%s : %s]) Error\nCode = %.8x\nCode meaning = %s\nSource = %s\nDescription = %s\nExecSqlStr = %s \n",
			this->url.gethost(),this->url.getport(),this->url.gettypeinfo()->head,this->url.getdbName(),ADOError.Error(),(LPCSTR)ADOError.ErrorMessage(),(LPCSTR)bstrSource,(LPCSTR)bstrDescription,my_sql.c_str());
		if (ADOError.Error()==_ADO_DISCONN_ERR && m_pConnection!=NULL && m_pConnection->GetState()!=adStateClosed){
			finalconn();
		}
		return pret;
	}
	catch (...) {}
	return "";
}
//------------------------------------------------------------------------
const char* CAdoSqlClientHandle::getconnstr(){
	if (url.getport()==0){
		return vformat(_ADO_MSSQL_CONN_STR,url.getdbName(),url.gethost());
	}else{
		return vformat(_ADO_MSSQL_CONN_STR,url.getdbName(),vformat("%s,%d",url.gethost(),url.getport()));
	}
}
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::initsqlconn()
{
	g_logger.forceLog( zLogger::zFATAL, "[Enter initsqlconn]" );

	if (!SUCCEEDED(m_pConnection.CreateInstance(__uuidof(Connection))))
	{
		g_logger.forceLog( zLogger::zFATAL, "%s", __FUNC_LINE__ );

		m_pConnection=NULL;
		return false;
	}
	if (!SUCCEEDED(m_pRecordset.CreateInstance("ADODB.Recordset")))
	{
		g_logger.forceLog( zLogger::zFATAL, "%s", __FUNC_LINE__ );
		m_pRecordset=NULL;
		return false;
	}

	g_logger.forceLog( zLogger::zFATAL, "CreateInstance成功" );

	try
	{
		if (m_pConnection==NULL || m_pRecordset==NULL) return false;

		_bstr_t bstTemp;
		g_logger.forceLog( zLogger::zFATAL, "[%s]", _bstr_t(getconnstr()) );

		return SUCCEEDED(m_pConnection->Open(_bstr_t(getconnstr()),url.getuser(),url.getpasswd(),adModeUnknown));
	}
	catch (CComError & e) 
	{
		g_logger.error("%s error: %s",__FUNC_LINE__,formatcomerrormsg(e));
		g_logger.forceLog( zLogger::zFATAL, "%s error: %s",__FUNC_LINE__,formatcomerrormsg(e));
		return false;
	}
}
//------------------------------------------------------------------------
void CAdoSqlClientHandle::finalsqlconn(){
	try
	{
		if (((void*)m_pRecordset)!=NULL){

			m_pRecordset=NULL;
		}
		if (((void*)m_pCatalog)!=NULL){
			m_pCatalog=NULL;
		}
		if (((void*)m_pConnection)!=NULL){

			m_pConnection=NULL;
		}
	}
	catch (CComError & e) 
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(e));
	}
	return ;
}
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::checksqlconn(){
	try
	{
		if (m_pConnection!=NULL && m_pRecordset!=NULL) return (m_pConnection->GetState()==adStateOpen);
	}
	catch (CComError& e)
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(e));
	}
	return false;
}
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::enumTables(stenumDataBaseCallback& callback)
{
	if(m_pConnection == NULL){return false;}
	try   
	{   
		m_pCatalog = NULL;
		if (SUCCEEDED(m_pCatalog.CreateInstance(__uuidof   (ADOX::Catalog)))){
			m_pCatalog->PutActiveConnection(_variant_t((IDispatch   *)   m_pConnection));   
			long   nTBCount   =   m_pCatalog->Tables->Count;
			for (int i=0;i<nTBCount;i++){
				ADOX::_TablePtr ptbl=m_pCatalog->Tables->GetItem((_variant_t)(short)i);
				if (ptbl!=NULL && ptbl->Name.length()>0){
					callback.enumtables(ptbl->Name);
					enumFields(callback,ptbl->Name);
				}
			}
			m_pCatalog=NULL;
			return true;
		}
	}   
	catch(CComError   &e)   
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(e));
	}   
	m_pCatalog=NULL;
	return false;
}
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::enumFields(stenumDataBaseCallback& callback,char* tableName){
	if(m_pConnection==NULL || m_pCatalog == NULL){return false;}
	try   
	{   
		long   nTBCount   =   m_pCatalog->Tables->Count;  
		for (int i=0;i<nTBCount;i++){
			ADOX::_TablePtr ptbl=m_pCatalog->Tables->GetItem((_variant_t)(short)i);
			if (ptbl!=NULL && ptbl->Name.length()>0 && (strcmp(ptbl->Name,tableName)==0) ){
				long nField=ptbl->Columns->Count;
				for (int i=0;i<nField;i++){
					ADOX::_ColumnPtr pField=ptbl->Columns->Item[(_variant_t)(short)i];
					if (pField!=NULL && pField->Name.length()>0){
						callback.enumfields(tableName,pField->Name,pField->Type);
					}
				}
				return true;
			}
		}
	}
	catch(CComError   &e)   
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(e));
	} 
	return false;
}
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::setTransactions(bool supportTransactions){
	try
	{
		if(m_pConnection == NULL){return false;}
		if (supportTransactions){
			m_pConnection->BeginTrans();
		}else{

		}
		return true;
	}
	catch (CComError & Error)
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(Error));
	}
	return false;
}
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::commit(){
	try
	{
		if(m_pConnection == NULL){return false;}
		return SUCCEEDED(m_pConnection->CommitTrans());
	}
	catch (CComError & Error)
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(Error));
	}
	return false;
}
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::rollback(){
	try
	{
		if(m_pConnection == NULL){return false;}
		return SUCCEEDED(m_pConnection->RollbackTrans());
	}
	catch (CComError & Error)
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(Error));
	}
	return false;
}
//------------------------------------------------------------------------
int CAdoSqlClientHandle::execSql(const char *sql,unsigned int sqllen)
{
	try
	{
		m_naffectedrows=0;
		if(m_pConnection == NULL){return -1;}
		my_sql=sql;
		m_pConnection->CursorLocation=adUseClient;
		VARIANT RecordsAffected;
		VariantInit(&RecordsAffected);
		m_pConnection->Execute(sql,&RecordsAffected,adExecuteNoRecords);
		m_naffectedrows=max(RecordsAffected.lVal,0);
		m_bocangetqinid=false;m_qinid=(unsigned int)-1;
		return 0;
	}
	catch (CComError & Error)
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(Error));
	}
	return -1;
}
//------------------------------------------------------------------------
/// 测试使用标准类型
#define _USE_RIGHT_CAST
//------------------------------------------------------------------------
CAdoSqlClientHandle::eSD2U_RC CAdoSqlClientHandle::sqldata2user(int _etype,_variant_t &sqldata,const unsigned int dwsqlLen,char* userbuf,
																const unsigned int dwuserlen,unsigned int offset)
{
	FUNCTION_BEGIN;
	char *pBuf = NULL;
	switch(_etype)
	{
	case DB_BYTE:
		{
#ifdef _USE_RIGHT_CAST
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(BYTE *)(userbuf+offset)=sqldata.bVal;
			else
				*(BYTE *)(userbuf+offset)=0;
#else
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(BYTE *)(userbuf+offset)=strtol(_bstr_t(sqldata),NULL,10);
			else
				*(BYTE *)(userbuf+offset)=0;
#endif

			offset+=sizeof(BYTE);
		}
		break;
	case DB_WORD:
		{
#ifdef _USE_RIGHT_CAST
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(WORD *)(userbuf+offset)=sqldata.uiVal;
			else
				*(WORD *)(userbuf+offset)=0;
#else
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(WORD *)(userbuf+offset)=strtol(_bstr_t(sqldata),NULL,10);
			else
				*(WORD *)(userbuf+offset)=0;
#endif

			offset+=sizeof(WORD);
		}
		break;
	case DB_DWORD:
		{
#ifdef _USE_RIGHT_CAST
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(DWORD *)(userbuf+offset)=sqldata.ulVal;
			else
				*(DWORD *)(userbuf+offset)=0L;
#else
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(DWORD *)(userbuf+offset)=strtol(_bstr_t(sqldata),NULL,10);
			else
				*(DWORD *)(userbuf+offset)=0L;
#endif
			offset+=sizeof(DWORD);
		}
		break;
	case DB_QWORD:
		{
#ifdef _USE_RIGHT_CAST
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) {
				*(long long *)(userbuf+offset)=strtoll(_bstr_t(sqldata),NULL,10);
			}else{
				*(long long *)(userbuf+offset)=0LL;
			}
#else
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) {
				*(long long *)(userbuf+offset)=strtoll(_bstr_t(sqldata),NULL,10);
			}else{
				*(long long *)(userbuf+offset)=0LL;
			}
#endif
			offset+=sizeof(QWORD);
		}
		break;
	case DB_FLOAT:
		{
#ifdef _USE_RIGHT_CAST
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(float *)(userbuf+offset)=sqldata.fltVal;
			else
				*(float *)(userbuf+offset)=0.0;
#else
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(float *)(userbuf+offset)=sqldata.fltVal;
			else
				*(float *)(userbuf+offset)=0.0;
#endif
			offset+=sizeof(float);
		}
		break;
	case DB_DOUBLE:
		{
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(double *)(userbuf+offset)=sqldata.dblVal;
			else
				*(double *)(userbuf+offset)=0.0;

			offset+=sizeof(double);
		}
		break;

	case DB_RAWSTR:
	case DB_STR:
		{
			if (dwuserlen>0){
				ZeroMemory(userbuf+offset,dwuserlen);

				if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
					strcpy_s(userbuf+offset,dwuserlen,_bstr_t(sqldata));
				offset+=dwuserlen;
			}
		}
		break;
	case DB_DATETIME:
		{
			if (sqldata.vt!=VT_EMPTY && sqldata.vt!=VT_NULL && dwsqlLen>0) 
				*(time_t *)(userbuf+offset)=strtotime(_bstr_t(sqldata));
			else
				*(time_t *)(userbuf+offset)=0;

			offset+=sizeof(time_t);
		}
		break;
	case DB_BIN:
		{
			if (dwuserlen>0){
				ZeroMemory(userbuf+offset,dwuserlen);
				if((sqldata.vt & VT_ARRAY )!=0){SafeArrayAccessData(sqldata.parray,(void **)&pBuf);}
				if (pBuf && dwsqlLen>0){
					bcopy(pBuf,userbuf+offset,dwuserlen>dwsqlLen?dwsqlLen:dwuserlen);
					SafeArrayUnaccessData (sqldata.parray);
				}
				offset+=dwuserlen;
			}
		}
		break;
	case DB_BIN2:
		{
			*((DWORD *)(userbuf+offset))=0;
			DWORD bin2size=sizeof(DWORD);
			if((sqldata.vt & VT_ARRAY )!=0){SafeArrayAccessData(sqldata.parray,(void **)&pBuf);}
			if (pBuf && dwsqlLen>=sizeof(DWORD))
			{
				bin2size=*((DWORD *)pBuf);

				if (dwuserlen!=0 && bin2size>dwuserlen)
				{
					g_logger.error("[ %s ] %s size large( %d > %d )...",__FUNC_LINE__ , getTypeString(_etype),bin2size ,dwuserlen );
					offset+=sizeof(DWORD);

					SafeArrayUnaccessData (sqldata.parray);
					return eDataSqlUserRetSizeError;
				}
				bin2size+=sizeof(DWORD);

				int truebinsize=(bin2size>dwsqlLen?dwsqlLen:bin2size);

				ZeroMemory(userbuf+offset,truebinsize);
				bcopy(pBuf,userbuf+offset,truebinsize);
				*((DWORD *)(userbuf+offset))=truebinsize-sizeof(DWORD);
				SafeArrayUnaccessData (sqldata.parray);
			}
			offset+=bin2size;
		}
		break;
	case DB_ZIP:
	case DB_ZIP2:
		{
			if (!(dwuserlen==0 && _etype==DB_ZIP))
			{
				DWORD bin2size=0;
				if (_etype==DB_ZIP){ZeroMemory(userbuf+offset,dwuserlen);bin2size=dwuserlen;}
				else	{*((DWORD *)(userbuf+offset))=0;bin2size=sizeof(DWORD);}
				if((sqldata.vt & VT_ARRAY )!=0){SafeArrayAccessData(sqldata.parray,(void **)&pBuf);}
				if (pBuf && dwsqlLen>0)
				{
					int retcode;
					uLong destLen = dwuserlen;
					if (_etype==DB_ZIP2 && dwuserlen==0){destLen=0x7fffffff;}

					retcode = uncompress((Bytef *)userbuf+offset, &destLen, (Bytef *)pBuf, dwsqlLen);
					switch(retcode) 
					{
					case Z_OK:
						{
							if (_etype==DB_ZIP2 && destLen>=sizeof(DWORD))
							{
								bin2size=*((DWORD *)(userbuf+offset));

								if (dwuserlen!=0 && bin2size>dwuserlen)
								{
									g_logger.error("[ %s ] %s size large( %d > %d )...",__FUNC_LINE__ , getTypeString(_etype),bin2size ,dwuserlen );
									*((DWORD *)(userbuf+offset))=0;
									offset+=sizeof(DWORD);

									SafeArrayUnaccessData (sqldata.parray);
									return eDataSqlUserRetSizeError;
								}
								bin2size+=sizeof(DWORD);
							}
							if (bin2size!=destLen)
							{
								g_logger.error("[ %s ] %s uncompress data error( %d<>%d )...",__FUNC_LINE__ , getTypeString(_etype),bin2size,destLen );	

								if (_etype==DB_ZIP){ZeroMemory(userbuf+offset,dwuserlen);bin2size=dwuserlen;}
								else	{*((DWORD *)(userbuf+offset))=0;bin2size=sizeof(DWORD);}
								offset+=bin2size;
								SafeArrayUnaccessData (sqldata.parray);
								return eDataSqlUserRetSizeError;					
							}
						}
						break;
					case Z_MEM_ERROR:
					case Z_BUF_ERROR:
					case Z_DATA_ERROR:
						{

							ZeroMemory(userbuf+offset,bin2size);
							offset+=bin2size;
							SafeArrayUnaccessData (sqldata.parray);
							return  eDataSqlUserRetSizeError;
						}
					}
					SafeArrayUnaccessData (sqldata.parray);
				}
				offset+=bin2size;
			}
		} 
		break;
	default:
		{
			offset+=dwuserlen;
			g_logger.error("[ %s ] %s invalid type...",__FUNC_LINE__ , getTypeString(_etype));
			return eDataSqlUserRetTypeError;
		}
	}       
	return eDataSqlUserRetOk;
}
//------------------------------------------------------------------------
struct  stEscape{
	char* src;
	int nsrclen;
	char* dest;
	int ndestlen;
};
//------------------------------------------------------------------------
const stEscape stc_escape[]={
	{"\0",1,"CHAR(0)",7},
	{"\'",1,"CHAR(39)",8},
};
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::ischs(const char* pstr){
	BYTE b0=(BYTE)pstr[0],b1=(BYTE)pstr[1];
	if (b0>=0x81 && b0<=0xfe && b1>=0x40 && b1<=0xfe){

		return true;
	}else if (b0>=0xa1 && b0<=0xF7 && b1>=0xA0 && b1<=0xFE){

		return true;
	}else if (b0>=0x81 && b0<=0xfe && ((b1>=0x40 && b1<=0x7E)||(b1>=0xA1 && b1<=0xFE)) ){

		return true;
	}
	return false;

}
//------------------------------------------------------------------------
char * CAdoSqlClientHandle::escapeStr(const char *src,char *dest,unsigned int srcsize)
{
	FUNCTION_BEGIN;
	if (srcsize<=0){srcsize=strlen(src);}
	if(dest){dest[0]=0;}
	if(src==NULL || dest==NULL || srcsize==0) return dest;

	dest[0]=0;
	char* bakdest=dest;
	bool boescape=false;
	bool bolastescape=false;
	unsigned int i=0;
	while (i<srcsize){
		boescape=false;
		if ((BYTE)src[i]>=0x80){
			char sztext[128]={0};
			int nsrclen=2;
			if (i<(srcsize-1))
			{
				if (bolastescape)
				{
					strcat(dest,"+\'");	dest+=2; dest[0]=0;
				}
				dest[0]=src[i];	
				dest[1]=src[i+1];
				dest+=nsrclen;
				i+=nsrclen;
				dest[0]=0;
				bolastescape=false;
				continue;
			}
			else
			{
				sprintf_s(sztext,sizeof(sztext),"CHAR(%u)\0",(BYTE)src[i]);
				nsrclen=1;
			}
			if (!bolastescape){
				strcat(dest,"\'+");	dest+=2;dest[0]=0;
			}else{
				dest[0]='+';dest++;dest[0]=0;
			}
			strcat(dest,sztext);
			dest+=strlen(sztext);
			dest[0]=0;
			if ((i+nsrclen)>=srcsize){
				strcat(dest,"+\'");dest+=2;dest[0]=0;
			}
			boescape=true;
			i+=nsrclen;
		}else{
			for (int ii=0;ii<count_of(stc_escape);ii++){
				if (strncmp(&src[i],stc_escape[ii].src,stc_escape[ii].nsrclen)==0){
					if (!bolastescape){
						strcat(dest,"\'+");	dest+=2;dest[0]=0;
					}else{
						dest[0]='+';dest++;dest[0]=0;
					}
					strcat(dest,stc_escape[ii].dest);
					dest+=stc_escape[ii].ndestlen;
					dest[0]=0;
					if ((i+stc_escape[ii].nsrclen)>=srcsize){
						strcat(dest,"+\'");dest+=2;dest[0]=0;
					}
					boescape=true;
					i+=stc_escape[ii].nsrclen;
					break;
				}
			}
		}
		if (!boescape){
			if (bolastescape){
				strcat(dest,"+\'");	dest+=2; dest[0]=0;
			}
			dest[0]=src[i];	dest++;	dest[0]=0;
			bolastescape=false;
			i++;
		}else{
			bolastescape=true;
		}
	}
	return bakdest;
}
//------------------------------------------------------------------------
std::string& CAdoSqlClientHandle::escapeStr(const std::string &src,std::string &dest)
{
	ZSTACK_ALLOCA(char* ,buff,16 * src.length());

	escapeStr(src.c_str(),buff,src.length());
	dest = buff;
	return dest;
}
//------------------------------------------------------------------------
#define	CONVERT_BIN_BEGIN		"CONVERT(varbinary(%d),"
#define	CONVERT_BIN_END			")"
//------------------------------------------------------------------------
bool CAdoSqlClientHandle::escapebin2str(const char *src,unsigned int srcsize,char *dest){
	if (src==NULL || dest==NULL){return false;}
	dest[0]=0;
	if(srcsize==0) { return true; }

	int nmaxhex=safe_min<unsigned int>(safe_max<unsigned int>((srcsize / 16),1),50);
	unsigned int i=0;
	int nhex=0;
	while (i<srcsize){

		if ((BYTE)src[i]>=0x80){
			i++;if (!(i<(srcsize-1))){nhex++;}
		}else{
			for (int ii=0;ii<count_of(stc_escape);ii++){
				if (strncmp(&src[i],stc_escape[ii].src,stc_escape[ii].nsrclen)==0){
					i=i+(stc_escape[ii].nsrclen-1);
					nhex++;
					break;
				}
			}
		}
		if (nhex>nmaxhex){break;}
		i++;
	}
	if (nhex>nmaxhex){
		sprintf_s(dest,sizeof(CONVERT_BIN_BEGIN)+16,CONVERT_BIN_BEGIN,srcsize);dest+=strlen(dest);dest[0]=0;
		strcat(dest,"0x");dest+=2;dest[0]=0;
		for (i=0;i<srcsize;i++){
			sprintf_s(dest,3,"%.2x",(BYTE)src[i]);dest+=2;dest[0]=0;
		}
		strcat(dest,CONVERT_BIN_END);dest+=(sizeof(CONVERT_BIN_END)-1);dest[0]=0;
		return false;
	}else{
		sprintf_s(dest,sizeof(CONVERT_BIN_BEGIN)+16,CONVERT_BIN_BEGIN,srcsize);dest+=strlen(dest);dest[0]=0;
		dest[0]='\'';dest++;dest[0]=0;
		escapeStr(src,dest,srcsize);
		strcat(dest,"\'");
		strcat(dest,CONVERT_BIN_END);
		return true;
	}
}
//------------------------------------------------------------------------
CAdoSqlClientHandle::eSD2U_RC CAdoSqlClientHandle::userdata2sql(int _etype,std::ostringstream& sqlout,const unsigned char * userbuf,
																const unsigned int dwuserlen,unsigned int offset)
{
	FUNCTION_BEGIN;
	switch(_etype)
	{
	case DB_BYTE:
		{
			sqlout << ((int)(0x00ff & (*(BYTE *)(userbuf+offset))));
			offset+=sizeof(BYTE);
		}
		break;
	case DB_WORD:
		{
			sqlout << ((int)(*(WORD *)(userbuf+offset)));
			offset+=sizeof(WORD);
		}
		break;
	case DB_DWORD:
		{
			sqlout << ((int)(*(DWORD *)(userbuf+offset)));
			offset+=sizeof(DWORD);
		}
		break;
	case DB_QWORD:
		{
			sqlout << *(__int64 *)(userbuf+offset);
			offset+=sizeof(QWORD);
		}
		break;
	case DB_FLOAT:
		{
			sqlout << *(float *)(userbuf+offset);
			offset+=sizeof(float);
		}
		break;
	case DB_DOUBLE:
		{
			sqlout << *(double *)(userbuf+offset);
			offset+=sizeof(double);
		}
		break;
	case DB_RAWSTR:
		{
			unsigned int len=strlen((char *)(userbuf+offset));
			len=safe_min(len,dwuserlen);

			if (dwuserlen>0){
				ZSTACK_ALLOCA(char* ,strData,len * 12 + 128);
				escapeStr((char *)(userbuf+offset),strData,len);
				sqlout << strData ;
			}else{
				sqlout <<"\'\'";
			}
			offset+=dwuserlen;
		}
		break;
	case DB_STR:
		{
			unsigned int len=strlen((char *)(userbuf+offset));
			len=safe_min(len,dwuserlen);

			if (dwuserlen>0){
				ZSTACK_ALLOCA(char* ,strData,len * 12 + 128);
				escapeStr((char *)(userbuf+offset),strData,len);
				sqlout << "\'" << strData << "\'";
			}else{
				sqlout <<"\'\'";
			}
			offset+=dwuserlen;
		}
		break;
	case DB_DATETIME:
		{
			sqlout << "\'" << timetostr(*(time_t *)(userbuf+offset)) << "\'";
			offset+=sizeof(time_t);
		}
		break;
	case DB_BIN:
		{

			if (dwuserlen>0)
			{
				ZSTACK_ALLOCA(char* ,strData,dwuserlen * 12 + 128);
				escapebin2str((char *)(userbuf+offset),dwuserlen,strData);
				sqlout << strData;
			}else{
				sqlout << "CONVERT(varbinary(0),\'\')";
			}
			offset+=dwuserlen;
		}
		break;
	case DB_BIN2:
		{
			unsigned int size = *((DWORD *)(userbuf+offset));

			if (dwuserlen!=0 && size>dwuserlen)
			{
				g_logger.error("[ %s ] %s size large( %d > %d )...",__FUNC_LINE__ , getTypeString(_etype),size ,dwuserlen );
				size += sizeof(DWORD);
				offset+=size;
				sqlout << "CONVERT(varbinary(0),\'\')";
				return eDataSqlUserRetSizeError;
			}
			size += sizeof(DWORD);


			ZSTACK_ALLOCA(char* ,strData,size * 12 + 128);
			escapebin2str((char *)(userbuf+offset),size,strData);
			sqlout << strData ;

			offset+=size;
		}
		break;
	case DB_ZIP2:
	case DB_ZIP:
		{
			if (!(dwuserlen==0 && _etype==DB_ZIP))
			{
				unsigned int size=0;
				if (_etype==DB_ZIP){

					size=dwuserlen;
				}else{
					unsigned int size = *((DWORD *)(userbuf+offset));

					if (dwuserlen!=0 && size>dwuserlen)
					{
						g_logger.error("[ %s ] %s size large( %d > %d )...",__FUNC_LINE__ , getTypeString(_etype),size ,dwuserlen );
						size += sizeof(DWORD);
						offset+=size;
						sqlout << "CONVERT(varbinary(0),\'\')";
						return eDataSqlUserRetSizeError;
					}
					size += sizeof(DWORD);
				}
				uLongf destLen = size * ((120 / 100)+1) + 32;
				ZSTACK_ALLOCA(Bytef* ,destBuffer,destLen * sizeof(Bytef));
				int retcode = compress(destBuffer, &destLen, (Bytef *)(userbuf+offset),size);
				switch(retcode)
				{
				case Z_OK:
					{

						ZSTACK_ALLOCA(char* ,strData,destLen * 12 + 128);
						escapebin2str((char *)destBuffer,destLen,strData);
						sqlout << strData;
					}
					break; 
				case Z_MEM_ERROR:
				case Z_BUF_ERROR:
					{
						g_logger.error("Not enough memory, NULL value instead.... %s",__FUNC_LINE__);
						sqlout << "CONVERT(varbinary(0),\'\')";
						offset+=size;
						return eDataSqlUserRetSizeError;
					}
				}
				offset+=size;
			}
		}
		break;
	default:
		{
			offset+=dwuserlen;
			g_logger.error("[ %s ] %s invalid type...",__FUNC_LINE__ , getTypeString(_etype));
			return eDataSqlUserRetTypeError;
		}
	}
	return eDataSqlUserRetOk;
}
//------------------------------------------------------------------------
int CAdoSqlClientHandle::execSelectSqlEx(const char *sql,const dbCol *column,unsigned char*& data,unsigned int maxbuflen,unsigned int sqllen){
	try
	{
		if (m_pRecordset==NULL || m_pConnection==NULL){return -1;}
		my_sql=sql;
		if (m_pRecordset->GetState()!=adStateClosed) m_pRecordset->Close();
		m_pRecordset->CursorLocation=adUseClient;
		if (SUCCEEDED(m_pRecordset->Open(sql,((IDispatch *)(m_pConnection)),adOpenStatic,adLockOptimistic,adCmdUnknown))){
			m_bocangetqinid=false;m_qinid=(unsigned int)-1;
			int retCount=m_pRecordset->RecordCount;
			if (retCount>0){

				int ncol,nbincol;
				unsigned int nrowsize=CSqlClientHandle::getColInfo(column,ncol,nbincol);

				bool boIsCreate=(data==NULL);
				unsigned int nnowsize=0;
				unsigned int ndatasize=0;
				unsigned int count=0;
				unsigned char * olddata=NULL;
				unsigned char *tempData=data;
				unsigned int offset=0;

				while (m_pRecordset->EndOfFile!=VARIANT_TRUE){
					if (boIsCreate)	{
						unsigned int alllength=0;
						for (int i=0;i<m_pRecordset->Fields->Count;i++){alllength+=safe_max<unsigned int>(m_pRecordset->Fields->Item[(_variant_t)(short)i]->ActualSize+1,8);}
						if (ndatasize<alllength || tempData==NULL || data==NULL){
							if (nbincol>0){

								ndatasize=ROUNDNUM2((safe_max(alllength,nrowsize)<<2)*(ROUNDNUM2(retCount,4)),512);
							}else{
								ndatasize=ROUNDNUM2(safe_max(alllength,nrowsize)*(ROUNDNUM2(retCount,4)),512);
							}
							data=new unsigned char[ndatasize];
							if (data){
								ZeroMemory(data,ndatasize);
								if (olddata){
									if (nnowsize>0){memcpy(data,olddata,nnowsize);}
									delete[] olddata;
								}
								olddata=data;
								tempData=data+nnowsize;
								ndatasize -= nnowsize;
							}else{
								if (olddata){delete[] olddata;};
								m_pRecordset->Close();
								return -1;
							}
						}
					}
					const dbCol *temp=column;
					int retfieldcount=m_pRecordset->Fields->Count;
					int ifieldidx=0;
					while ( temp->name){
						if (ifieldidx>=retfieldcount){ break;}
						if (temp->canRead()){
							if (temp->name[0]!=0 || !temp->checkName()){
								FieldPtr pFld =NULL;
								if (!temp->checkName()){
									try{
										pFld = m_pRecordset->Fields->GetItem((_variant_t)(short)ifieldidx);
									}catch (CComError & Error) {
										g_logger.error("%s error: 在对应集合中，未找列 index:%d [%s] : %s ",__FUNC_LINE__,ifieldidx,my_sql.c_str(),formatcomerrormsg(Error));
										return -1;
									}
									try{
										if (pFld){	((dbCol *)temp)->writeNameBack(pFld->Name);	}
									}catch (...) {
										g_logger.error("%s error: 在对应集合中，字段名 %s 无法回写 [%s]",__FUNC_LINE__,pFld->Name,my_sql.c_str());
										return -1;
									}
								}else{
									try{
										pFld = m_pRecordset->Fields->GetItem(temp->name);
									}catch (CComError & Error) {
										g_logger.error("%s error: 在对应集合中，未找到字段 %s [%s] : %s ",__FUNC_LINE__,temp->name,my_sql.c_str(),formatcomerrormsg(Error));
										return -1;
									}
								}
								if (pFld==NULL){
									m_pRecordset->Close();
									if (data && boIsCreate){SAFE_DELETEARRAY(data);}
									return -1;
								}						
								if (sqldata2user(temp->type,pFld->Value,pFld->ActualSize,temp->data_addr==NULL?(char *)tempData:(char *)temp->data_addr,temp->type_size,temp->data_offset)<0){
									m_pRecordset->Close();
									if (data && boIsCreate){SAFE_DELETEARRAY(data);}
									return -1;
								}
								ifieldidx++;
							}else{
								_variant_t varempty;
								VariantInit(&varempty);
								if (sqldata2user(temp->type,varempty,0,temp->data_addr==NULL?(char *)tempData:(char *)temp->data_addr,temp->type_size,temp->data_offset)<0){
									m_pRecordset->Close();
									if (data && boIsCreate){SAFE_DELETEARRAY(data);}							
									return -1;
								}
							}
						}else if (temp->name[0]!=0){
							//ifieldidx++;
						}
						temp++;
					}
					offset=offset+getColSize(column,tempData);
					if (boIsCreate){ndatasize=(ndatasize>offset?ndatasize-offset:0);nnowsize+=offset;}
					tempData+=offset;
					offset=0;
					count++;
					m_pRecordset->MoveNext();
				}
				m_pRecordset->Close();
				if (data && count<=0 && boIsCreate){SAFE_DELETEARRAY(data);}
				return count;
			}else{
				return 0;
			}
		}
	}
	catch (CComError & Error) 
	{
		g_logger.error("%s error: %s ",__FUNC_LINE__,formatcomerrormsg(Error));
	}
	return -1;
}
//------------------------------------------------------------------------
void CAdoSqlClientHandle::updateDatatimeCol(const char* tableName, const char *colName, const char *where)
{
	FUNCTION_BEGIN;
	if (tableName	 && tableName[0]!=0	 && colName	&& colName[0]!=0)
	{
		std::string sql = "UPDATE ";
		sql += tableName;
		sql += " SET ";
		sql += colName;
		sql += " = GETDATE() ";
		if (where && where[0]!=0){sql += where;}
		execSql(sql.c_str(), sql.length());
	}
}
//------------------------------------------------------------------------
int CAdoSqlClientHandle::getCount(const char* tableName, const char *where)
{
	FUNCTION_BEGIN;
	if (!tableName){return -1;};
	DWORD count=0;
	dbCol mycountCol_define[]=
	{
		{ _DBC_SA_("RETCOUNT",DB_DWORD,count) },
		{_DBC_NULL_}
	};
	unsigned int retval = 0;
	std::string sql;
	sql="SELECT TOP 1 COUNT(*) AS RETCOUNT FROM ";
	sql+=tableName;
	if (where){	sql+=" WHERE ";sql+=where;}
	unsigned int i = execSelectSql(sql.c_str(),mycountCol_define , (unsigned char *)(&count),sizeof(count),sql.length());
	if (1 == i)
		retval = count;
	else
		retval=(unsigned int)-1;
	return retval;
}
//------------------------------------------------------------------------
int CAdoSqlClientHandle::execInsert(const char *tableName,const dbCol *column,const unsigned char *data,const char* noexists_where,const char* noexists_table,FieldSet* table)
{
	FUNCTION_BEGIN;
	const dbCol *temp;
	if(tableName == NULL || data==NULL || column==NULL || m_pConnection==NULL)
	{
		g_logger.error("null pointer error. ---- %s",__FUNC_LINE__);
		return (unsigned int)-1;
	}

	std::ostringstream strSql;

	strSql << "INSERT INTO ";
	strSql << tableName;
	strSql << " ( ";
	temp = column; 
	bool first=true;
	while(temp->name) 
	{
		if(temp->name[0]!=0)
		{       
			if(first)
				first=false;
			else
				strSql << ", ";
			strSql << temp->name;
		}       
		temp++; 
	}

	if (!noexists_where){	strSql << ") VALUES( ";	}
	else{strSql << ") SELECT ";}

	first=true;
	temp = column; 

	while(temp->name) 
	{
		if(temp->name[0]!=0&& temp->canWrite())
		{       
			if(first)
				first=false;
			else
				strSql << ", ";
			if (userdata2sql(temp->type,strSql,temp->data_addr==NULL?data:temp->data_addr,temp->type_size,temp->data_offset)<0)
			{
				return (unsigned int)-1;
			}
		}       
		temp++;
	}   
	if (!noexists_where)
	{
		strSql << ")";
	}
	else
	{
		strSql << " where not exists (select * from ";
		if (noexists_table){strSql<< noexists_table;}
		else{strSql << tableName; }
		strSql<< " where ";
		strSql<<noexists_where<< ")";
	}	

	if(0 == execSql(strSql.str().c_str(),strSql.str().length())){
		return max(0,getaffectedrows());
	}else{return (unsigned int)-1;}
}
//------------------------------------------------------------------------
int CAdoSqlClientHandle::getinsertid(){
	if (m_bocangetqinid){
		return m_qinid;
	}
	if (getaffectedrows()>=0)
	{
		dbCol myinto_define[]=
		{
			{ _DBC_SA_("RETIID",DB_DWORD,m_qinid) },
			{_DBC_NULL_}
		};

		unsigned int nrcode = execSelectSql(SELECT_IDENTITY,myinto_define,(unsigned char *)(&m_qinid),sizeof(m_qinid),sizeof(SELECT_IDENTITY)-1);
		m_bocangetqinid=true;
		if (nrcode<=0 || m_qinid<=0){	m_bocangetqinid=false;m_qinid=(unsigned int)-1;}

		unsigned int nrcode1 = execSelectSql(SELECT_IDENTITY_COUNTON,myinto_define,(unsigned char *)(&m_qinid),sizeof(m_qinid),sizeof(SELECT_IDENTITY_COUNTON)-1);
		m_bocangetqinid = false;
	}
	return m_qinid;
}
//------------------------------------------------------------------------
int CAdoSqlClientHandle::execDelete(const char *tableName, const char *where)
{
	FUNCTION_BEGIN;
	if(tableName==NULL || m_pConnection==NULL){
		g_logger.error("null pointer error. ---- %s",__FUNC_LINE__);
		return (unsigned int)-1;
	}
	std::string strSql="DELETE FROM ";
	strSql+=tableName;
	if(where){
		strSql+=" WHERE ";
		strSql+=where;
	}

	if (0 == execSql(strSql.c_str(),strSql.length())){
		return getaffectedrows();
	}else{ return (unsigned int)-1;}
}
//------------------------------------------------------------------------
int CAdoSqlClientHandle::execUpdate(const char *tableName,const dbCol *column,const unsigned char *data, const char *where,FieldSet* table)
{
	FUNCTION_BEGIN;
	std::ostringstream out_sql;
	const dbCol *temp;
	if(tableName==NULL || column==NULL || data==NULL || m_pConnection==NULL){
		g_logger.error("null pointer error. ---- %s",__FUNC_LINE__);
		return (unsigned int)-1;
	}

	out_sql << "UPDATE " << tableName << " SET ";
	temp = column;
	bool first=true;

	while(temp->name)
	{
		if(temp->name[0]!=0&& temp->canWrite())
		{
			if(first)
				first=false;
			else
				out_sql << ", ";
			out_sql << temp->name << "=";
			if (userdata2sql(temp->type,out_sql,temp->data_addr==NULL?data:temp->data_addr,temp->type_size,temp->data_offset)<0)
			{
				return (unsigned int)-1;
			}
		}       
		temp++;
	}
	if(where!=NULL){out_sql << " WHERE " << where;}

	if (0 == execSql(out_sql.str().c_str(),out_sql.str().length()))
	{
		return max(0,getaffectedrows());
	}
	else
		return (unsigned int)-1;
}
//------------------------------------------------------------------------
const char* CAccessMdbClient::getconnstr(){
	return vformat(_ADO_MDB_CONN_STR,url.getdbName());
}
//------------------------------------------------------------------------