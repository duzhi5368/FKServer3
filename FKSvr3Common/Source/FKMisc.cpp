/**
*	created:		2013-4-8   15:44
*	filename: 		FKMisc
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../Include/FKMisc.h"
#include "../Include/FKStringEx.h"
#include "../Include/Dump/FKDumpErrorBase.h"
#include "../Include/FKLogger.h"
#include  <stdio.h>
//------------------------------------------------------------------------
#ifdef _DEBUG
#pragma comment( lib,"../FKSvr3Common/Lib/zlib_debug.lib")
#else
#pragma comment( lib,"../FKSvr3Common/Lib/zlib.lib")
#endif
//------------------------------------------------------------------------
#ifndef _WIN32
	//------------------------------------------------------------------------
	#include <termio.h>
	#ifndef STDIN_FILENO
		#define STDIN_FILENO 0
	#endif
	//------------------------------------------------------------------------
	int getch(void){
		struct termios tm, tm_old;
		int fd = STDIN_FILENO, c;
		if(tcgetattr(fd, &tm) < 0)
			return -1;
		tm_old = tm;
		cfmakeraw(&tm);
		if(tcsetattr(fd, TCSANOW, &tm) < 0)
			return -1;
		c = fgetc(stdin);
		if(tcsetattr(fd, TCSANOW, &tm_old) < 0)
			return -1;
		return c;
	}
#else                            
	#include <conio.h>
#endif	
//------------------------------------------------------------------------
#define BACKSPACE 8
#define ENTER     13
#define ALARM     7
//------------------------------------------------------------------------
char * getpasswd(const char *prompt,char * szInputbuf,int npasslen,char chpass){
	int i=0, ch;
	char* p=szInputbuf;
	printf("%s", prompt);
	while((ch = getch())!= -1 && ch != ENTER){
		if(i == npasslen && ch != BACKSPACE){
			putchar(ALARM);
			continue;
		}
		if(ch == BACKSPACE){
			if(i==0){
				putchar(ALARM);
				continue;
			}
			i--;
			putchar(BACKSPACE);
			putchar(' ');
			putchar(BACKSPACE);
		}else{
			p[i] = (char)ch;
			putchar(chpass);
			i++;
		}
	}
	if(ch == -1){
		while(i != -1){
			p[i--] = '\0';
		}
		return NULL;
	}
	p[i]='\0';
	printf("\n");
	return szInputbuf;
}
//------------------------------------------------------------------------
#include <shellapi.h>
//------------------------------------------------------------------------
BOOL GotoURL( const char* url, int showcmd )
{
	static bool sHaveKey = false;
	static char sWebKey[512];

	if ( !sHaveKey )
	{
		HKEY regKey;
		DWORD size = sizeof( sWebKey );

		if ( RegOpenKeyEx( HKEY_CLASSES_ROOT, "\\http\\shell\\open\\command", 0, KEY_QUERY_VALUE, &regKey ) != ERROR_SUCCESS )
		{

			return( false );
		}

		size = sizeof( sWebKey );
		if ( RegQueryValueEx( regKey, "", NULL, NULL, (unsigned char*)sWebKey, &size ) != ERROR_SUCCESS ) 
		{

			return( false );
		}

		RegCloseKey( regKey );
		sHaveKey = true;

		char *p = strstr(sWebKey, "\"%1\"");
		if (p) *p = 0;
	}

	STARTUPINFO si;
	memset( &si, 0, sizeof( si ) );
	si.cb = sizeof( si );

	char buf[1024];
	sprintf_s( buf, sizeof( buf ), "%s %s", (const char*) sWebKey, url );
	buf[1023] = 0;

	PROCESS_INFORMATION pi;
	memset( &pi, 0, sizeof( pi ) );
	if(!CreateProcess( NULL,
		buf, 
		NULL,
		NULL, 
		false, 
		CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP, 
		NULL, 
		NULL, 
		&si, 
		&pi ))
		return false;

	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	return true;
}
//------------------------------------------------------------------------
bool SetClipboardText(const char* lpszBuffer){
	bool bSuccess = false;
	if (::OpenClipboard(NULL)){
		::EmptyClipboard();
		int nSize = lstrlen(lpszBuffer);
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, nSize+1);
		if (hGlobal){
			LPSTR lpszData = (LPSTR) ::GlobalLock(hGlobal);
			if (lpszData){
				lstrcpy(lpszData, lpszBuffer);
				::GlobalUnlock(hGlobal);
				::SetClipboardData(CF_TEXT, hGlobal);
				bSuccess = TRUE;
			}


		}
		::CloseClipboard();
	}
	return bSuccess;
}
//------------------------------------------------------------------------
int compresszlib(unsigned char* pIn,unsigned long nInLen,unsigned char* pOut,unsigned long& pOutLen,int level){
	FUNCTION_BEGIN;

	z_stream_s zipStream;
	zipStream.zalloc = Z_NULL;
	zipStream.zfree  = Z_NULL;
	zipStream.opaque = Z_NULL;

	zipStream.next_in   = pIn;
	zipStream.avail_in  = nInLen;
	zipStream.total_in  = 0;
	zipStream.next_out  = pOut;
	zipStream.avail_out = pOutLen;
	zipStream.total_out = 0;

	int err;
	err = deflateInit(&zipStream, level);
	if (err==Z_OK){
		err = deflate(&zipStream, Z_FINISH);
		if (err == Z_STREAM_END) {
			pOutLen = zipStream.total_out;
			err = deflateEnd(&zipStream);
		}else{
			zipStream.avail_in = zipStream.avail_out = 1;
			int n=0;
			while (n<10){
				int tmperr = deflate(&zipStream, Z_FINISH);
				if (tmperr == Z_STREAM_END) break;
				n++;
			}
			if (n>=10)
			{	
				g_logger.forceLog(zLogger::zERROR,"zliberror: compresszlib error");
			}else
			{
				deflateEnd(&zipStream);
			}

			err = (err == Z_OK ? Z_BUF_ERROR : err);
		}
	}
	return err;
}
//------------------------------------------------------------------------
int uncompresszlib(unsigned char* pIn,unsigned long nInLen,unsigned char* pOut,unsigned long& pOutLen){
	FUNCTION_BEGIN;

	z_stream_s zipStream;
	zipStream.zalloc = Z_NULL;
	zipStream.zfree  = Z_NULL;
	zipStream.opaque = Z_NULL;

	zipStream.next_in   = pIn;
	zipStream.avail_in  = nInLen;
	zipStream.total_in  = 0;
	zipStream.next_out  = pOut;
	zipStream.avail_out = pOutLen;
	zipStream.total_out = 0;

	int err;
	err = inflateInit(&zipStream);
	if (err==Z_OK){
		err = inflate(&zipStream, Z_FINISH);
		if (err == Z_STREAM_END) {
			pOutLen = zipStream.total_out;
			err = inflateEnd(&zipStream);
		}else{
			zipStream.avail_in = zipStream.avail_out = 1;
			int n=0;
			while (n<10){
				int tmperr = inflate(&zipStream, Z_FINISH);
				if (tmperr == Z_STREAM_END) break;
				n++;
			}
			if (n>=10)
			{	
				g_logger.forceLog(zLogger::zERROR,"zliberror: uncompresszlib error");
			}else
			{
				inflateEnd(&zipStream);
			}	
			err = (err == Z_OK ? Z_BUF_ERROR : err);
		}
	}
	return err;
}
//------------------------------------------------------------------------
static bool static_bosrand=false;
//------------------------------------------------------------------------
unsigned int _random(unsigned int nMax,unsigned int nMin)
{
	if (!static_bosrand){
		srand( GetTickCount() );
		static_bosrand=true;
	}
	if(nMax>nMin){
		int nr1=((rand()<<16) | rand());
		int nr2=((rand()<<16) | rand());

		unsigned int nmod=(nMax-nMin+1);
		unsigned int nr=(abs(nr1-nr2));
		return nmod==0?nr:((nr % nmod) + nMin);
	}else{
		return nMin;
	}
}
//------------------------------------------------------------------------
unsigned int _random_d(unsigned int nMax,unsigned int nMin)
{
	if (!static_bosrand){
		srand( GetTickCount() );
		static_bosrand=true;
	}
	if(nMax>nMin){
		int nr1=((rand()<<16) | rand());
		int nr2=((rand()<<16) | rand());
		unsigned int nmod=(nMax-nMin);
		unsigned int nr=(abs(nr1-nr2));
		return nmod==0?nr:((nr % nmod) + nMin);
	}else{
		return nMin;
	}
}
//------------------------------------------------------------------------
HBITMAP CopyScreenToBitmap(LPRECT lpRect){
	RECT tmprect;
	if (lpRect==NULL){

		tmprect.left = 0;
		tmprect.top = 0;
		tmprect.right = GetSystemMetrics(SM_CXSCREEN);
		tmprect.bottom = GetSystemMetrics(SM_CYSCREEN);


		lpRect=&tmprect; 
	}

	HDC hScrDC, hMemDC;      

	HBITMAP hBitmap=0,hOldBitmap=0;   
	int       nX, nY, nX2, nY2;      
	int       nWidth, nHeight;      
	int       xScrn, yScrn;         

	if (IsRectEmpty(lpRect))
		return NULL;

	hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);
	if (hScrDC==0){return 0;}

	hMemDC = CreateCompatibleDC(hScrDC);
	if (hMemDC==0){return 0;}

	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	xScrn = GetDeviceCaps(hScrDC, HORZRES);
	yScrn = GetDeviceCaps(hScrDC, VERTRES);

	if (nX < 0)	nX = 0;
	if (nY < 0)	nY = 0;
	if (nX2 > xScrn)nX2 = xScrn;
	if (nY2 > yScrn)nY2 = yScrn;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;

	hBitmap=CreateCompatibleBitmap(hScrDC,nWidth,nHeight);
	if (hBitmap==0){return 0;}

	hOldBitmap=(HBITMAP)SelectObject(hMemDC,hBitmap);

	BitBlt(hMemDC,0,0, nWidth,nHeight,hScrDC, nX, nY, SRCCOPY);

	hBitmap=(HBITMAP)SelectObject(hMemDC,hOldBitmap);

	DeleteDC(hScrDC);
	DeleteDC(hMemDC);


	return hBitmap;
}
//------------------------------------------------------------------------
BOOL SaveBitmapToFile(HBITMAP hBitmap, LPSTR lpFileName)
{   
	HDC     			hDC;         
	int     			iBits;      
	WORD    			wBitCount;   
	DWORD           	dwPaletteSize=0,dwBmBitsSize,dwDIBSize, dwWritten;
	BITMAP          	Bitmap;        
	BITMAPFILEHEADER   	bmfHdr;        
	BITMAPINFOHEADER   	bi;            
	LPBITMAPINFOHEADER 	lpbi;          
	HANDLE       		fh, hDib, hPal;
	HPALETTE     		hOldPal=NULL;

	hDC = CreateDC("DISPLAY",NULL,NULL,NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)wBitCount = 1;
	else if (iBits <= 4)wBitCount = 4;
	else if (iBits <= 8)wBitCount = 8;
	else if (iBits <= 24)wBitCount = 24;
	else  wBitCount = 32;

	if (wBitCount <= 8)	dwPaletteSize=(1<<wBitCount)*sizeof(RGBQUAD);

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	bi.biSize            = sizeof(BITMAPINFOHEADER);
	bi.biWidth           = Bitmap.bmWidth;
	bi.biHeight          = Bitmap.bmHeight;
	bi.biPlanes          = 1;
	bi.biBitCount         = wBitCount;
	bi.biCompression      = BI_RGB;
	bi.biSizeImage        = 0;
	bi.biXPelsPerMeter     = 0;
	bi.biYPelsPerMeter     = 0;
	bi.biClrUsed         = 0;
	bi.biClrImportant      = 0;

	dwBmBitsSize = ((Bitmap.bmWidth*wBitCount+31)/32)*4*Bitmap.bmHeight;

	hDib  = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));
	if (hDib==0){return FALSE;}
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	if (lpbi==0){return FALSE;}
	*lpbi = bi;

	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal){
		hDC = ::GetDC(NULL);
		hOldPal=SelectPalette(hDC,(HPALETTE)hPal,FALSE);
		RealizePalette(hDC);
	}

	GetDIBits(hDC,hBitmap,0,(UINT)Bitmap.bmHeight,(LPSTR)lpbi+sizeof(BITMAPINFOHEADER)+dwPaletteSize, (BITMAPINFO *)lpbi,DIB_RGB_COLORS);

	if (hOldPal){
		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	fh=CreateFile(lpFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh==INVALID_HANDLE_VALUE)return FALSE;

	bmfHdr.bfType = 0x4D42;  
	dwDIBSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize;  
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

	WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize , &dwWritten, NULL); 

	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}
//------------------------------------------------------------------------