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
typedef DWORD (*fnPalLoadSpr) (DWORD,CHAR* FileName,PBYTE DataBuffers,DWORD DataSize);
fnCreateFont pfnCreateFont;
fnGetProcAddress pfnGetProcAddress;

fnPalLoadSpr pPalLoadSpr;

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

	if(strcmp((char*)data,HEAD_STRING)==0)
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
						   int nHeight,
						   int nWidth,
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
BOOL FirstPatch = FALSE;

BOOL ScreenSaveFile(const char *file, int width, int height, void *data)
{
	int nAlignWidth = (width*32+31)/32;
	
	BYTE* tmp_data = new BYTE[4 * nAlignWidth * height];

	BITMAPFILEHEADER Header;
	BITMAPINFOHEADER HeaderInfo;
	Header.bfType = 0x4D42;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) ;
	Header.bfSize =(DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nAlignWidth* height * 4);
	HeaderInfo.biSize = sizeof(BITMAPINFOHEADER);
	HeaderInfo.biWidth = width;
	HeaderInfo.biHeight = height;
	HeaderInfo.biPlanes = 1;
	HeaderInfo.biBitCount = 32;
	HeaderInfo.biCompression = 0;
	HeaderInfo.biSizeImage = 4 * nAlignWidth * height;
	HeaderInfo.biXPelsPerMeter = 0;
	HeaderInfo.biYPelsPerMeter = 0;
	HeaderInfo.biClrUsed = 0;
	HeaderInfo.biClrImportant = 0; 
	FILE *pfile;
	
	if(!(pfile = fopen(file, "wb+")))
	{
		return FALSE;
	}
	
	//反转图片,修正图片信息
	int widthlen = width * 4; //对齐宽度大小
	//widthlen * nHeight
	for(DWORD m_height=0;m_height<height;m_height++) //用高度*(宽度*对齐)寻址
	{
		DWORD destbuf = (DWORD)tmp_data + (((height-m_height)-1)*widthlen); //将平行线对齐到目标线尾部
		DWORD srcbuf = (DWORD)data + widthlen * m_height; //增加偏移
		memcpy((void*)destbuf,(void*)srcbuf,widthlen); //复制内存
	}

	fwrite(&Header, 1, sizeof(BITMAPFILEHEADER), pfile);
	fwrite(&HeaderInfo, 1, sizeof(BITMAPINFOHEADER), pfile);
	fwrite(tmp_data, 1, HeaderInfo.biSizeImage, pfile);
	fclose(pfile);
	
	delete tmp_data;

	return TRUE;
}
int g_width;
int g_height;
int g_bitcount;
int g_oldbit;

PBYTE GetBMPData(PBYTE bmp)
{
	BITMAPFILEHEADER* Header;
	BITMAPINFOHEADER* HeaderInfo;
	
	
	Header = (BITMAPFILEHEADER*)bmp;
	HeaderInfo = (BITMAPINFOHEADER*)(bmp+sizeof(BITMAPFILEHEADER));

	PBYTE old_data = &bmp[Header->bfOffBits];
	
	int BitCount =  HeaderInfo->biBitCount;

	int nAlignWidth = (HeaderInfo->biWidth*32+31)/32;
	int nOldAlignWidth = (HeaderInfo->biWidth*BitCount+31)/32;
	PBYTE DIB_NewData;
	DWORD newFileSize = 4 * nAlignWidth * HeaderInfo->biHeight;
	
	g_width = HeaderInfo->biWidth;
	g_height = HeaderInfo->biHeight;
	g_bitcount = 32;
	
	g_oldbit = BitCount;

	DIB_NewData = new BYTE[newFileSize];


	for(LONG fix_height=0;fix_height<HeaderInfo->biHeight;fix_height++)
	{
		PBYTE scan_line = &old_data[nOldAlignWidth * fix_height * 4];
		PBYTE scan_line_new = &DIB_NewData[nAlignWidth * fix_height * 4];
		for(DWORD fix_width=0;fix_width<HeaderInfo->biWidth;fix_width++)
		{
			scan_line_new[0] = scan_line[0];
			scan_line_new[1] = scan_line[1];
			scan_line_new[2] = scan_line[2];
			if(BitCount == 32)
			{
				scan_line_new[3] = scan_line[3];
			}
			else
			{
				scan_line_new[3] = 0xFF;
			}
			
			scan_line += (BitCount / 8);
			scan_line_new += 4;
		}
	}

	//ScreenSaveFile("c:\\xxx.bmp",HeaderInfo->biWidth,HeaderInfo->biHeight,DIB_NewData);
	return DIB_NewData;
}


