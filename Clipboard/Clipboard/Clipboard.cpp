// ConsoleApplication2.cpp : Defines the entry point for the console application.
//



#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <Windows.h>
using namespace std;

BOOL SaveBitmapToFile(HBITMAP hBitmap, const string & szfilename);
wstring& operator + (wstring & wstr, const int i);
string operator + (string str, const int i);

int main(int argc, char * argv[])
{
	if(argc != 3)
	{
		cout << _T("Please input FileName & BeginingNumber!") << endl;
		return 0;
	}

	cout << _T("If you find any bug or advice please contact moon.wang@sap.com") << endl;

	string path = argv[1];
	int i = atoi(argv[2]);

	while (1)
	{
		if (IsClipboardFormatAvailable(CF_BITMAP)) 
		{
			string fileName = path + i++ + _T(".bmp");

			if(!OpenClipboard(NULL))
			{
				MessageBox(NULL, _T("OpenClipboard"), _T("OpenClipboard"), 0);
				continue;
			}

			HBITMAP h = (HBITMAP) ::GetClipboardData(CF_BITMAP);
			if(!h) 
			{
				MessageBox(NULL, _T("GetClipboardData"), _T("GetClipboardData"), 0);
				continue;
			}
			SaveBitmapToFile(h, fileName);
			EmptyClipboard();
			CloseClipboard();
		}
	} 

	system("pause");
	return 0;
}

BOOL SaveBitmapToFile(HBITMAP   hBitmap, const string & szfilename)
{
	HDC hDC; 
	int iBits; 
	WORD wBitCount; 
	DWORD dwPaletteSize = 0; 

	DWORD dwBmBitsSize;
	DWORD dwDIBSize, dwWritten;
	BITMAP Bitmap;
	BITMAPFILEHEADER bmfHdr; 
	BITMAPINFOHEADER bi; 
	LPBITMAPINFOHEADER lpbi;   
	HANDLE fh, hDib, hPal, hOldPal = NULL; 

	hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
	{
		wBitCount = 1;
	}
	else if (iBits <= 4)
	{
		wBitCount = 4;
	}
	else if (iBits <= 8)
	{
		wBitCount = 8;
	}
	else if (iBits <= 24)
	{
		wBitCount = 24;
	}
	else if (iBits <= 32)
	{
		wBitCount = 32;
	}

	if (wBitCount <= 8)
	{
		dwPaletteSize = (1 << wBitCount) * sizeof(RGBQUAD);
	}

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	if(!hDib) 
	{
		MessageBox(NULL, _T("hDib"), NULL, 0);
		return false;
	}

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	if (!lpbi)
	{
		MessageBox(NULL, _T("lpbi"), NULL, 0);
		return false;
	}
	
	*lpbi = bi;
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)+dwPaletteSize, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

	if (hOldPal)
	{
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	fh = CreateFile(szfilename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "CreateFile", NULL, 0);
		return false;
	}

	bmfHdr.bfType = 0x4D42;  
	dwDIBSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+dwPaletteSize;
	
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return   true;
}

int SetClipboardContent()
{
	HGLOBAL hMemory;
	LPTSTR lpMemory;
	char * content = "test"; 
	int contentSize = strlen(content) + 1;

	if (!OpenClipboard(NULL)) 
	{
		puts("Cannot open the clipboard!");
		return 1;
	}

	if (!EmptyClipboard())  
	{
		puts("Cannot empty the clipboard!");
		CloseClipboard();
		return 1;
	}

	if ((hMemory = GlobalAlloc(GMEM_MOVEABLE, contentSize)) == NULL)  
	{
		puts("GlobalAlloc failed!");
		CloseClipboard();
		return 1;
	}

	if ((lpMemory = (LPTSTR)GlobalLock(hMemory)) == NULL) 
	{
		puts("GlobalLock failed!");
		CloseClipboard();
		return 1;
	}

	memcpy_s(lpMemory, contentSize, content, contentSize);

	GlobalUnlock(hMemory); 

	if (SetClipboardData(CF_TEXT, hMemory) == NULL)
	{
		puts("SetClipboardData failed!");
		CloseClipboard();
		return 1;
	}
	return 0;
}


wstring& operator + (wstring & wstr, const int i)
{
	wchar_t buf[10];
	_itow_s(i, buf,10, 10);
	wstr += buf;
	return wstr;
}
string operator + (string str, const int i)
{
	char buf[10];
	_itoa_s(i, buf, 10, 10);
	str += buf;
	return str;
}