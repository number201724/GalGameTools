#include <Windows.h>
#include <locale.h>
#include <Shlwapi.h>
#include <ShlObj.h>
#include <stdio.h>
#include <stdlib.h>
#include "detours.h"
#include "filesystem.h"
#include "native.h"

#define HEAD_STRING "$TEXT_LIST__"


char** chineseTextList;
int chineseTextCount;

char gameFontName[64];

fnCreateFontA CreateFontAOrg;
fnGetProcAddress GetProcAddressOrg;
fnPalLoadSpr PalLoadSprOrg;


//==============================================================================================================================================================================
//文字修正
//==============================================================================================================================================================================


#define SYS_ReadUInt32(pt,uv) \
	uv = *(unsigned int*)pt; \
	pt += sizeof(unsigned int) 

char* SYS_ReadString(unsigned char** pt)
{
	char* ptx;
	ptx = (char*)*pt;
	*pt += strlen(ptx) +1;

	return ptx;
}

void SYS_DecodeTextFile(unsigned char* c)
{
	unsigned char* pt;
	unsigned int strcount;
	unsigned int strid;

	unsigned int curid;
	char* cursc;

	pt = c;

	pt += sizeof(HEAD_STRING) - 1;

	SYS_ReadUInt32(pt,strcount);

	chineseTextCount = strcount;
	chineseTextList = (char**)malloc(sizeof(char*) * strcount);

	for(strid = 0;strid<strcount;strid++)
	{
		SYS_ReadUInt32(pt,curid);
		cursc = SYS_ReadString(&pt);

		chineseTextList[strid] = cursc;
	}
}

VOID SYS_LoadText()
{
	void* f_buf;
	unsigned long f_length;

	fs_readfile("TEXT.DAT",&f_buf,&f_length);

	if(!f_buf){
		MessageBoxA(NULL,"无法找到汉化文本文件TEXT.DAT","",MB_OK);
		ExitProcess(0);
	}

	SYS_DecodeTextFile((unsigned char*)f_buf);
}

char* (*GetTextByIndexOrg)(int v1,int v2,int v3,int v4);

char* GetTextByIndex(int v1,int v2,int v3,int v4)
{
	char* ret;
	int index;

	ret = GetTextByIndexOrg(v1,v2,v3,v4);

	if((v3 & 0x10000000) || v4 == 0xFFFFFFF){
		return ret;
	}

	index = *(int*)ret;
	if(index < chineseTextCount){
		char* s_start = ret + sizeof(int);

		strcpy(s_start,chineseTextList[index]);
	}
	return ret;
}

void* pOrgCharCheck;
void* pDrawChar;
__declspec(naked)void _asm_CharCheck()
{
	__asm
	{
		cmp     edi, 0x20;		//半角的空格
		je have;
		cmp     edi, 0xA1A1;	//全角的空格
		je have;
		jmp pDrawChar;			//绘制文本
have:
		jmp pOrgCharCheck;		//空出位置
	}
}


//==============================================================================================================================================================================
//图片修正
//==============================================================================================================================================================================