void BMP_TO_DIB(PBYTE data,int width,int height,int BitCount)
{
	int nAlignWidth = (width*32+31)/32;
	size_t data_size = 4 * nAlignWidth * height;
	BYTE* tmp_data = new BYTE[data_size];

	//反转图片,修正图片信息
	int widthlen = width * (BitCount / 8); //对齐宽度大小
	//widthlen * nHeight
	for(DWORD m_height=0;m_height<height;m_height++) //用高度*(宽度*对齐)寻址
	{
		DWORD destbuf = (DWORD)tmp_data + (((height-m_height)-1)*widthlen); //将平行线对齐到目标线尾部
		DWORD srcbuf = (DWORD)data + widthlen * m_height; //增加偏移
		memcpy((void*)destbuf,(void*)srcbuf,widthlen); //复制内存
	}

	memcpy(data,tmp_data,data_size);

	delete tmp_data;
}


PBYTE Fix_Width(PBYTE data,int width,int height,int BitCount,int new_width,DWORD& ref_size)
{
	int nAlignWidth = (width*32+31)/32;
	int nNewAlignWidth = (new_width*32+31)/32;
	size_t new_data_size = 4 * nNewAlignWidth * height;

	PBYTE new_data = new BYTE[new_data_size];
	
	ref_size = new_data_size;

	memset(new_data,0,new_data_size);

	for(DWORD fix_height=0;fix_height<height;fix_height++)
	{
		PBYTE scan_line = &data[nAlignWidth * fix_height * 4];
		PBYTE scan_line_new = &new_data[nNewAlignWidth * fix_height * 4];
		for(DWORD fix_width=0;fix_width<width;fix_width++)
		{
			scan_line_new[0] = scan_line[0];
			scan_line_new[1] = scan_line[1];
			scan_line_new[2] = scan_line[2];
			scan_line_new[3] = scan_line[3];
			scan_line += (BitCount / 8);
			scan_line_new += 4;
		}
	}
	return new_data;
}

__declspec(dllexport)void TestWriteBMP(void* buf)
{
	ScreenSaveFile("c:\\test.bmp",2048,720,buf);
}


byte* copy_buf=0;

BOOL ScreenSaveFile2(const char *file, int width, int height, void *data)
{
	int nAlignWidth = (width*32+31)/32;
	

	BITMAPFILEHEADER Header;
	BITMAPINFOHEADER HeaderInfo;
	Header.bfType = 0x4D42;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) ;
	Header.bfSize =(DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nAlignWidth* height * 4);
	HeaderInfo.biSize = sizeof(BITMAPINFOHEADER);
	HeaderInfo.biWidth = width;
	HeaderInfo.biHeight = height;
	HeaderInfo.biPlanes = 1;
	HeaderInfo.biBitCount = 32;
	HeaderInfo.biCompression = 0;
	HeaderInfo.biSizeImage = 4 * nAlignWidth * height;
	HeaderInfo.biXPelsPerMeter = 0;
	HeaderInfo.biYPelsPerMeter = 0;
	HeaderInfo.biClrUsed = 0;
	HeaderInfo.biClrImportant = 0; 
	FILE *pfile;
	
	if(!(pfile = fopen(file, "wb+")))
	{
		return FALSE;
	}
	

	fwrite(&Header, 1, sizeof(BITMAPFILEHEADER), pfile);
	fwrite(&HeaderInfo, 1, sizeof(BITMAPINFOHEADER), pfile);
	fwrite(data, 1, HeaderInfo.biSizeImage, pfile);
	fclose(pfile);

	return TRUE;
}

