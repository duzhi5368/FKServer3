/**
*	created:		2013-4-8   15:52
*	filename: 		FKWinFileIO
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKWinFileIO.h"
#include "../Include/FKOutput.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include <vector>
#include <time.h>
#include <sys/utime.h>
//------------------------------------------------------------------------
bool FileSystem::delfile(const char* fileName){
	if(fileName == NULL) return false;
	DWORD dwAttrib = GetFileAttributes(fileName);
	if(dwAttrib == INVALID_FILE_ATTRIBUTES) return true;
	if((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) !=0) return false;

	dwAttrib &= (~FILE_ATTRIBUTE_READONLY);
	dwAttrib &= (~FILE_ATTRIBUTE_SYSTEM);
	::SetFileAttributes(fileName,dwAttrib); 
	if(!::DeleteFile(fileName)){

		return false;
	}
	return true;
}
//------------------------------------------------------------------------
bool FileSystem::replacefile(const char* srcFileName,const char* dstFileName,bool delsrc)
{
	if(FileSystem::IsFileExist(dstFileName)){
		if(!delfile((char *)dstFileName)){
			return false;
		}
	}
	if (delsrc){
		if(!::MoveFile(srcFileName,dstFileName)) return false;
	}else{
		if(!::CopyFile(srcFileName,dstFileName,true)) return false;
	}
	return true;
}
//------------------------------------------------------------------------
bool FileSystem::createPath(const char *srcfile)
{
	char filebuf[MAX_PATH];
	ZeroMemory(filebuf,sizeof(filebuf));
	strcpy_s(filebuf,sizeof(filebuf)-2,srcfile);

	replaceFrontlashPath(filebuf);
	if (filebuf[strlen(filebuf)-1]!='\\'){
		strcat(filebuf,"\\");
	}
	const char* file=&filebuf[0];
	char pathbuf[MAX_PATH];
	const char *dir;
	ZeroMemory(pathbuf,sizeof(pathbuf));
	DWORD pathLen = 0;
	bool ret=false;
	while((dir = strchr(file, '\\')) != NULL){
		strncpy_s(pathbuf + pathLen,MAX_PATH-pathLen-1 ,file, dir - file);
		pathbuf[pathLen + dir-file] = 0;			
		if (strlen((const char *)&pathbuf[pathLen])==0){
			file = dir + 1;
			continue;
		}
		if (!((strcmp((const char *)&pathbuf[pathLen],".")==0) || (strcmp((const char *)&pathbuf[pathLen],"..")==0) )){
			if (!FileSystem::IsDirExist(pathbuf)){
				ret = (CreateDirectory(pathbuf, NULL)==TRUE);
				if (!ret)	  {  break;  }
			}
		}
		pathLen += dir - file;
		pathbuf[pathLen++] = '\\';
		file = dir + 1;
	}
	return ret;
}
//------------------------------------------------------------------------
File::File()
: currentStatus(Closed), capability(0)
{
	handle = (void *)INVALID_HANDLE_VALUE;
}
//------------------------------------------------------------------------
File::~File()
{
	close();
	handle = (void *)INVALID_HANDLE_VALUE;
}
//------------------------------------------------------------------------
File::Status File::close()
{
	if (Closed == currentStatus)
		return currentStatus;

	if (INVALID_HANDLE_VALUE != (HANDLE)handle)
	{
		if (0 == CloseHandle((HANDLE)handle))
			return setStatus();                                    
	}
	handle = (void *)INVALID_HANDLE_VALUE;
	return currentStatus = Closed;
}
//------------------------------------------------------------------------
File::Status File::open(const char *filename, const AccessMode openMode,bool randomAccess)
{
	char filebuf[MAX_PATH];
	strcpy(filebuf , filename);
	replaceFrontlashPath(filebuf);
	filename = filebuf;

	if (Closed != currentStatus)
		close();

	DWORD dwFlags = (randomAccess ? 0 : FILE_FLAG_SEQUENTIAL_SCAN);
	switch (openMode)
	{
	case Read:
		handle = (void *)CreateFile(filename,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | dwFlags,
			NULL);
		break;
	case Write:
		handle = (void *)CreateFile(filename,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | dwFlags,
			NULL);
		break;
	case ReadWrite:
		handle = (void *)CreateFile(filename,
			GENERIC_WRITE | GENERIC_READ,
			0,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | dwFlags,
			NULL);
		break;
	case WriteAppend:
		handle = (void *)CreateFile(filename,
			GENERIC_WRITE,
			0,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | dwFlags,
			NULL);
		break;

	default:
		assert( 0 && "File::open: bad access mode");    
	}

	if (INVALID_HANDLE_VALUE == (HANDLE)handle)                
		return setStatus();
	else
	{
		switch (openMode)
		{
		case Read:
			capability = DWORD(FileRead);
			break;
		case Write:
		case WriteAppend:
			capability = DWORD(FileWrite);
			break;
		case ReadWrite:
			capability = DWORD(FileRead)  |
				DWORD(FileWrite);
			break;
		default:
			assert( 0 && "File::open: bad access mode");
		}
		return currentStatus = Ok;                                
	}
}
//------------------------------------------------------------------------
File::Status File::getStatus() const
{
	return currentStatus;
}
//------------------------------------------------------------------------
File::Status File::setStatus()
{
	switch (GetLastError())
	{
	case ERROR_INVALID_HANDLE:
	case ERROR_INVALID_ACCESS:
	case ERROR_TOO_MANY_OPEN_FILES:
	case ERROR_FILE_NOT_FOUND:
	case ERROR_SHARING_VIOLATION:
	case ERROR_HANDLE_DISK_FULL:
		return currentStatus = IOError;

	default:
		return currentStatus = UnknownError;
	}
}
//------------------------------------------------------------------------
File::Status File::setStatus(File::Status status)
{
	return currentStatus = status;
}
//------------------------------------------------------------------------
File::Status File::read(void *dst,DWORD size,  DWORD *bytesRead)
{
	assert(Closed != currentStatus);
	assert(INVALID_HANDLE_VALUE != (HANDLE)handle);
	assert(NULL != dst);
	assert(hasCapability(FileRead));

	if (Ok != currentStatus || 0 == size)
		return currentStatus;
	else
	{
		DWORD lastBytes;
		DWORD *bytes = (NULL == bytesRead) ? &lastBytes : (DWORD *)bytesRead;
		if (0 != ReadFile((HANDLE)handle, dst, size, bytes, NULL))
		{
			if(*((DWORD *)bytes) != size)
				return currentStatus = EOS;                        
		}
		else
			return setStatus();                                    
	}
	return currentStatus = Ok;                                    
}
//------------------------------------------------------------------------
File::Status File::write(const void *src,DWORD size,  DWORD *bytesWritten)
{
	assert(Closed != currentStatus);
	assert(INVALID_HANDLE_VALUE != (HANDLE)handle);
	assert(NULL != src);
	assert(hasCapability(FileWrite));

	if ((Ok != currentStatus && EOS != currentStatus) || 0 == size)
		return currentStatus;
	else
	{
		DWORD lastBytes;
		DWORD *bytes = (NULL == bytesWritten) ? &lastBytes : (DWORD *)bytesWritten;
		if (0 != WriteFile((HANDLE)handle, src, size, bytes, NULL))
			return currentStatus = Ok;                            
		else
			return setStatus();                                    
	}
}
//------------------------------------------------------------------------
bool File::hasCapability(Capability cap) const
{
	return (0 != (DWORD(cap) & capability));
}
//------------------------------------------------------------------------
File::Status File::setPosition(DWORD position, bool absolutePos)
{
	assert(Closed != currentStatus);
	assert(INVALID_HANDLE_VALUE != (HANDLE)handle);

	if (Ok != currentStatus && EOS != currentStatus)
		return currentStatus;

	DWORD finalPos;
	if(absolutePos)
	{
		assert(0 <= position);
		finalPos = SetFilePointer((HANDLE)handle,
			position,
			NULL,
			FILE_BEGIN);
	}
	else
	{
		assert((getPosition() >= (DWORD)abs((long)position) && 0 > position) || 0 <= position);
		finalPos = SetFilePointer((HANDLE)handle,
			position,
			NULL,
			FILE_CURRENT);
	}

	if (0xffffffff == finalPos)
		return setStatus();                                        
	else if (finalPos >= getSize())
		return currentStatus = EOS;                                
	else
		return currentStatus = Ok;                                
}
//------------------------------------------------------------------------
DWORD File::getPosition() const
{
#if (_MSC_VER >= 1400)
	AssertFatal(Closed != currentStatus, wszErrorMsg);

	AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, wszErrorMsg);

	return SetFilePointer((HANDLE)handle,
		0,                                    
		NULL,                                    
		FILE_CURRENT);                        
#else
	AssertFatal(Closed != currentStatus, "File::getPosition: file closed");
	AssertFatal(INVALID_HANDLE_VALUE != (HANDLE)handle, "File::getPosition: invalid file handle");

	return SetFilePointer((HANDLE)handle,
		0,                                    
		NULL,                                    
		FILE_CURRENT);                        
#endif
}
//------------------------------------------------------------------------
DWORD File::getSize() const
{
	assert(INVALID_HANDLE_VALUE != (HANDLE)handle);

	if (Ok == currentStatus || EOS == currentStatus)
	{
		DWORD high;
		return GetFileSize((HANDLE)handle, &high);                
	}
	else
		return 0;                                                
}
//------------------------------------------------------------------------
File::Status File::flush()
{
	assert(Closed != currentStatus);
	assert(INVALID_HANDLE_VALUE != (HANDLE)handle);
	assert(hasCapability(FileWrite));

	if (0 != FlushFileBuffers((HANDLE)handle))
		return setStatus();                                        
	else
		return currentStatus = Ok;                                
}
//------------------------------------------------------------------------