VOID GetBMPData(PBYTE BMPImage,LONG* pdwWidth,LONG* pdwHeight,int* pBitCount,BYTE** OutputBits,DWORD* BitsSize)
{
	BITMAPFILEHEADER* Header;
	BITMAPINFOHEADER* HeaderInfo;
	BYTE* SourceBits;
	BYTE* TargetBits;
	int TargetAlignWidth;
	int SourceAlignWidth;
	int BitCount;
	ULONG TargetBitsLength;
	BYTE* SourceLines;
	BYTE* TargetLines;
	LONG ProcessHeight;
	LONG ProcessWidth;

	
	Header = (BITMAPFILEHEADER*)BMPImage;
	HeaderInfo = (BITMAPINFOHEADER*)(BMPImage+sizeof(BITMAPFILEHEADER));

	SourceBits = &BMPImage[Header->bfOffBits];

	BitCount = HeaderInfo->biBitCount;

	TargetAlignWidth = (HeaderInfo->biWidth*32+31)/32;
	SourceAlignWidth = (HeaderInfo->biWidth*BitCount+31)/32;

	TargetBitsLength = 4 * TargetAlignWidth * HeaderInfo->biHeight;

	TargetBits = (BYTE*)malloc(TargetBitsLength);

	for(ProcessHeight=0;ProcessHeight<HeaderInfo->biHeight;ProcessHeight++)
	{
		SourceLines = &SourceBits[SourceAlignWidth * ProcessHeight * 4];
		TargetLines = &TargetBits[TargetAlignWidth * ProcessHeight * 4];

		for(ProcessWidth=0;ProcessWidth<HeaderInfo->biWidth;ProcessWidth++)
		{
			TargetLines[0] = SourceLines[0];
			TargetLines[1] = SourceLines[1];
			TargetLines[2] = SourceLines[2];
			if(BitCount == 32)
			{
				TargetLines[3] = SourceLines[3];
			}
			else
			{
				TargetLines[3] = 0xFF;
			}
			
			SourceLines += (BitCount / 8);
			TargetLines += 4;
		}
	}

	*pdwWidth = HeaderInfo->biWidth;
	*pdwHeight = HeaderInfo->biHeight;
	*pBitCount = BitCount;
	*OutputBits = TargetBits;
	*BitsSize = TargetBitsLength;
}

void BMP_TO_DIB(PBYTE data,int width,int height,int BitCount)
{
	BYTE* TempBuffer;
	int i;
	int widthlen;

	int nAlignWidth = (width*32+31)/32;
	size_t BufferSize = 4 * nAlignWidth * height;
	TempBuffer = (BYTE*)malloc(BufferSize);

	//反转图片,修正图片信息
	widthlen = width * (BitCount / 8); //对齐宽度大小
	for(i=0;i<height;i++){
		memcpy(&TempBuffer[(((height-i)-1)*widthlen)],&data[widthlen * i],widthlen);
	}

	memcpy(data,TempBuffer,BufferSize);

	free(TempBuffer);
}

BYTE* g_ImageBits;
ULONG g_ImageBitsLength;
LONG g_ImageWidth;
LONG g_ImageHeight;
int g_ImageBitCount;

DWORD PalLoadSprEx(DWORD unk,CHAR* FileName,PBYTE DataBuffers,DWORD DataSize)
{
	void* f_buf;
	unsigned long f_length;
	
	DWORD result;


	char ConvertName[128];
	int ConvertNameLen;

	strcpy(ConvertName,FileName);
	ConvertNameLen= strlen(ConvertName);
	ConvertName[ConvertNameLen - 3] = 'B';
	ConvertName[ConvertNameLen - 2] = 'M';
	ConvertName[ConvertNameLen - 1] = 'P';
	
	//从文件包读取对应图片
	if(fs_havefile(ConvertName))
	{
		fs_readfile(ConvertName,&f_buf,&f_length);

		if(f_buf && f_length){
			GetBMPData((PBYTE)f_buf,&g_ImageWidth,&g_ImageHeight,&g_ImageBitCount,&g_ImageBits,&g_ImageBitsLength);
			BMP_TO_DIB(g_ImageBits,g_ImageWidth,g_ImageHeight,32);
		}
	}

	result = PalLoadSprOrg(unk,FileName,DataBuffers,DataSize);

	//释放数据
	if(g_ImageBits)
	{
		free(g_ImageBits);
		g_ImageBits = NULL;
	}

	return result;
}
VOID Fix_Width(PBYTE SourceDIB,int Width,int Height,int BitCount,int NewWidth,BYTE** Output,DWORD* OutputLength)
{
	BYTE* TmpBuffer;
	int i,j;
	BYTE* SourceLines;
	BYTE* TargetLines;

	int nAlignWidth = (Width*32+31)/32;
	int nNewAlignWidth = (NewWidth*32+31)/32;
	*OutputLength = 4 * nNewAlignWidth * Height;
	*Output = (BYTE*)malloc(*OutputLength);
	TmpBuffer = *Output;


	memset(TmpBuffer,0,*OutputLength);

	for(i=0;i<Height;i++)
	{
		SourceLines = &SourceDIB[nAlignWidth * i * 4];
		TargetLines = &TmpBuffer[nNewAlignWidth * i * 4];
		for(j=0;j<Width;j++)
		{
			TargetLines[0] = SourceLines[0];
			TargetLines[1] = SourceLines[1];
			TargetLines[2] = SourceLines[2];
			TargetLines[3] = SourceLines[3];
			SourceLines += (BitCount / 8);
			TargetLines += 4;
		}
	}
}