void copy_blx(PBYTE draw_buf,int new_width)
{
	PBYTE new_draw_buf;
	DWORD ref_size;
	if(copy_buf)
	{
		new_draw_buf = Fix_Width(copy_buf,g_width,g_height,g_bitcount,new_width,ref_size);
		
		if(g_oldbit != 32)
		{
			memcpy(draw_buf,new_draw_buf,ref_size);
		}
		else
		{
			int nNewAlignWidth = (new_width*32+31)/32;
	
			for(DWORD fix_height=0;fix_height<g_height;fix_height++)
			{
				PBYTE scan_line = & new_draw_buf[nNewAlignWidth * fix_height * 4];
				PBYTE scan_line_new = &draw_buf[nNewAlignWidth * fix_height * 4];
				for(DWORD fix_width=0;fix_width<new_width;fix_width++)
				{
					scan_line_new[0] = scan_line[0];
					scan_line_new[1] = scan_line[1];
					scan_line_new[2] = scan_line[2];
					scan_line_new[3] = scan_line[3];
					
					scan_line += 4;
					scan_line_new += 4;
				}
			}
		}
		delete new_draw_buf;
	}
}
int screen_width;

PBYTE type3_draw_1_ret;
PBYTE type3_draw_1_call;

PBYTE type3_draw_2_ret;
PBYTE type3_draw_2_call;

PBYTE type3_draw_3_ret;
PBYTE type3_draw_3_call;


PBYTE type3_draw_4_ret;
PBYTE type3_draw_4_call;


__declspec(naked)void update_bmp_draw_4()
{
	__asm
	{
		push [esp+4]
		pop screen_width
		call type3_draw_4_call
		pushad
		push screen_width
		push dword ptr [ebp+8]
		call copy_blx
		add esp,0x8
		popad
		jmp type3_draw_4_ret
	}
}

__declspec(naked)void update_bmp_draw_3()
{
	__asm
	{
		push [esp+0xC]
		pop screen_width
		call type3_draw_3_call
		pushad
		push screen_width
		push dword ptr [ebp+8]
		call copy_blx
		add esp,0x8
		popad
		jmp type3_draw_3_ret
	}
}

__declspec(naked)void update_bmp_draw_2()
{
	__asm
	{
		push [esp+0xC]
		pop screen_width
		call type3_draw_2_call
		pushad
		push screen_width
		push dword ptr [ebp+8]
		call copy_blx
		add esp,0x8
		popad
		jmp type3_draw_2_ret
	}
}
__declspec(naked)void update_bmp_draw_1()
{
	__asm
	{
		push [esp+4]
		pop screen_width
		call type3_draw_1_call
		pushad
		push screen_width
		push dword ptr [ebp+8]
		call copy_blx
		add esp,0x8
		popad
		jmp type3_draw_1_ret
	}
}
void fk_jump(PBYTE src,PBYTE dst)
{
	DWORD oldProtect;
	
	VirtualProtect((LPVOID)src,10,PAGE_EXECUTE_READWRITE,&oldProtect);
	
	src[0] = 0xE9;
	*(DWORD*)&src[1] = (DWORD)(dst - src - 5);
}
PBYTE get_call_addr(PBYTE cal)
{
	return cal + *(DWORD*)&cal[1] + 5;
}
void inteli_patch()
{
	PBYTE pal = (PBYTE)GetModuleHandle("pal.dll");
	
	PBYTE cxc_addr = pal + 0x2108D;
	
	type3_draw_1_ret = cxc_addr + 5;
	
	type3_draw_1_call = get_call_addr(cxc_addr);
	
	fk_jump(cxc_addr,(PBYTE)&update_bmp_draw_1);
	
	cxc_addr = pal + 0x21068;
	
	
	type3_draw_2_ret = cxc_addr + 5;
	
	type3_draw_2_call = get_call_addr(cxc_addr);
	
	fk_jump(cxc_addr,(PBYTE)&update_bmp_draw_2);
	
	
	cxc_addr = pal + 0x210BC;
	type3_draw_3_ret = cxc_addr + 5;
	
	type3_draw_3_call = get_call_addr(cxc_addr);
	
	fk_jump(cxc_addr,(PBYTE)&update_bmp_draw_3);
	
	cxc_addr = pal + 0x210DE;
	type3_draw_4_ret = cxc_addr + 5;
	
	type3_draw_4_call = get_call_addr(cxc_addr);
	
	fk_jump(cxc_addr,(PBYTE)&update_bmp_draw_4);
	
	//
}
//015710DE


