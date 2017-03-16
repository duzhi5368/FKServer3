/**
*	created:		2013-3-22   23:13
*	filename: 		FKWinFileIO
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#pragma once
//------------------------------------------------------------------------
#include "FKBaseDefine.h"
#include <vector>
#include "FKStringEx.h"
//------------------------------------------------------------------------
namespace FileSystem
{
	bool delfile(const char* fileName=NULL);
	bool createPath(const char *srcfile);
	bool replacefile(const char* srcFileName,const char* dstFileName,bool delsrc=true);
	static __inline bool IsDirExist(const char* pszDir)
	{
		DWORD dwFA = GetFileAttributes(pszDir);
		return ((dwFA != 0xffffffff) && (dwFA & FILE_ATTRIBUTE_DIRECTORY) );
	}
	static __inline bool IsFileExist(const char* pszFileName)
	{
		DWORD dwFA = GetFileAttributes(pszFileName);
		return ( (dwFA != INVALID_FILE_ATTRIBUTES) && (dwFA & FILE_ATTRIBUTE_DIRECTORY)== 0 );
	}
	static __inline size_t GetFileSize(const char* pszFileName){
		HANDLE hFd;
		WIN32_FIND_DATA fd;
		memset(&fd,0,sizeof(fd));
		hFd = FindFirstFile(pszFileName,&fd);
		if((hFd != INVALID_HANDLE_VALUE) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			return fd.nFileSizeLow;
		}
		return 0;
	}

	template < class _PARAM=DWORD >
	struct stEnumFileInfo{
		DWORD fileattributes;
		DWORD size;
		char szName[MAX_PATH];
		_PARAM param;

		stEnumFileInfo(){fileattributes=0;size=0;ZeroMemory(szName,sizeof(szName));};
		bool isdir(){return (fileattributes & FILE_ATTRIBUTE_DIRECTORY)!=0;}
		bool ishide(){return (fileattributes & FILE_ATTRIBUTE_HIDDEN)!=0;}
		bool issys(){return (fileattributes & FILE_ATTRIBUTE_SYSTEM)!=0;}
	};

	template < class _PARAM,class _IfFunc >
	void EnumFiles(LPCSTR szBaseDir,std::vector< stEnumFileInfo< _PARAM > > & aPack,_IfFunc pfnfilefilter,bool SearchSubDir=true,const char* fileext="*.*",bool addcannotsee=false,bool adddir=false)
	{
		if (szBaseDir==NULL){return;}
		char s[MAX_PATH];
		strcpy(s,szBaseDir);
		if (s[strlen(s)-1]!='\\' && s[strlen(s)-1]!='/'){strcat(s,"\\");}
		replaceFrontlashPath(s);
		char szDir[MAX_PATH];
		strcpy(szDir,s);
		strcat(s,fileext);
		WIN32_FIND_DATA fd;
		memset(&fd,0,sizeof(fd));
		HANDLE hFd;
		hFd = FindFirstFile(s,&fd);
		BOOL b = (hFd != INVALID_HANDLE_VALUE);
		while(b){
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if(strcmp(fd.cFileName,".") == 0 || strcmp(fd.cFileName,"..")==0){
					b = FindNextFile(hFd,&fd);
					continue;
				}
				if((!( (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))) || addcannotsee) {
					stEnumFileInfo< _PARAM > info;
					info.fileattributes=fd.dwFileAttributes;
					info.size=0;
					sprintf(info.szName,"%s%s",szDir,fd.cFileName);
					strlwr(info.szName);
					replaceFrontlashPath(info.szName);
					if(pfnfilefilter(info)){
						if (adddir){aPack.push_back(info);}		
						if(SearchSubDir){
							EnumFiles(info.szName,aPack,pfnfilefilter,SearchSubDir,fileext,addcannotsee,adddir);
						}
					}
				}
			}else if(fd.cFileName[0]){
				if((!( (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))) || addcannotsee) {
					stEnumFileInfo< _PARAM > info;

					info.fileattributes=fd.dwFileAttributes;
					info.size =fd.nFileSizeLow;
					sprintf(info.szName,"%s%s",szDir,fd.cFileName);
					strlwr(info.szName);
					replaceFrontlashPath(info.szName);
					if(pfnfilefilter(info)){aPack.push_back(info);}
				}
			}
			b = FindNextFile(hFd,&fd);
		}
		FindClose(hFd);
	}
};
//------------------------------------------------------------------------
class File  
{
public:
	enum Status
	{
		Ok = 0,           
		IOError,          
		EOS,              
		IllegalCall,      
		Closed,           
		UnknownError      
	};
	enum AccessMode
	{
		Read         = 0,  
		Write        = 1,  
		ReadWrite    = 2,  
		WriteAppend  = 3   
	};
	enum Capability
	{
		FileRead         = 1<<0,
		FileWrite        = 1<<1
	};
private:
	HANDLE handle;           
	Status currentStatus;   
	DWORD capability;         
protected:
	Status setStatus();                 
	Status setStatus(Status status);    
public:
	bool hasCapability(Capability cap) const;
	File::Status read(void *dst,DWORD size,  DWORD *bytesRead = NULL);
	Status write(const void *src,DWORD size,  DWORD *bytesWritten = NULL);
	Status getStatus() const;
	Status close();
	Status open(const char *filename, const AccessMode openMode = Read,bool randomAccess = false);
	Status setPosition(DWORD position, bool absolutePos = true);
	DWORD getPosition() const;
	DWORD getSize() const;
	Status flush();
	void* getHandle() { return handle; }
public:
	File();
	~File();
};
//------------------------------------------------------------------------