void PALCopyImage(PBYTE draw_buf,int new_width)
{
	int nNewAlignWidth ;
	BYTE* Output = NULL;
	ULONG OutputLength;
	BYTE* SourceLines;
	BYTE* TargetLines;
	int i,j;

	if(g_ImageBits)
	{
		Fix_Width(g_ImageBits,g_ImageWidth,g_ImageHeight,32,new_width,&Output,&OutputLength);


		if(g_ImageBitCount != 32){
			memcpy(draw_buf,Output,OutputLength);
		} else {
			nNewAlignWidth = (new_width*32+31)/32;

			for(i=0;i<g_ImageHeight;i++)
			{
				SourceLines = &Output[nNewAlignWidth * i * 4];
				TargetLines = &draw_buf[nNewAlignWidth * i * 4];
				for(j=0;j<new_width;j++)
				{
					TargetLines[0] = SourceLines[0];
					TargetLines[1] = SourceLines[1];
					TargetLines[2] = SourceLines[2];
					TargetLines[3] = SourceLines[3];
					
					SourceLines += 4;
					TargetLines += 4;
				}
			}
		}
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
		call PALCopyImage
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
		call PALCopyImage
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
		call PALCopyImage
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
		call PALCopyImage
		add esp,0x8
		popad
		jmp type3_draw_1_ret
	}
}
void ReplaceJump(PBYTE src,PBYTE dst)
{
	DWORD oldProtect;
	
	VirtualProtect((LPVOID)src,10,PAGE_EXECUTE_READWRITE,&oldProtect);
	
	src[0] = 0xE9;
	*(DWORD*)&src[1] = (DWORD)(dst - src - 5);
}
PBYTE GetCALLTarget(PBYTE cal)
{
	return cal + *(DWORD*)&cal[1] + 5;
}


//==============================================================================================================================================================================
//补丁开始
//==============================================================================================================================================================================

void MEM_SetNopCode(BYTE* lpNop,SIZE_T dwSize)
{
	DWORD op;

	SIZE_T i;
	PBYTE adr;
	BYTE nix = 0x90;
	VirtualProtect(lpNop,dwSize,PAGE_EXECUTE_READWRITE,&op);

	for( i=0;i<dwSize;i++)
	{
		adr = &lpNop[i];
		WriteProcessMemory(GetCurrentProcess(),adr,&nix,sizeof(BYTE),NULL);
	}
}

const char* szGameOver = "确定要结束吗?";

