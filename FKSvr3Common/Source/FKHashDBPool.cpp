/**
*	created:		2013-4-7   22:57
*	filename: 		FKHashDBPool
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include <wchar.h>
#include "../Include/Database/FKHashDBPool.h"
#include "../Include/FKStringEx.h"
//------------------------------------------------------------------------
HashDBPool::HashDBPool(const char* tblnamefmt, int DBCount, int OneDBTblCount, int nhashcode)
{
	init(tblnamefmt, DBCount, OneDBTblCount, nhashcode);
}
//------------------------------------------------------------------------
void HashDBPool::init(const char* tblnamefmt, int DBCount, int OneDBTblCount, int nhashcode)
{
	stDBParam* pparam = &m_dbparams[nhashcode];
	pparam->m_nHashDBCount = DBCount;
	pparam->m_nOneHashDBTblCount = OneDBTblCount;
	pparam->m_nAllTblCount = pparam->m_nOneHashDBTblCount * pparam->m_nHashDBCount;
	ZeroMemory(pparam->m_sztblnamefmt, sizeof(pparam->m_sztblnamefmt));
	strcpy_s(pparam->m_sztblnamefmt, sizeof(pparam->m_sztblnamefmt) - 1, tblnamefmt);
}
//------------------------------------------------------------------------
const char* HashDBPool::gethash_tblname(const char* szaccount, unsigned int& ndbhashcode, unsigned int& nhashvalue, int nhashcode)
{
	stDBParam* pparam = &m_dbparams[nhashcode];
	nhashvalue = std_str_hash(szaccount, 128);
	int ntblid = nhashvalue % pparam->m_nAllTblCount;
	ndbhashcode = (ntblid / pparam->m_nOneHashDBTblCount);
	return vformat(pparam->m_sztblnamefmt, ntblid);
}
//------------------------------------------------------------------------
const char* HashDBPool::gethash_tblinfo(const char* szaccount, unsigned int& ntblid, unsigned int& ndbhashcode, unsigned int& nhashvalue, int nhashcode)
{
	stDBParam* pparam = &m_dbparams[nhashcode];
	nhashvalue = std_str_hash(szaccount, 128);
	ntblid = nhashvalue % pparam->m_nAllTblCount;
	ndbhashcode = (ntblid / pparam->m_nOneHashDBTblCount);
	return pparam->m_sztblnamefmt;
}
//------------------------------------------------------------------------