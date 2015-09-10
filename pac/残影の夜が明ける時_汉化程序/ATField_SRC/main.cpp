#include <Windows.h>
#include <string>
#include <locale.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
#include "detours.h"
#pragma comment(lib,"shlwapi.lib")

#define HEAD_STRING "$TEXT_LIST__"
typedef unsigned char byte;
typedef unsigned int uint;

#define RET_NUL return 0;
HMODULE hPAL;
void* org_func;

vector <char*> new_text;

typedef HFONT (WINAPI *fnCreateFont)(
int nHeight, // logical height of font height
int nWidth, // logical average character width
int nEscapement, // angle of escapement
int nOrientation, // base-line orientation angle
int fnWeight, // font weight
DWORD fdwItalic, // italic attribute flag
DWORD fdwUnderline, // underline attribute flag
DWORD fdwStrikeOut, // strikeout attribute flag
DWORD fdwCharSet, // character set identifier
DWORD fdwOutputPrecision, // output precision
DWORD fdwClipPrecision, // clipping precision
DWORD fdwQuality, // output quality
DWORD fdwPitchAndFamily, // pitch and family
LPCTSTR lpszFace // pointer to typeface name string
);

typedef FARPROC (WINAPI * fnGetProcAddress)(HMODULE hModule,LPCSTR lpFuncName);
fnCreateFont pfnCreateFont;
fnGetProcAddress pfnGetProcAddress;

extern "C" __declspec(dllexport) void Init_Dll(){};

char szFontName[256];


long get_file_size(FILE* f){
	long size_of_file;
	fseek(f, 0, SEEK_END);
	size_of_file = ftell(f);
	fseek(f,0,SEEK_SET);
	return size_of_file;
}
#define _read_uint(pt,uv) \
	uv = *(uint*)pt; \
	pt += sizeof(uint) 

char* _read_str(byte** pt)
{
	char* ptx;
	ptx = (char*)*pt;
	*pt += strlen(ptx) +1;

	return ptx;
}


void dec_char(byte* c)
{
	byte* pt;
	uint strcount;
	uint strid;

	uint curid;
	char* cursc;

	pt = c;

	pt += sizeof(HEAD_STRING) - 1;

	_read_uint(pt,strcount);


	for(strid = 0;strid<strcount;strid++)
	{
		_read_uint(pt,curid);
		cursc = _read_str(&pt);

		new_text.push_back(cursc);

	}
}
void do_text()
{
	byte * data;
	long size_of_file;

	FILE* f;


	f = fopen("ATField.DAT","rb");
	if(!f) return;

	size_of_file = get_file_size(f);

	data = (byte*)malloc(size_of_file);

	fread(data,size_of_file,1,f);

	fclose(f);

	if(memcmp(data,HEAD_STRING,sizeof(HEAD_STRING)-1)==0)
	{
		dec_char(data);
	}
}
byte* (*func_ret)(int v1,int v2,int v3,int v4);
void WINAPI retree_text(byte* trx_text)
{
	ULONG index = *(ULONG*)trx_text;
	trx_text += sizeof(ULONG);

	strcpy((char*)trx_text,new_text[index]);
}
byte* get_chars(int v1,int v2,int v3,int v4)
{
	byte* xf;
	xf = func_ret(v1,v2,v3,v4);

	if((v3 & 0x10000000) || v4 == 0xFFFFFFF)
		return xf;
	retree_text(xf);
	return xf;
}
void SetNopCode(LPVOID base_addr,SIZE_T fill_size)
{
	BYTE nNopCode = 0x90;
	for(SIZE_T i=0;i<fill_size;i++)
	{
		PBYTE wt_addr = (PBYTE)base_addr + i;
		WriteProcessMemory(GetCurrentProcess(),wt_addr,&nNopCode,1,NULL);
	}
}


