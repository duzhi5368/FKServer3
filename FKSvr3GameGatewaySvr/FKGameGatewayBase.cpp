/**
*	created:		2013-4-9   17:14
*	filename: 		FKGameGatewayBase
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "FKGameGatewayBase.h"
//------------------------------------------------------------------------
int	CGatewayConnManage::getemptyindex(){
	AILOCKT(*this);
	if (!m_v.empty()){
		while (!m_idxs.empty()){
			WORD nret=m_idxs.front();
			m_idxs.pop();
			if (nret>=0 && nret<m_v.size()){
				if (m_v[nret].sidx==INVALID_SOCKET && m_v[nret].obj==NULL){
					return nret;
				}
			}
		}
		for (size_t i=0;i<m_v.size();i++){
			if (m_v[i].sidx==INVALID_SOCKET && m_v[i].obj==NULL){
				return i;
			}
		}
	}
	return -1;
}
//------------------------------------------------------------------------
CGatewayConnManage::CGatewayConnManage(){
	m_count=0;
	m_v.reserve(32);
}
//------------------------------------------------------------------------
CGatewayConnManage::~CGatewayConnManage(){
	m_count=0;
	m_v.clear();
}
//------------------------------------------------------------------------
int CGatewayConnManage::add(long sidx,void* po){
	AILOCKT(*this);
	if ( sidx!=INVALID_SOCKET && m_count<30000 ){
		int nret=getemptyindex();
		if (nret<0){
			CGatewayConn temp;
			temp.obj=po;
			temp.sidx=sidx;
			m_v.push_back(temp);
			nret=m_v.size()-1;
		}else {
			m_v[nret].obj=po;
			m_v[nret].sidx=sidx;	
		}
		m_count++;
		return nret;
	}
	return -1;
}
//------------------------------------------------------------------------
bool CGatewayConnManage::modify(int nidx,long sidx,void* po){
	AILOCKT(*this);
	if (nidx>=0 && ((size_t)nidx)<m_v.size()){ if (m_v[nidx].sidx==sidx){ m_v[nidx].obj=po; return true; } }
	nidx=getindex(sidx);
	if (nidx>=0){ m_v[nidx].obj=po; return true; }
	return false;
}
//------------------------------------------------------------------------
int CGatewayConnManage::getindex(long sidx){
	if (sidx==-1){ return -1; }
	AILOCKT(*this);
	for (size_t i=0;i<m_v.size();i++){
		if (m_v[i].sidx==sidx){return i;}
	}
	return -1;
}
//------------------------------------------------------------------------
int CGatewayConnManage::remove(int nidx,long sidx){
	AILOCKT(*this);
	if (nidx>=0 && ((size_t)nidx)<m_v.size()){
		if (m_v[nidx].sidx==sidx){
			m_v[nidx].sidx=INVALID_SOCKET;
			m_v[nidx].obj=NULL;
			m_idxs.push((WORD)nidx);
			m_count--;
			return nidx;
		}
	}
	nidx=getindex(sidx);
	if (nidx>=0){
		m_v[nidx].sidx=INVALID_SOCKET;
		m_v[nidx].obj=NULL;
		m_idxs.push((WORD)nidx);
		m_count--;
		return nidx;
	}
	return -1;
}
//------------------------------------------------------------------------
int CGatewayConnManage::removeall(){
	AILOCKT(*this);
	for (size_t i=0;i<m_v.size();i++){m_v[i].sidx=INVALID_SOCKET;m_v[i].obj=NULL;}
	m_count=0;
	return m_v.size();
}
//------------------------------------------------------------------------