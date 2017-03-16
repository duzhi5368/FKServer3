/**
*	created:		2013-3-22   23:01
*	filename: 		FKStringEx
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKVsVer8Define.h"
#include "../Include/FKStringEx.h"
#include <time.h>
#include <malloc.h>
#include <xhash>
#include "../Include/FKThread.h"
//------------------------------------------------------------------------
const char CEasyStrParse::NullChar[4]={0,0,0,0};
//------------------------------------------------------------------------
void CEasyStrParse::ParseParam(int nCount)
{
	if (m_pStart && m_pStart[0]!=0 && nCount>0)
	{
		if (m_pch==NULL || m_pch[0]==0)
		{
			if (m_pszParams.size()==0){
				m_pszParams.push_back(m_pStart);
				m_pStart=(char *)&NullChar;
			}
			return;
		}
		if ((size_t)nCount<=m_pszParams.size())
		{
			return;
		}

		int b_a_idx=-1;
		char curch=0;
		int tmpidx=0;
		int curch_state=0;
		char* pread=m_pStart;
		char* pwrite=m_pStart;
		char* m_laststart=pwrite;
		bool isfirstb_a=false;
		bool iszychar=false;

		while(*pread)
		{
			curch=*pread;
			isfirstb_a=false;
			iszychar=false;
			switch(curch_state)   
			{
			case 0:
				{
					tmpidx=0;
					while(m_pBAch[tmpidx]){
						if (curch==m_pBAch[tmpidx]){ 
							isfirstb_a=true;b_a_idx=tmpidx+1;curch_state=1; break; 
						}
						tmpidx+=2;
					}
				}
				break;
			case 1:
				{
					if ( curch==m_zychr && (b_a_idx>=0 && pread[1]!=0 && pread[1]==m_pBAch[b_a_idx]) ){
						iszychar=true;
						pread++;
					}else if (b_a_idx>=0 && curch==m_pBAch[b_a_idx]){
						curch_state=0;
						b_a_idx=-1;
					}
				}
				break;
			}
			if ( !iszychar &&  (isfirstb_a || curch_state==0) )
			{
				tmpidx=0;
				while(m_pch[tmpidx])
				{
					if (curch==m_pch[tmpidx])
					{
						*pwrite='\0';
						*pread='\0';

						if (m_bokeepnil || m_laststart[0]!=0)
						{
							m_pszParams.push_back(m_laststart);
							m_laststart=pwrite;m_laststart++;
							if (m_pszParams.size()>=(size_t)nCount && curch_state==0)
							{
								m_pStart=pread;m_pStart++;return;
							}
							else if (pread[1]==0)
							{
								m_laststart=NULL;
								m_pStart=pread;m_pStart++;return;
							}
						}
						else
						{
							m_laststart=pwrite;m_laststart++;
						}
						break;
					}
					tmpidx++;
				}
			}
			*pwrite=*pread;
			pread++;
			pwrite++;
		}
		*pwrite=*pread;
		if (m_laststart)
		{
			if (m_laststart[0]!=0)
			{
				m_pszParams.push_back(m_laststart);
			}
			m_laststart=NULL;
		}
		m_pStart=pread;
	}
	return;
}
//------------------------------------------------------------------------
CEasyStrParse::CEasyStrParse()
{
	m_pStart=NULL;
	m_pSrcStart=NULL;
	m_pch=NULL;
	m_pBAch=NULL;
	m_psrc=NULL;
	m_zychr=0;
	m_npsrc_maxlen=0;
	m_bokeepnil=false;
}
//------------------------------------------------------------------------
CEasyStrParse::~CEasyStrParse()
{
	if (m_psrc){	__mt_char_alloc.deallocate(m_psrc);}
	m_psrc=NULL;
	m_npsrc_maxlen=0;
}
//------------------------------------------------------------------------
bool CEasyStrParse::SetParseStrEx(const char* psz,char* pszCh,char* pszBAch,char zychr,bool keepnil)
{
	size_t nlen=(strlen(psz)+16);
	if (m_psrc && m_npsrc_maxlen<nlen)
	{
		__mt_char_alloc.deallocate(m_psrc);
		m_psrc=NULL;
		m_npsrc_maxlen=0;
	}
	if (m_psrc==NULL)
	{
		m_psrc=(char*)__mt_char_alloc.allocate(nlen);
	}
	if (m_psrc)
	{
		m_npsrc_maxlen=nlen;
		strcpy_s(m_psrc,nlen,psz);
		return SetParseStr(m_psrc,pszCh,pszBAch,zychr,keepnil);
	}
	return false;
}
//------------------------------------------------------------------------
bool CEasyStrParse::SetParseStr(char* psz,char* pszCh,char* pszBAch,char zychr,bool keepnil){
	m_pszParams.clear();
	m_pszParams.reserve(8);
	m_zychr=zychr;
	m_pStart=NULL;
	m_pSrcStart=NULL;
	m_pch=NULL;
	m_pBAch=NULL;
	m_bokeepnil=keepnil;
	if (SetBAch(pszBAch)){
		if (psz){
			m_pSrcStart=psz;
			m_pStart=psz;
			SetCh(pszCh);
			return true;
		}
		else{return false;}
	}
	return false;
}
//------------------------------------------------------------------------
bool CEasyStrParse::SetCh(char* psz){
	m_pch=(char *)&NullChar;
	if (psz){ m_pch=psz; return true;}
	return false;
}
//------------------------------------------------------------------------
bool CEasyStrParse::SetBAch(char* psz){
	m_pBAch=(char *)&NullChar;
	if (psz==NULL){	return true;}
	else if ((strlen(psz) % 2)==0){m_pBAch=psz;return true;}
	return false;
}
//------------------------------------------------------------------------
char* CEasyStrParse::Param(int i)
{
	char* pRet=NULL;

	i++;
	if(i<1){
		return m_pSrcStart;
	}
	if ((size_t)i<=m_pszParams.size()){
		pRet= m_pszParams[i-1];
	}else{
		ParseParam(i);
		if ((size_t)i<=m_pszParams.size()){
			pRet= m_pszParams[i-1];
		}
	}
	if (pRet==NULL){pRet=(char *)&NullChar;}
	return pRet;
}
//------------------------------------------------------------------------
const char* vformat(const char* pszFmt,...)
{
	va_list ap;
	va_start( ap, pszFmt );

	_GET_TLS_LOOPCHARBUF(1024);
	_vsnprintf(ptlsbuf,ntlslen,pszFmt,ap);
	ptlsbuf[ntlslen-1] = 0;
	va_end  ( ap );
	return ptlsbuf;
}
//------------------------------------------------------------------------
const char* itochina(int i)
{
	if ( i>9 ) i = 9;
	static char china[10][3]={"零","一","二","三","四","五","六","七","八","九"};
	return china[i];
}
//------------------------------------------------------------------------
char* endate2date(char* src)
{
	CEasyStrParse parse;

	char sztempdate[32];
	strcpy_s(sztempdate,sizeof(sztempdate)-1,src);
	parse.SetParseStr(sztempdate," ",NULL);
	src=parse[0];

	int mm=0;
	switch(src[0])
	{
	case  'J':
		{
			if (src[1]=='a')	
				mm= 01;
			else if (src[2]=='l')	
				mm= 07;
			else 
				mm= 06;
		}
		break;
	case 'F': mm= 02;break;
	case 'M':
		{
			if (src[2]=='r')	
				mm= 03;
			else
				mm= 05;
		}
		break;
	case 'A':
		{
			if (src[1]=='u')	
				mm= 04;
			else
				mm= 8;
		}
		break;
	case 'S': mm= 9;break;
	case 'O': mm= 10;break;
	case 'N': mm= 11;break;
	case 'D': mm= 12;break;
	} 
	_GET_TLS_LOOPCHARBUF(1024);
	sprintf_s(ptlsbuf,ntlslen-1,"%s-%2.2d-%s",parse[2],mm,parse[1]);
	ptlsbuf[ntlslen-1] = 0;
	return ptlsbuf;
}
//------------------------------------------------------------------------
const char* sec2str(int iSecond)
{
	int second = iSecond % 60;
	int minute = iSecond / 60;
	int hour =   minute / 60;
	int day =    hour / 24;
	_GET_TLS_LOOPCHARBUF(1024);
	if( iSecond <  0 )
	{
		return "";
	}
	else if( iSecond < 60 )
	{
		_snprintf(ptlsbuf,ntlslen-1,"%d 秒",iSecond);
		ptlsbuf[ntlslen-1] = 0;
	}
	else if( iSecond < 60*60 )
	{		
		_snprintf(ptlsbuf,ntlslen-1,"%d分%d秒",minute%60,second);
		ptlsbuf[ntlslen-1] = 0;
	}
	else if( iSecond < 60*60*24 )
	{
		_snprintf(ptlsbuf,ntlslen-1,"%d小时%d分%d秒",hour,minute%60,second);
		ptlsbuf[ntlslen-1] = 0;
	}
	else
	{
		_snprintf(ptlsbuf,ntlslen-1,"%d天%d小时%d分%d秒",day,hour%24,minute%60,second);
		ptlsbuf[ntlslen-1] = 0;
	}
	return ptlsbuf;
}
//------------------------------------------------------------------------
char* find_path_dian_houzui(const char* pname)
{
	char* p=(char*)strrchr(pname,'.');
	if (p && p[1]!='\\' && p[1]!='/'){
		const char* p1=safe_max(strrchr(pname,'\\'),strrchr(pname,'/'));
		if (p>p1){
			return p;
		}
	};
	return NULL;
}
//------------------------------------------------------------------------
const char* extractfiletitle(const char* pname)
{
	return changefileext(extractfilename(pname),"");
}
//------------------------------------------------------------------------
const char* extractfilename(const char* pname)
{
	if (!pname){return pname;}

	const char* p=safe_max(strrchr(pname,'\\'),strrchr(pname,'/'));
	if (p){p++;}else{p=pname;}

	_GET_TLS_LOOPCHARBUF(1024);
	strcpy_s(ptlsbuf,ntlslen-1,p);
	ptlsbuf[ntlslen-1] = 0;
	return ptlsbuf;
}
//------------------------------------------------------------------------
const char* extractfiledir(const char* pname)
{
	if (!pname){return pname;}

	_GET_TLS_LOOPCHARBUF(1024);

	strcpy_s(ptlsbuf,ntlslen-1,pname);
	int nlen=strlen(ptlsbuf);

	if (nlen>0)
	{
		char* p=find_path_dian_houzui(ptlsbuf);
		if (p==NULL){if (ptlsbuf[nlen-1]=='\\' || ptlsbuf[nlen-1]=='/'){ptlsbuf[nlen-1]=0;}}
		else
		{
			char* pend=ptlsbuf+nlen-1;
			while (pend>=ptlsbuf){if (*pend=='/' || *pend=='\\' ){break;};pend--;};
			if (pend<ptlsbuf){ptlsbuf[0]=0;}
			else if (pend<p){*pend=0;}
		}
	}
	ptlsbuf[ntlslen-1] = 0;
	return ptlsbuf;
}
//------------------------------------------------------------------------
const char* extractfilepath(const char* pname)
{
	if (!pname){return pname;}

	_GET_TLS_LOOPCHARBUF(1024);

	strcpy_s(ptlsbuf,ntlslen-1,pname);
	int nlen=strlen(ptlsbuf);
	if (nlen>0)
	{
		char* p=find_path_dian_houzui(ptlsbuf);
		if (p==NULL){if (ptlsbuf[nlen-1]!='\\' && ptlsbuf[nlen-1]!='/'){ptlsbuf[nlen]='\\';ptlsbuf[nlen+1]=0;}}
		else
		{
			char* pend=ptlsbuf+nlen-1;
			while (pend>=ptlsbuf){if (*pend=='/' || *pend=='\\' ){break;};pend--;};
			if (pend<ptlsbuf){ptlsbuf[0]=0;}
			else if (pend<p){*(pend+1)=0;}
		}
	}
	ptlsbuf[ntlslen-1] = 0;
	return ptlsbuf;
}
//------------------------------------------------------------------------
const char* extractfileext(const char* pname)
{
	if (!pname){return pname;}

	_GET_TLS_LOOPCHARBUF(1024);
	char* p=find_path_dian_houzui(pname);
	if (p){p++;strcpy_s(ptlsbuf,ntlslen-1,p);}
	else {ptlsbuf[0]=0;};
	ptlsbuf[ntlslen-1] = 0;
	return ptlsbuf;
}
//------------------------------------------------------------------------
const char* changefileext(const char* pname,const char* pchg){
	if (!pname){return pname;}

	_GET_TLS_LOOPCHARBUF(1024);

	strcpy_s(ptlsbuf,ntlslen-1,pname);

	char* p=find_path_dian_houzui(ptlsbuf);
	if (p && strlen(pchg)>0 && pchg[0]!='.'){p++;};
	if (p){strcpy_s(p,ntlslen-(p-ptlsbuf)-1,pchg);}
	return ptlsbuf;
}
//------------------------------------------------------------------------
const char* addfileext(const char* pname,const char* padd,char bchg){
	if (!pname || !padd){return pname;}

	_GET_TLS_LOOPCHARBUF(1024);
	strcpy_s(ptlsbuf,ntlslen-1,pname);

	char* p=find_path_dian_houzui(ptlsbuf);
	if (p){*p=bchg;}
	if (padd[0]!='.'){strncat(ptlsbuf,".",ntlslen-1);}
	strncat(ptlsbuf,padd,ntlslen-1);
	ptlsbuf[ntlslen-1] = 0;
	return ptlsbuf;
}
//------------------------------------------------------------------------
char* getwithtimefilename(char* srcpch){
	if (srcpch){

		char pch[MAX_PATH]={0};
		strcpy_s(pch,sizeof(pch),srcpch);

		time_t ti = time(NULL);
		tm* t = localtime(&ti);
		char szTime[MAX_PATH];
		strftime(szTime,MAX_PATH,"%y%m%d%H%M%S",t);

		_GET_TLS_LOOPCHARBUF(1024);
		char* p=find_path_dian_houzui(pch);
		if(p){*p=0;p++;}
		else{p="";}
		sprintf_s(ptlsbuf,ntlslen-1,"%s_%s.%s\0",pch,szTime,p);
		ptlsbuf[ntlslen-1] = 0;
		return ptlsbuf;
	}
	return NULL;
}
//------------------------------------------------------------------------
void replacelashPath(char *str,bool front){
	char src=(!front)?'\\':'/';
	char dst=front?'\\':'/';
	while(*str){
		if(*str == src)
			*str =dst;
		str++;
	}
}
//------------------------------------------------------------------------
char* timetostr(time_t time1,char *szTime,int nLen,const char* sformat){
	struct tm tm1;
	memset(&tm1,0,sizeof(tm1));
	_GET_TLS_LOOPCHARBUF(1024);
	localtime_s(&tm1,&time1);
	sprintf_s( ptlsbuf,ntlslen-1, sformat,tm1.tm_year+1900, tm1.tm_mon+1, tm1.tm_mday,tm1.tm_hour, tm1.tm_min,tm1.tm_sec);
	ptlsbuf[ntlslen-1] = 0;
	if (szTime)
	{
		strcpy_s(szTime,nLen,ptlsbuf);
	}
	return ptlsbuf;
}
//------------------------------------------------------------------------
time_t strtotime(const char * szTime,const char* sformat)
{
	if( !szTime )
		return 0;

	std::string _TimeFormat = szTime;
	if( _TimeFormat.find( '/' ) != string::npos )
	{
		sformat = "%4d/%2d/%2d %2d:%2d:%2d";
	}
	else
	{
		sformat = "%4d-%2d-%2d %2d:%2d:%2d";
	}

	struct tm tm1;
	memset(&tm1,0,sizeof(tm1));
	time_t time1;
	sscanf_s(szTime, sformat,     
		&tm1.tm_year, 
		&tm1.tm_mon, 
		&tm1.tm_mday, 
		&tm1.tm_hour, 
		&tm1.tm_min,
		&tm1.tm_sec);
	tm1.tm_year -= 1900;
	tm1.tm_mon --;
	tm1.tm_isdst=-1;
	time1 = mktime(&tm1);
	return time1;
}
//------------------------------------------------------------------------
void str_replace(const char* pszSrc,char* pszDst,int ndlen ,const char* pszOld,const char* pszNew){
	const char* p,*q;
	char *q1;
	int len_old = (int)strlen(pszOld);
	int len_new = (int)strlen(pszNew);
	int nnext=(int)strlen(pszSrc)+(len_new-len_old);
	int l;
	q = pszSrc;
	q1 = pszDst;
	for(;;){
		p = strstr(q,pszOld);
		if(p == NULL || nnext>=ndlen){
			strcpy(q1,q);
			break;
		}else{
			l = (int)(p-q);
			memcpy(q1,q,l);
			q1 += l;
			memcpy(q1,pszNew,len_new);
			q1 += len_new;
			q = p + len_old;
			q1[0]=0;
			nnext +=(len_new-len_old);
		}
	}
}
//------------------------------------------------------------------------
void str_replace(char* pszSrc,const char* pszOld,const char* pszNew)
{
	int len = (int)strlen(pszSrc);

	STACK_ALLOCA(char*,psz,len*4);
	if (psz){
		str_replace(pszSrc,psz,len*2,pszOld,pszNew);
		strcpy(pszSrc,psz);
	}
}
//------------------------------------------------------------------------
void str_replace(char* pszSrc,char cOld,char cNew){
	while(*pszSrc){
		if((*pszSrc) & 0x80){

			if(!(*(pszSrc+1))) break;
			pszSrc+=2;
		}else{
			if(*pszSrc == cOld)
				*pszSrc = cNew;
			pszSrc++;
		}
	}
}
//------------------------------------------------------------------------
void str_replace(char* pszSrc,wchar_t cOld,wchar_t cNew){
	while(*pszSrc){
		if((*pszSrc) & 0x80){

			if(!(*(pszSrc+1))) break;
			if((*(wchar_t*)pszSrc) == cOld){
				(*(wchar_t*)pszSrc) = cNew;
			}
			pszSrc+=2;
		}else{
			pszSrc++;
		}
	}
}
//------------------------------------------------------------------------
std::string str_Replace(std::string &strText,const char *pszSrcText,const char *pszDestText){
	char *p = (char*)strstr(strText.c_str(),pszSrcText);
	if(!p) return strText;
	size_t size = (size_t )(p - strText.c_str());
	while(size <= strText.length() - strlen(pszSrcText) || !p)
	{
		strText.replace(size,strlen(pszSrcText),pszDestText);
		if(size_t ( p - strText.c_str()) > strText.length() - 2) break;
		p = strstr(p+1,pszSrcText);
		if(!p) break;
		size = (size_t )(p - strText.c_str()) + 1;
	}
	return strText;
}
//------------------------------------------------------------------------
void str_del(char* pszSrc,char c){
	while(*pszSrc){
		if((*pszSrc) & 0x80){

			if(!(*(pszSrc+1))) break;
			pszSrc+=2;
		}else{
			if((*pszSrc) == c){
				strcpy(pszSrc,pszSrc+1);
			}else{
				pszSrc++;
			}
		}
	}
}
//------------------------------------------------------------------------
void str_del(char* pszSrc,wchar_t c){
	while(*pszSrc){
		if((*pszSrc) & 0x80){

			if(!(*(pszSrc+1))) break;

			if((*(wchar_t*)pszSrc) == c){
				strcpy(pszSrc,pszSrc+2);
			}else{
				pszSrc+=2;
			}
		}else{
			pszSrc++;
		}
	}
}
//------------------------------------------------------------------------
void str_del(char* pszSrc,int c){
	if(c & 0x80)
		str_del(pszSrc,(wchar_t)c);
	else
		str_del(pszSrc,(char)c);
}
//------------------------------------------------------------------------
void str_del_sp( char* pszSrc, char c )
{
	char* pszStr = pszSrc;
	while ( *pszStr ){
		if ( *pszStr == c ){

			if ( !(*(pszStr+1)) ){
				strcpy( pszSrc, pszStr );
				break;
			}
			if ( *(pszStr+1) == c ){
				pszStr++ ;
			}else{
				strcpy( pszSrc++, pszStr );
				pszStr = pszSrc;
			}
		}else{
			if ( (*pszStr) & 0x80 ){

				if ( !(*(pszStr+1)) )	break;
				pszStr += 2;
			}else{
				pszStr++ ;
			}
			pszSrc = pszStr;
		}		
	}
}
//------------------------------------------------------------------------
void str_del_sp_full( char* pszSrc, char c )
{
	char* pszStr = pszSrc;
	while ( *pszStr ){
		if ( *pszStr == c ){

			if ( !(*(pszStr+1)) ){
				strcpy( pszSrc, pszStr+1 );
				break;
			}else{
				if ( *(pszStr+1) == c ){
					pszStr++ ;
				}else{
					strcpy( pszSrc, ++pszStr );
					pszStr = pszSrc;
				}
			}
		}
		else{
			if ( (*pszStr) & 0x80 ){

				if ( !(*(pszStr+1)) )	break;
				pszStr += 2;
			}else{
				pszStr++ ;
			}
			pszSrc = pszStr;
		}		
	}
}
//------------------------------------------------------------------------
#include <tchar.h>
//------------------------------------------------------------------------
BOOL str_isempty(const char* psz)
{
	if(psz == NULL) return TRUE;
	while(*psz)
	{
		if((*psz) & 0x80)
		{
			if(!(*(psz+1))) break;

			if(!_ismbcspace(*((WORD*)psz))) break;
			psz+=2;
		}
		else
		{
			if(!isspace(*psz)) break;
			psz++;
		}
	}
	return (*psz == 0);
}
//------------------------------------------------------------------------
char* str_cpy(char* pszDest,const char* pszSrc,int len){
	int i = 0;
	char* p = pszDest;
	while(*pszSrc){
		if((*pszSrc) & 0x80){

			if(*(pszSrc+1) == 0) break;
			if(len-i <= 2) break;
			*(unsigned short*)pszDest = *(const unsigned short*)pszSrc;
			pszDest += 2;
			pszSrc  += 2;
			i+=2;		
		}else{
			if(len-i <= 1) break;
			*pszDest = *pszSrc;
			++pszDest;
			++pszSrc;
			++i;
		}
	}
	*pszDest = 0;
	return p;
}
//------------------------------------------------------------------------
void str_trim_right(char* psz){
	char* p = psz;
	char* p1 = NULL;
	while(*p){
		if((*p) & 0x80){
			if(iswspace(*((WORD*)p))){
				if(p1 == NULL)
					p1 = p;
			}else{
				if(p1)p1 = NULL;
			}
			p+=2;
		}else{
			if(isspace(*p)){
				if(p1 == NULL)
					p1 = p;
			}else{
				if(p1)p1 = NULL;
			}
			p++;
		}
	}
	if(p1) *p1 = 0;
}
//------------------------------------------------------------------------
void str_trim_left(char* psz){
	char* p = psz;
	while(*p){
		if((*p) & 0x80){
			if(iswspace(*((WORD*)p))){}
			else{break;}
			p+=2;
		}else{
			if(isspace(*p)){}
			else{break;}
			p++;
		}
	}
	strcpy(psz,p);
}
//------------------------------------------------------------------------
void str_trim(char* psz){
	str_trim_left(psz);
	str_trim_right(psz);
}
//------------------------------------------------------------------------
WORD cA2W(WORD ch){
	BYTE wcstr[2];
	if(ch & 0x80){
		BYTE* p = (BYTE*)&ch;
		wcstr[0] = p[1];
		wcstr[1] = p[0];
	}else{
		wcstr[0] = (BYTE)ch;
		wcstr[1] = 0;
	}
	return *(WORD*)wcstr;
}
//------------------------------------------------------------------------
WORD cW2A(WORD ch){
	BYTE mbstr[2];
	if( ch & 0xff00 ){
		BYTE* p = (BYTE*)&ch;
		mbstr[0] = p[1];
		mbstr[1] = p[0];
	}else{
		mbstr[0] = (BYTE)(WORD)ch;
		mbstr[1] = 0;
	}
	return *(WORD*)mbstr;
}
//------------------------------------------------------------------------
int AsciiToUnicode(const char * szAscii, wchar_t * szUnicode){
	DWORD dwNum = MultiByteToWideChar(CP_ACP,0,szAscii,-1,NULL,0);
	dwNum = MultiByteToWideChar(CP_ACP,0,szAscii,-1,szUnicode,dwNum);
	szUnicode[dwNum]=0;
	return dwNum;
}
//------------------------------------------------------------------------
int UnicodeToAscii(const wchar_t * szUnicode, char * szAscii){
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP,NULL,szUnicode,-1,NULL,0,NULL,FALSE);
	dwNum = WideCharToMultiByte(CP_OEMCP,NULL,szUnicode,-1,szAscii,dwNum,NULL,FALSE);
	szAscii[dwNum]=0;
	return dwNum;
}
//------------------------------------------------------------------------
bool isnumber(const char* pstr){
	if (!(pstr[0]=='+' || pstr[0]=='-' || (pstr[0]>='0' && pstr[0]<='9'))){	return false; }
	int diancount=0;
	pstr++;
	while(pstr[0]!=0){
		if (pstr[0]>='0' && pstr[0]<='9'){
			pstr++;
		}else if(pstr[0]=='.' && diancount==0){
			diancount++;
			pstr++;
		}else{
			return false;
		}
	}
	return true;
}
//------------------------------------------------------------------------
size_t sA2W(wchar_t *wcstr,const char *mbstr,size_t count ){
	size_t i = 0;
	size_t ret = 0;
	while(i < count && mbstr[i]){
		if(mbstr[i] & 0x80){
			if( i+1 >= count || mbstr[i + 1] == 0) break;
			BYTE* p = (BYTE*)&wcstr[ret++];
			p[0] = mbstr[i+1];
			p[1] = mbstr[i];
			i+=2;
		}else{
			wcstr[ret++] = (WORD)(BYTE)mbstr[i];
			i++;
		}
	}
	wcstr[ret] = 0;
	return ret;
}
//------------------------------------------------------------------------
size_t sW2A(char *mbstr,const wchar_t *wcstr,size_t count ){
	size_t i = 0;
	size_t ret = 0;
	while( i < count && wcstr[i]){
		if(wcstr[i] & 0xff00){
			BYTE* p = (BYTE*)&wcstr[i];
			mbstr[ret++] = p[1];
			mbstr[ret++] = p[0];
			i++;
		}else{
			mbstr[ret++] = (BYTE)(WORD)wcstr[i];
			i++;
		}
	}
	mbstr[ret] = 0;
	return ret;
}
//------------------------------------------------------------------------
int getcharcount(const char * str){
	size_t i = 0;
	size_t ret = 0;
	while(str[i]){
		if(str[i] & 0x80){
			if( str[i + 1] == 0) break;
			ret++;
			i+=2;
		}else{
			ret++;
			i++;
		}
	}
	return ret;
}
//------------------------------------------------------------------------
void str_split(const char* pszSrc,char* pszDst,size_t nDstBufLen,size_t nLineLen){
	size_t i;
	i =0;
	while(*pszSrc){
		if((*pszSrc) & 0x80){
			if(!(*(pszSrc+1))) break;
			if(i+1 >= nLineLen){

				if(nDstBufLen < 2) break;
				*pszDst++ = '\n';
				nDstBufLen--;
				i = 0;
			}
			if(nDstBufLen<3) break;
			*pszDst++ = *pszSrc++;
			*pszDst++ = *pszSrc++;
			nDstBufLen-=2;
			i+=2;
		}else{
			if(i >= nLineLen){

				if(nDstBufLen < 2) break;
				*pszDst++ = '\n';
				nDstBufLen--;
				i = 0;
			}
			if(nDstBufLen < 2) break;
			*pszDst++ = *pszSrc++;
			nDstBufLen--;
			i++;
		}
	}
	*pszDst = 0;
}
//------------------------------------------------------------------------
long str_getnextl(const char* pszString,char** endptr,int base){
	char* p;
	long l = strtol(pszString,&p,10);
	char* result = p;
	while( *p ){
		++p;
		result = p;
		strtol(result,&p,base);
		if(result != p) break;
	}
	if(endptr)
		*endptr = result;
	return l;
}
//------------------------------------------------------------------------
void to_lower(std::string &s){
	std::transform(s.begin(), s.end(),
		s.begin(),
		tolower);
}
//------------------------------------------------------------------------
void to_upper(std::string &s){
	std::transform(s.begin(), s.end(),
		s.begin(),
		toupper);
}
//------------------------------------------------------------------------
size_t string_key_hash::operator()(const std::string &x) const{
#ifdef _NOT_USE_STLPORT
	/*stdext::*/hash<const char *> H;