HFONT WINAPI MyCreateFontA(
int nHeight, // logical height of font height
int nWidth, // logical average character width
int nEscapement, // angle of escapement
int nOrientation, // base-line orientation angle
int fnWeight, // font weight
DWORD fdwItalic, // italic attribute flag
DWORD fdwUnderline, // underline attribute flag
DWORD fdwStrikeOut, // strikeout attribute flag
DWORD fdwCharSet, // character set identifier
DWORD fdwOutputPrecision, // output precision
DWORD fdwClipPrecision, // clipping precision
DWORD fdwQuality, // output quality
DWORD fdwPitchAndFamily, // pitch and family
LPCTSTR lpszFace // pointer to typeface name string
)
{
	if(fdwCharSet == 0x80)
	{
		fdwCharSet = 0;
		fnWeight = FW_SEMIBOLD;
		return pfnCreateFont(nHeight,nWidth,nEscapement,nOrientation,fnWeight,fdwItalic,fdwUnderline,fdwStrikeOut,fdwCharSet,fdwOutputPrecision,fdwClipPrecision,fdwQuality,fdwPitchAndFamily,szFontName);
	}
	
	return pfnCreateFont(nHeight,nWidth,nEscapement,nOrientation,fnWeight,fdwItalic,fdwUnderline,fdwStrikeOut,fdwCharSet,fdwOutputPrecision,fdwClipPrecision,fdwQuality,fdwPitchAndFamily,lpszFace);
}
BOOL IsPatch=FALSE;
void* func_space_check;
void* func_ret_draw;
__declspec(naked)void check_space()
{
	__asm
	{
		cmp     edi, 0x20
		je l_is_space
		cmp     edi, 0xA1A1
		je l_is_space
		jmp func_ret_draw
		l_is_space:
		jmp func_space_check
	}
}
FARPROC WINAPI MyGetProcAddress(HMODULE hModule,LPCSTR lpFuncName)
{
	DWORD oldProtect;
	FARPROC fc = pfnGetProcAddress(hModule,lpFuncName);
	if((DWORD)lpFuncName > 0xFFFF && IsPatch == FALSE)
	{
		if(strcmp(lpFuncName,"PalFontSetType") == 0)
		{
			SetNopCode(	(PBYTE)fc + 0x1E,7);
			
			SetNopCode(	(PBYTE)hModule + 0x8B70,5);
			
			func_space_check = (void*)((PBYTE)hModule + 0x8B70);
			
			func_ret_draw = (void*)((PBYTE)hModule + 0x8B8A);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach((void**)&func_space_check,check_space);
			DetourTransactionCommit();

			const char* szGameOver = "确定要结束吗?";
			VirtualProtect((PBYTE)hModule + 0xFB914,100,PAGE_EXECUTE_READWRITE,&oldProtect);
			memcpy((PBYTE)hModule + 0xFB914,szGameOver,(strlen(szGameOver)+1));
	
			IsPatch = TRUE;
		}
	}
	return fc;
}

void PatchPAL()
{
	pfnCreateFont = CreateFontA;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfnCreateFont,MyCreateFontA);
	DetourTransactionCommit();
	
	pfnGetProcAddress = GetProcAddress;
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pfnGetProcAddress,MyGetProcAddress);
	DetourTransactionCommit();
}
//PalFontSetType
//DWORD GetPrivateProfileString(lpszSection, lpszKey, lpszDefault, lpszReturnBuffer, cchReturnBuffer, lpszFile)
BOOL WINAPI DllMain(HMODULE hModule,DWORD dwReason,LPVOID lpNone)
{
	char szLoadName[MAX_PATH];
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
			do_text();

			func_ret = (byte *(__cdecl *)(int,int,int,int))0x00430F90;
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach((void**)&func_ret,get_chars);
			DetourTransactionCommit();
			char szConfigName[MAX_PATH];
			GetCurrentDirectory(sizeof(szConfigName),szConfigName);
			strcat(szConfigName,"\\ATField.ini");
			GetPrivateProfileString("ATField","字体","黑体",szFontName,sizeof(szFontName),szConfigName);
			PatchPAL();

			break;
		}
	}

	return TRUE;
}