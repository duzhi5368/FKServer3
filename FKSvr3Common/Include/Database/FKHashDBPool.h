/**
*	created:		2013-4-7   22:53
*	filename: 		FKHashDBPool
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKDBConnPool.h"
#include "../FKSingleton.h"
#include "../RTTI/FKReflect.h"
#include <malloc.h>
//------------------------------------------------------------------------
struct stDBParam
{
	int		m_nHashDBCount;
	int		m_nOneHashDBTblCount;		//每个数据库几个表
	int		m_nAllTblCount;				//一共分散在多少个表里
	char	m_sztblnamefmt[MAX_PATH];
};
//------------------------------------------------------------------------
class HashDBPool : public CLD_DBConnPool
{
public:
	std::map<int,stDBParam>	m_dbparams;
public:
	HashDBPool( const char* tblnamefmt = "mydb_name_nameex_tbl%d", int DBCount = 1, int OneDBTblCount = 1, int nhashcode = 0);
public:
	void		init			(const char* tblnamefmt, int DBCount, int OneDBTblCount, int nhashcode = 0);
	const char* gethash_tblname	(const char* szaccount, unsigned int& ndbhashcode, unsigned int& nhashvalue, int nhashcode = 0);
	const char* gethash_tblinfo	(const char* szaccount, unsigned int& ntblid, unsigned int& ndbhashcode, unsigned int& nhashvalue, int nhashcode = 0);
};
//------------------------------------------------------------------------