#else
	std::hash<const char *> H;
#endif
	return H(x.c_str());
}
//------------------------------------------------------------------------
size_t string_key_case_hash::operator()(const std::string &x) const{
	ZSTACK_ALLOCA(char*,ptmpbuf,x.length()+1);
	strcpy(ptmpbuf,x.c_str());
	strlwr(ptmpbuf);
#ifdef _NOT_USE_STLPORT
	/*stdext::*/hash<const char *> H;
#else
	std::hash<const char *> H;
#endif
	return H(ptmpbuf);
}
//------------------------------------------------------------------------
bool string_key_equal::operator()(const std::string &s1, const std::string &s2) const{
	return strcmp(s1.c_str(), s2.c_str()) == 0;	
}
//------------------------------------------------------------------------
bool string_key_case_equal::operator()(const std::string &s1, const std::string &s2) const{
	return stricmp(s1.c_str(), s2.c_str()) == 0;	
}
//------------------------------------------------------------------------
size_t pchar_key_hash::operator()(const char* x) const{
#ifdef _NOT_USE_STLPORT
	/*stdext::*/hash<const char *> H;
#else
	std::hash<const char *> H;
#endif
	return H(x);
}
//------------------------------------------------------------------------
size_t pchar_key_case_hash::operator()(const char* x) const{
	ZSTACK_ALLOCA(char*,ptmpbuf,strlen(x)+1);
	strcpy(ptmpbuf,x);
	strlwr(ptmpbuf);
#ifdef _NOT_USE_STLPORT
	/*stdext::*/hash<const char *> H;
#else
	std::hash<const char *> H;
#endif
	return H(ptmpbuf);
}
//------------------------------------------------------------------------
bool pchar_key_equal::operator()(const char* s1, const char* s2) const{
	return strcmp(s1, s2) == 0;	
}
//------------------------------------------------------------------------
bool pchar_key_case_equal::operator()(const char* s1, const char* s2) const{
	return stricmp(s1, s2) == 0;	
}
//------------------------------------------------------------------------