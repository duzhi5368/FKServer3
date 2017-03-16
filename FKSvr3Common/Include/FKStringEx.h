/**
*	created:		2013-3-22   22:53
*	filename: 		FKStringEx
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKBaseDefine.h"
#include <string>
#include <algorithm>
#include <cctype>
#include "STLTemplate/FKSyncList.h"
#include <hash_map>
#include "FKThread.h"
#include "Dump/FKDumpErrorBase.h"
#include "STLTemplate/FKLookasideAlloc.h"
#include "Endec/FKCrc32.h"
//------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------
#ifdef _NOT_USE_STLPORT
using namespace stdext;
#endif
//------------------------------------------------------------------------
class CEasyStrParse
{
protected:
	std::vector< char* >  m_pszParams;
	char* m_pStart;		
	char* m_pSrcStart;	
	char* m_pch;	
	char* m_pBAch;	
	char* m_psrc;	
	size_t m_npsrc_maxlen;	
	bool m_bokeepnil;
	char m_zychr;	

	static const char NullChar[4];
protected:
	bool SetCh(char* psz);
	bool SetBAch(char* psz);
	void ParseParam(int nCount);
	char* Param(int i);
public:
	CEasyStrParse();
	virtual ~CEasyStrParse();
public:
	bool SetParseStrEx(const char* psz,char* pszCh,char* pszBAch=NULL,char zychr='\0',bool keepnil=false);
	bool SetParseStr(char* psz,char* pszCh,char* pszBAch=NULL,char zychr='\0',bool keepnil=false);
	int ParamCount(){ Param(0x7ffffff0);return (int)m_pszParams.size(); }
	char * operator []  ( int  num  ){return Param(num);}
};
//------------------------------------------------------------------------
#define MAX_TLS_LOOPCHARBUFFER				1024*128

extern _TH_VAR_ARRAY(char,tls_loop_charbuffer,MAX_TLS_LOOPCHARBUFFER+1);

#define _GET_TLS_LOOPCHARBUF(x)				char* ptlsbuf=(char*)_TH_VAR_PTR(tls_loop_charbuffer);		\
	char** ptlslast= (char**)(ptlsbuf);		\
	ptlsbuf+=sizeof(char**);		\
	if (!(*ptlslast>=ptlsbuf && *ptlslast<=(ptlsbuf+MAX_TLS_LOOPCHARBUFFER-8)))	\
	{*ptlslast=ptlsbuf;}		\
	int ntlslen=strlen(*ptlslast)+(*ptlslast-ptlsbuf);	\
	if (ntlslen>(MAX_TLS_LOOPCHARBUFFER-1024-128) || ((ntlslen+(x))>(MAX_TLS_LOOPCHARBUFFER-32)) ){		\
	ntlslen=_TH_VAR_COUNT_OF(tls_loop_charbuffer)-sizeof(char**);\
	ZeroMemory(ptlsbuf,MAX_PATH);*ptlslast=(ptlsbuf+4);ntlslen -= 16;}\
											else {*ptlslast=ptlsbuf+ntlslen+4;		\
											ntlslen=_TH_VAR_COUNT_OF(tls_loop_charbuffer)-ntlslen-16;}	\
											ptlsbuf=*ptlslast;	ptlsbuf[0]=0;	\

#define _GET_TH_LOOPCHARBUF(x,y)	_GET_TLS_LOOPCHARBUF(x)
#define __BUILD_DATE_TIME__			__ZDATE__
#define __ZDATE__					endate2date(__DATE__)
//------------------------------------------------------------------------
const char* vformat(const char* pszFmt,...);
const char* itochina(int i);
char* endate2date(char* src);
const char* sec2str(int iSecond);
char* timetostr(time_t time1, char *szTime=NULL,int nLen=0,const char* sformat="%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d");
time_t strtotime(const char * szTime,const char* sformat="%4d-%2d-%2d %2d:%2d:%2d");
void str_replace(const char* pszSrc,char* pszDst,int ndlen ,const char* pszOld,const char* pszNew);
void str_replace(char* pszSrc,const char* pszOld,const char* pszNew);
void str_replace(char* pszSrc,char cOld,char cNew);
void str_replace(char* pszSrc,wchar_t cOld,wchar_t cNew);
void str_del(char* pszSrc,char c);
void str_del(char* pszSrc,wchar_t c);
void str_del(char* pszSrc,int c);
std::string str_Replace(std::string &strText,const char *pszSrcText,const char *pszDestText);
//------------------------------------------------------------------------
const char* extractfilename(const char* pname);
const char* extractfiledir(const char* pname);
const char* extractfilepath(const char* pname);
const char* extractfileext(const char* pname);
const char* changefileext(const char* pname,const char* pchg);
const char* addfileext(const char* pname,const char* padd,char bchg='_');
const char* extractfiletitle(const char* pname);
//------------------------------------------------------------------------
char* getwithtimefilename(char* srcpch);
void replacelashPath(char *str,bool front=true);
//------------------------------------------------------------------------
#define  replaceBacklashPath(x)		replacelashPath(x,false)
#define  replaceFrontlashPath(x)	replacelashPath(x,true)
//------------------------------------------------------------------------
void str_del_sp(char* pszSrc, char c);
void str_del_sp_full(char* pszSrc, char c);
BOOL str_isempty(const char* psz);
char* str_cpy(char* pszDest,const char* pszSrc,int len);
void str_trim_right(char* psz);
void str_trim_left(char* psz);
void str_trim(char* psz);
void str_split(const char* pszSrc,char* pszDst,size_t nDstBufLen,size_t nLineLen);
long str_getnextl(const char* pszString,char** endptr,int base);
//------------------------------------------------------------------------
int AsciiToUnicode(const char * szAscii, wchar_t * szUnicode);
int UnicodeToAscii(const wchar_t * szUnicode, char * szAscii);
//------------------------------------------------------------------------
bool isnumber(const char* pstr);
//------------------------------------------------------------------------
WORD cA2W(WORD ch);
WORD cW2A(WORD ch);
//------------------------------------------------------------------------
size_t sA2W(wchar_t *wcstr,const char *mbstr,size_t count );
size_t sW2A(char *mbstr,const wchar_t *wcstr,size_t count );
//------------------------------------------------------------------------
int getcharcount(const char * str);
//------------------------------------------------------------------------
__inline int __stdcall str_crc32(const char* __s)
{
	return (int) crc32data((unsigned char *) __s, safe_min((int)strlen(__s), 128));
}
//------------------------------------------------------------------------
__inline int __stdcall std_str_hash(const char* __s, int nlen)
{
	if(!__s)
	{
		return 0;
	}

	unsigned int __h = 0;

	for(; *__s && nlen > 0; ++__s, --nlen)
	{
		__h = 5 * __h + *__s;
	}

	return int(__h);
};
//------------------------------------------------------------------------
__inline int __stdcall str_dbhashcode(const char* __s, int nlen, int dbcount, int tblcount1db, int alltblcount)
{
	int nhashvalue = std_str_hash(__s, nlen);
	int ntblid = nhashvalue % alltblcount;
	int ndbhashcode = (ntblid / tblcount1db);
	return ndbhashcode;
};
//------------------------------------------------------------------------
__inline int Mem2Hex(char* pin,int nsize,char* pout,int nout){
	for (int i=0;i<nsize;i++){
		sprintf_s( pout,nout-1,"%.2x",BYTE(pin[i]) );
		pout+=2;
		nout-=2;
	}
	return (nsize*2);
}
//------------------------------------------------------------------------
__inline int AbsMem2Hex(char* pin,int nsize,int nbef,int nmid,int naft,char* pout,int nout){

	if (nsize<=(nbef+nmid+naft)){
		return Mem2Hex(pin,nsize,pout,nout);
	}else{
		char* poldout=pout;
		int nret=Mem2Hex(pin,nbef,pout,nout);
		pout+=nret;
		nout-=nret;

		strcat(pout,"...");
		pout+=3;
		nout-=3;

		int nmidpos=(((nsize-nbef-naft-nmid)/2)+nbef);
		nret=Mem2Hex(&pin[nmidpos],nmid,pout,nout);
		pout+=nret;
		nout-=nret;

		strcat(pout,"...");
		pout+=3;
		nout-=3;

		nret=Mem2Hex(&pin[nsize-naft],naft,pout,nout);
		pout+=nret;
		nout-=nret;
		*pout=0;
		return (poldout-pout);
	}
}
//------------------------------------------------------------------------
template <typename T>
bool PrintMem2C(const T *p, UINT nSize,std::string& sRet,char* cpname=NULL)
{
	std::string stmp="";
	char tmp[128]={0};

	char cName[256]={"arrayname"};
	if (cpname)
	{
		strcpy_s(cName,sizeof(cName),cpname);
	}
	sprintf_s(tmp,sizeof(tmp)-1,"%s %s[%i] = {\r\n",typeid(T).name(),cName, nSize);

	sRet=tmp;
	int iNrPrinted=0;
	char szformat[128];
	sprintf_s(szformat,sizeof(szformat),"0x%%%.2dlx, ",sizeof(T)*2);
	for ( UINT i=0; i<nSize; i++ )
	{

		if (i==nSize-1)
		{
			char* pf=strrchr(szformat,',');
			if (pf)
			{
				*pf='\0';
			}
			sprintf_s(tmp,sizeof(tmp),szformat, p[i]);
		}
		else
		{
			sprintf_s(tmp,sizeof(tmp),szformat, p[i]);
		}
		iNrPrinted++;
		sRet+=tmp;
		if(iNrPrinted%6==0)
			sRet+="\r\n";
	}
	sRet+="};";
	return true;	
}
//------------------------------------------------------------------------
void to_lower(std::string &s);
void to_upper(std::string &s);
//------------------------------------------------------------------------
inline bool IsNewLine(const char* p,size_t &charnum){
	if((*p) == '\r'){
		if(*(p + 1) == '\n')
			charnum = 2;
		else
			charnum = 1;
		return true;
	}
	if((*p) == '\n'){
		charnum = 1;
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
inline bool IsNewLineW(const WCHAR* p,size_t &charnum)
{
	if((*p) == L'\r'){
		if(*(p + 1) == L'\n')
			charnum = 2;
		else
			charnum = 1;
		return true;
	}
	if((*p) == L'\n'){
		charnum = 1;
		return true;
	}
	return false;
}
//------------------------------------------------------------------------
template <class tVecString >
void str_split(const char* pszSrc,tVecString &listString,size_t nLineLen)
{
	size_t i;
	i =0;

	tVecString::value_type * pstrDst = NULL;
	listString.reserve(8);

	while(*pszSrc)
	{
		if((*pszSrc) & 0x80)
		{
			if(!(*(pszSrc+1))) break;

			if(!pstrDst)
			{
				listString.resize(listString.size()+1);
				pstrDst = &listString.back();
				pstrDst->reserve(256);
			}

			pstrDst->push_back(*pszSrc++);
			pstrDst->push_back(*pszSrc++);
			i+=2;

			if(i >= nLineLen)
			{

				Assert(pstrDst);
				pstrDst->push_back(0);
				pstrDst = NULL;
				i = 0;
			}
		}
		else
		{
			if(!pstrDst)
			{
				listString.resize(listString.size()+1);
				pstrDst = &listString.back();
				pstrDst->reserve(256);
			}

			size_t iCharNum;
			if(IsNewLine(pszSrc,iCharNum))
			{

				pstrDst->push_back(0);
				pstrDst = NULL;
				i = 0;
				pszSrc += iCharNum;
			}
			else
			{
				pstrDst->push_back(*pszSrc++);
				i++;

				if( i >= nLineLen)
				{

					Assert(pstrDst);
					pstrDst->push_back(0);
					pstrDst = NULL;
					i = 0;
				}
			}

		}
	}
	if(pstrDst) pstrDst->push_back(0);
}
//------------------------------------------------------------------------
#include <vector>
//------------------------------------------------------------------------
template < class Type, class Allocator >
void _parse_str(std::vector<Type,Allocator> & vRet,const char* pszString,char ch)
{
	vRet.resize(0);
	const char* p1,*p2;
	p1 = pszString;
	for(;;)
	{
		while(*p1 != 0 && *p1 == ch)
		{
			p1++;
		}

		if(*p1 == 0) break;

		p2 = p1;

		while(*p2 != 0 && *p2!=ch)
		{
			p2++;
		}

		vRet.resize(vRet.size()+1);
		Type & str = vRet.back();
		str.resize(p2 - p1 +1 );
		memcpy(&str[0],p1,p2-p1);
		str[p2-p1] = 0;
		if(*p2 == 0) break;

		p1 = p2+1;
	}
}
//------------------------------------------------------------------------
template <class tVectorInt>
void _parse_str_num(tVectorInt & vRet,const char* pszString)
{
	vRet.resize(0);
	const char* p1,*p2;
	p1 = pszString;
	for(;;)
	{
		while(*p1 != 0 && !( '0' <= *p1 && *p1 <= '9'))
		{
			p1++;
		}

		if(*p1 == 0) break;

		p2 = p1;

		while(*p2 != 0 && ( '0' <= *p2 && *p2 <= '9'))
		{
			p2++;
		}

		char szTmp[256];
		int len = safe_min(p2-p1,255);
		memcpy(szTmp,p1,len);
		szTmp[len] = 0;

		vRet.push_back(atol(szTmp));

		if(*p2 == 0) break;

		p1 = p2+1;
	}
}
//------------------------------------------------------------------------
template <class tVectorInt>
void _parse_str_num_sign(tVectorInt & vRet,const char* pszString)
{
	vRet.resize(0);
	const char* p1,*p2;
	p1 = pszString;
	for(;;)
	{
		while(*p1 != 0 && !( ( '0' <= *p1 && *p1 <= '9') || *p1 == '-'))
		{
			p1++;
		}
		if(*p1 == 0) break;

		p2 = p1;
		if(*p2 == '-') ++p2;
		if(*p2 == 0) break;

		while(*p2 != 0 && ( '0' <= *p2 && *p2 <= '9' ))
		{
			p2++;
		}

		char szTmp[256];
		int len = safe_min(p2-p1,255);
		memcpy(szTmp,p1,len);
		szTmp[len] = 0;

		vRet.push_back(atol(szTmp));

		if(*p2 == 0) break;

		p1 = p2+1;
	}
}
//------------------------------------------------------------------------
struct string_key_hash : public std::unary_function<const std::string, size_t>
{
	size_t operator()(const std::string &x) const;
};
//------------------------------------------------------------------------
struct string_key_case_hash : public std::unary_function<const std::string, size_t>
{
	size_t operator()(const std::string &x) const;
};
//------------------------------------------------------------------------
struct string_key_equal : public std::binary_function<const std::string, const std::string, bool>
{
	bool operator()(const std::string &s1, const std::string &s2) const;
};
//------------------------------------------------------------------------
struct string_key_case_equal : public std::binary_function<const std::string, const std::string, bool>
{
	bool operator()(const std::string &s1, const std::string &s2) const;
};
//------------------------------------------------------------------------
struct pchar_key_hash : public std::unary_function<const char*, size_t>
{
	size_t operator()(const char* x) const;
};
//------------------------------------------------------------------------
struct pchar_key_case_hash : public std::unary_function<const char*, size_t>
{
	size_t operator()(const char* x) const;
};
//------------------------------------------------------------------------
struct pchar_key_equal : public std::binary_function<const char*, const char*, bool>
{
	bool operator()(const char* s1, const char* s2) const;
};
//------------------------------------------------------------------------
struct pchar_key_case_equal : public std::binary_function<const char*, const char*, bool>
{
	bool operator()(const char* s1, const char* s2) const;
};
//------------------------------------------------------------------------
template <int tILen>
class tString
{
private:
	char szString[tILen];
public:
	typedef tString<tILen> _Myt;
	operator const char* () const {return szString;}
	tString()
	{
		szString[0] = 0;
	}
	const char* c_str() const 
	{	
		return szString;
	}
	tString(const char* psz)
	{
		if (psz==NULL){ szString[0] = 0;return; }
		strcpy_s(szString,tILen,psz);
		szString[tILen-1] = 0;
	}
	tString(const _Myt& ts)
	{
		strcpy_s(szString,tILen,ts.c_str());
		szString[tILen-1] = 0;
	}
	_Myt & operator = (const char* psz)
	{
		if (psz==NULL){ szString[0] = 0;return (*this); }
		strcpy_s(szString,tILen,psz);
		szString[tILen-1] = 0;
		return (*this);
	}
	BOOL operator == (const char* psz) const 
	{
		if (psz==NULL)
		{
			return (szString[0]==0);
		}
		else
		{
			return (strcmp(szString,psz) == 0);
		}
	}
	BOOL operator != (const char* psz) const
	{
		if (psz==NULL)
		{
			return (szString[0]!=0);
		}
		else
		{
			return (strcmp(szString,psz) != 0);
		}
	}
	_Myt & operator +=(const char* psz)
	{
		if (psz==NULL){return (*this); }
		strncat(szString,psz,tILen);
		szString[tILen-1] = 0;
		return (*this);
	}
	_Myt & operator += (char ch)
	{
		size_t len = strlen(szString);
		if(len < tILen-1)
		{
			szString[len] = ch;
			szString[len+1] = 0;
		}
		return (*this);
	}
	BOOL operator > (const char* psz)
	{
		if (psz==NULL)
		{
			return false;
		}
		else
		{
			return strcmp(szString,psz)>0;
		}
	}
	BOOL operator >= (const char* psz)
	{
		if (psz==NULL)
		{
			return false;
		}
		else
		{
			return strcmp(szString,psz)>=0;
		}
	}
	BOOL operator < (const char* psz)
	{
		if (psz==NULL)
		{
			return false;
		}
		else
		{
			return strcmp(szString,psz)<0;
		}
	}
	BOOL operator <= (const char* psz)
	{
		if (psz==NULL)
		{
			return false;
		}
		else
		{
			return strcmp(szString,psz)<=0;
		}
	}
	size_t length() const
	{
		return strlen(szString);
	}
	void set(char ch,size_t count)
	{
		count = (count < (tILen-1) ? count : (tILen-1));
		memset(szString,ch,count);
		szString[count] = 0;
	}
	void set(const char* sz,size_t count)
	{
		if (sz==NULL){ szString[0] = 0;return };
		count = (count < (tILen-1) ? count : (tILen-1));
		memcpy(szString,sz,count);
		szString[count] = 0;
	}
	void set(size_t index,const char* psz)
	{
		if (index>=tILen){return;}
		if (psz==NULL){ szString[index] = 0;return };
		strcpy_s(&szString[index],tILen-index,psz);
		szString[tILen-1] = 0;
	}
	void setAt(size_t index,char ch)
	{
		if (index>=tILen){return;}
		szString[index] = ch;
		szString[tILen-1] = 0;
	}
	void Delete(size_t index,size_t count)
	{
		if (index>=tILen || index+count>=tILen){return;}
		strcpy_s(&szString[index],tILen-index,&szString[index + count]);
	}
	void Format(const char* szFmt,...)
	{
		if (szFmt==NULL){ return };
		va_list ap;
		va_start( ap, szFmt );
		_vsnprintf(szString,tILen,szFmt,ap);
		szString[tILen-1] = 0;
		va_end  ( ap );
	}
};
//------------------------------------------------------------------------