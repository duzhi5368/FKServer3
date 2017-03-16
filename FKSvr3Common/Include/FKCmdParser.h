/**
*	created:		2013-4-8   13:20
*	filename: 		FKCmdParser
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include <sstream>
#include <algorithm>
#include "FKBaseDefine.h"
#include "FKStringEx.h"
#include <hash_map>
#include <malloc.h>
//------------------------------------------------------------------------
// 属性关联类容器，所有属性关键字和值都使用字符串代表，关键字不区分大小写
class CCmdlineParse
{
public:
	const std::string &getProperty(const std::string &key)
	{
		return properties[key];
	}

	void setProperty(const std::string &key, const std::string &value)
	{
		properties[key] = value;
	}

	std::string & operator[] (const std::string &key)
	{
		return properties[key];
	}

	void dump(std::ostream &out)
	{

		if (m_getcmd){ out<< m_cmd << " ";}
		property_hashtype::const_iterator it;
		for(it = properties.begin(); it != properties.end(); it++)
			out << it->first << "=\"" << it->second << "\" ";
	}

	std::string & strcmd(){return m_cmd;}
	const char* cmd(){return m_cmd.c_str();}

	unsigned int parseCmdLine(const char *cmdLine)
	{
		int nret=0;
		m_cmd="";
		m_getcmd=false;
		CEasyStrParse parse;
		int nlen=strlen(cmdLine);
		STACK_ALLOCA(char*,pcmd,nlen+1);
		strcpy_s(pcmd,nlen+1,cmdLine);
		parse.SetParseStr(pcmd," \x9,;:","\"\"",'"');
		char* pd=NULL;
		for (int i=0;i<parse.ParamCount();i++)
		{
			pd=strchr(parse[i],'=');
			if (pd!=NULL)
			{
				*pd=0;pd++;
				int nlen2=strlen(pd);
				if (nlen2>=2 && pd[0]=='"' && pd[nlen2-1]=='"')
				{
					*pd=0;pd[nlen2-1]=0;pd++;	
				}
				if (strlen(pd)>0)
				{
					properties[parse[i]]=pd;
					nret++;
				}
			}
			else
			{
				m_cmd=parse[i];
				m_getcmd=true;
			}
		}
		return nret;
	}

	CCmdlineParse(){ m_cmd="";m_getcmd=false; }
protected:
#ifdef _NOT_USE_STLPORT
	typedef stdext::hash_map<std::string, std::string> property_hashtype;
#else
	typedef std::hash_map<std::string, std::string, string_key_case_hash, string_key_case_equal> property_hashtype;
#endif


	property_hashtype properties;			 
	std::string m_cmd; 
	bool m_getcmd;
};
//------------------------------------------------------------------------