VOID SYS_PatchRender(){
	PBYTE pal = (PBYTE)GetModuleHandleA("PAL.DLL");
	
	PBYTE cxc_addr = &pal[0x2108D];
	
	type3_draw_1_ret = cxc_addr + 5;
	
	type3_draw_1_call = GetCALLTarget(cxc_addr);
	
	ReplaceJump(cxc_addr,(PBYTE)&update_bmp_draw_1);
	
	cxc_addr = pal + 0x21068;
	
	
	type3_draw_2_ret = cxc_addr + 5;
	
	type3_draw_2_call = GetCALLTarget(cxc_addr);
	
	ReplaceJump(cxc_addr,(PBYTE)&update_bmp_draw_2);
	
	
	cxc_addr = pal + 0x210BC;
	type3_draw_3_ret = cxc_addr + 5;
	
	type3_draw_3_call = GetCALLTarget(cxc_addr);
	
	ReplaceJump(cxc_addr,(PBYTE)&update_bmp_draw_3);
	
	cxc_addr = pal + 0x210DE;
	type3_draw_4_ret = cxc_addr + 5;
	
	type3_draw_4_call = GetCALLTarget(cxc_addr);
	
	ReplaceJump(cxc_addr,(PBYTE)&update_bmp_draw_4);
}
VOID SYS_PatchPAL(){
	BYTE* pPalFontSetType;
	DWORD op;

	BYTE* hPAL = (BYTE*)GetModuleHandleA("PAL.DLL");


	pPalFontSetType = (BYTE*)GetProcAddressOrg((HMODULE)hPAL,"PalFontSetType");

	//补丁一些文本绘制的地方
	if( pPalFontSetType ){
		MEM_SetNopCode(&pPalFontSetType[0x1E],7);
	}
	MEM_SetNopCode(&((BYTE*)hPAL)[0x8A10],5);


	//补丁空格文本检查
	pOrgCharCheck = (void*)&hPAL[0x8A10];
	pDrawChar = (void*)&hPAL[0x8A2A];
	DetourTransactionBegin();
	DetourAttach((void**)&pOrgCharCheck,_asm_CharCheck);
	DetourTransactionCommit();


	//补丁游戏关闭文本
	
	VirtualProtect(&hPAL[0xF93F4],100,PAGE_EXECUTE_READWRITE,&op);
	memcpy(&hPAL[0xF93F4],szGameOver,(strlen(szGameOver)+1));

	*(DWORD*)&PalLoadSprOrg = (DWORD)&hPAL[0x22D00];

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&PalLoadSprOrg,PalLoadSprEx);
	DetourTransactionCommit();


	SYS_PatchRender();
}
FARPROC WINAPI GetProcAddressEx(HMODULE hModule,LPCSTR lpFuncName)
{
	static int done;

	if(!done && GetModuleHandleA("PAL.DLL") == hModule){

		SYS_PatchPAL();

		done = 1;
	}

	return GetProcAddressOrg(hModule,lpFuncName);
}

HFONT WINAPI CreateFontAEx(int nHeight,int nWidth,int nEscapement,int nOrientation,int fnWeight,DWORD fdwItalic,DWORD fdwUnderline, DWORD fdwStrikeOut,DWORD fdwCharSet,DWORD fdwOutputPrecision,DWORD fdwClipPrecision,DWORD fdwQuality,DWORD fdwPitchAndFamily,LPCSTR lpszFace)
{
	if(fdwCharSet == 0x80)
	{
		fdwCharSet = 0;
		fnWeight = FW_SEMIBOLD;
		return CreateFontAOrg(nHeight,nWidth,nEscapement,nOrientation,fnWeight,fdwItalic,fdwUnderline,fdwStrikeOut,fdwCharSet,fdwOutputPrecision,fdwClipPrecision,fdwQuality,fdwPitchAndFamily,gameFontName);
	}

	return CreateFontAOrg(nHeight,nWidth,nEscapement,nOrientation,fnWeight,fdwItalic,fdwUnderline,fdwStrikeOut,fdwCharSet,fdwOutputPrecision,fdwClipPrecision,fdwQuality,fdwPitchAndFamily,lpszFace);
}

VOID SYS_PatchSystem(){
	*(DWORD*)&GetTextByIndexOrg = 0x0042FC40;
	CreateFontAOrg = CreateFontA;
	GetProcAddressOrg = GetProcAddress;


	DetourTransactionBegin();
	DetourAttach((void**)&GetTextByIndexOrg,GetTextByIndex);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&CreateFontAOrg,CreateFontAEx);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&GetProcAddressOrg,GetProcAddressEx);
	DetourTransactionCommit();
}

VOID SYS_GetSystemFont(){
	char szConfigName[MAX_PATH];
	GetCurrentDirectoryA(sizeof(szConfigName),szConfigName);
	strcat(szConfigName,"\\ATField.ini");
	GetPrivateProfileStringA("ATField","字体","黑体",gameFontName,sizeof(gameFontName),szConfigName);
}

BOOL WINAPI DllMain(HMODULE hModule,DWORD dwReason,LPVOID lpReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH){
		if(fs_open("ATField.DAT")){
			MessageBoxA(NULL,"汉化文件ATField.DAT不存在!","",MB_OK);
			ExitProcess(0);
		}

		SYS_GetSystemFont();
		SYS_LoadText();

		SYS_PatchSystem();
	}
	return TRUE;
}



__declspec(dllexport) void Init_Dll(){};