DWORD myPalLoadSpr(DWORD unk,CHAR* FileName,PBYTE DataBuffers,DWORD DataSize)
{
	char name[MAX_PATH];
	char DiskFileName[MAX_PATH];

	FILE* f;
	byte* buf = 0;
	//byte* dib_buf = 0;

	strcpy(name,FileName);
	int len = strlen(name);
	name[len - 3] = 'b';
	name[len - 2] = 'm';
	name[len - 1] = 'p';
	
	GetCurrentDirectory(sizeof(DiskFileName),DiskFileName);
	strcat(DiskFileName,"\\bmp\\");
	strcat(DiskFileName,name);

	f = fopen(DiskFileName,"rb");
	
	copy_buf = 0;
	if(f)
	{
		long size;
		fseek(f,0,SEEK_END);
		size = ftell(f);
		fseek(f,0,SEEK_SET);
		buf = (byte*)malloc(size);
		fread(buf,size,1,f);
		fclose(f);
		
		copy_buf = GetBMPData(buf);
		BMP_TO_DIB(copy_buf,g_width,g_height,g_bitcount);
		

		delete buf;
	}


	//ScreenSaveFile
	DWORD retval = pPalLoadSpr(unk,FileName,DataBuffers,DataSize);
	
	if(copy_buf)
	{
		delete copy_buf;
		copy_buf = 0;
	}
	

	//if(dib_buf)	free(dib_buf);

	return retval;
}

FARPROC WINAPI MyGetProcAddress(HMODULE hModule,LPCSTR lpFuncName)
{
	DWORD oldProtect;
	FARPROC ret = pfnGetProcAddress(hModule,lpFuncName);

	if(!FirstPatch && GetModuleHandle("pal.dll") == hModule)
	{
		FARPROC fc = pfnGetProcAddress(hModule,"PalFontSetType");
		SetNopCode(	(PBYTE)fc + 0x1E,7);

		SetNopCode(	(PBYTE)hModule + 0x8A10,5);

		func_space_check = (void*)((PBYTE)hModule + 0x8A10);
		func_ret_draw = (void*)((PBYTE)hModule + 0x8A2A);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((void**)&func_space_check,check_space);
		DetourTransactionCommit();

		const char* szGameOver = "确定要结束吗?";
		VirtualProtect((PBYTE)hModule + 0xF93F4,100,PAGE_EXECUTE_READWRITE,&oldProtect);
		memcpy((PBYTE)hModule + 0xF93F4,szGameOver,(strlen(szGameOver)+1));

		pPalLoadSpr = (fnPalLoadSpr)((DWORD)hModule + 0x22D00);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((void**)&pPalLoadSpr,myPalLoadSpr);
		DetourTransactionCommit();
		
		inteli_patch();
		

		FirstPatch = TRUE;
	}
	return ret;
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

			func_ret = (byte *(__cdecl *)(int,int,int,int))0x0042FC40;
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