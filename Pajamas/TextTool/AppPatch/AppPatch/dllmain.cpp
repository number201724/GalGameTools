// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "detours.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <vector>
#include "FileStream.h"
using namespace std;
LPD3DXFONT g_pFont;
CFileStream fileStream;
LPDIRECT3DDEVICE9 pDevice;
typedef struct draw_str_s
{
	DWORD unk;  //0
	DWORD unk2; //4
	DWORD unk3; //8
	DWORD unk4; //c
	DWORD unk5; //10
	DWORD unk6; //14
	DWORD unk7; //18
	DWORD unk8; //1C
	DWORD unk9; //20
	DWORD solid_rgba[4]; //24 28 2C
	DWORD rgba[4]; //34 38 3c 40
	DWORD pstring; //44
	DWORD unk11; //48
	float width;//4C 文字间隔
	//可能是坐标
	DWORD height; //50Height partition
	DWORD x; //54
	DWORD y; //58
	DWORD unk15; //5C
	DWORD unk16; //60
	DWORD fontsize; //64
	DWORD unk18; //68
	DWORD unk19; //6C
	DWORD unk20; //70
	DWORD unk24; //74
	DWORD unk25; //78
	DWORD unk26; //7C - pointer
	DWORD unk27; //80 - pointer
	DWORD unk28; //84
	DWORD unk29; //88
	DWORD unk30; //8c
	DWORD unk31; //90 - pointer
}draw_str_t;
void SetNopCode(BYTE* pnop,size_t size)
{
	DWORD oldProtect;
	VirtualProtect((PVOID)pnop,size,PAGE_EXECUTE_READWRITE,&oldProtect);
	for(size_t i=0;i<size;i++)
	{
		pnop[i] = 0x90;
	}
}
bool CovtShiftJISToGB(const char* JISStr, char* GBBuf, int nSize) 
{ 
	static wchar_t wbuf[2048];
	int nLen; 

	nLen = strlen(JISStr)+1; 
	//if (wbuf==NULL) return false; 
	nLen = MultiByteToWideChar(932, 0, JISStr, 
		nLen, wbuf, nLen); 
	if (nLen > 0) 
		nLen = WideCharToMultiByte(936, 0, wbuf, 
		nLen, GBBuf, nSize, NULL, 0); 
	return nLen!=0; 
}
int CovtGBTOJIS(char* GBBuf , char* JISStr, int nSize) 
{ 
	static wchar_t wbuf[2048];
	int nLen; 

	nLen = strlen(GBBuf)+1; 
	if (wbuf==NULL) return false; 
	nLen = MultiByteToWideChar(936, 0, GBBuf, 
		nLen, wbuf, nLen); 
	if (nLen > 0) 
		nLen = WideCharToMultiByte(932, 0, wbuf, 
		nLen, JISStr, nSize, NULL, 0); 
	return nLen; 
}
typedef HRESULT (WINAPI * fnCreateDevice)(
	LPDIRECT3D9 pDx9,UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface
	);
fnCreateDevice pCreateDevice;
ID3DXSprite*                g_pTextSprite = NULL;   // Sprite for batching draw text calls
HRESULT WINAPI newCreateDevice(
	LPDIRECT3D9 pDx9,
	UINT Adapter,
	D3DDEVTYPE DeviceType,
	HWND hFocusWindow,
	DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS* pPresentationParameters,
	IDirect3DDevice9** ppReturnedDeviceInterface
	)
{//D3D_OK
	HRESULT status =  pCreateDevice(pDx9,Adapter,DeviceType,hFocusWindow,BehaviorFlags,pPresentationParameters,ppReturnedDeviceInterface);
	if(status==D3D_OK)
	{
		pDevice=*ppReturnedDeviceInterface;
		
		D3DXCreateFont( pDevice, -28, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
			L"宋体", &g_pFont );

		D3DXCreateSprite( pDevice, &g_pTextSprite );

	}
	return status;
}
void* pDrawHistoryHook1 = (void*)0x0046A030;
__declspec(naked)void _DrawHistoryHook1()
{
	__asm
	{
		mov     eax, dword ptr [esp+0x14]
		mov     dl, byte ptr [eax+1]
		test    dl, dl
		jmp pDrawHistoryHook1
	}
}

void* pDrawHistoryHook2 = (void*)0x0046A3E0;
__declspec(naked)void _DrawHistoryHook2()
{
	__asm
	{
		mov     eax, dword ptr [esp+0x14]
		mov     dl, byte ptr [eax+1]
		test    dl, dl
		jmp pDrawHistoryHook2
	}
}
void memcopy(void* dest,void*src,size_t size)
{
	DWORD oldProtect;
	VirtualProtect(dest,size,PAGE_EXECUTE_READWRITE,&oldProtect);
	memcpy(dest,src,size);
}
void InstallPatch()
{
	DWORD oldProtect;
	VirtualProtect((PVOID)0x00469530,0x2000,PAGE_EXECUTE_READWRITE,&oldProtect);
	BYTE nPatch1[] = {0xEB,0x24};
	BYTE nPatch2[] = {0xE9,0x0B,0x02,0x00,0x00,0x90,0x90,0x90,0x90,0x66,0x8B,0x1E,0x66,0x85,0xDB,0xEB,0x73,0x66,0x8B,0x1E,0x66,0x85,0xDB,0xEB,0xD6,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};
	BYTE nPatch3[] = {0xEB,0x87};
	BYTE nPatch4[] = {0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90};
	BYTE nPatch5[] = {0xB8,0x01,0x00,0x00,0x00,0x90};


	//
	memcopy((void*)0x0046957E,nPatch1,2);

	memcopy((void*)0x00469593,nPatch2,sizeof(nPatch2));
	memcopy((void*)0x00469613,nPatch3,sizeof(nPatch3));
	memcopy((void*)0x004697A3,nPatch4,sizeof(nPatch4));


	memcopy((void*)0x0046A403,nPatch5,sizeof(nPatch5));
	memcopy((void*)0x0046A042,nPatch5,5);

	//

	SetNopCode((PBYTE)pDrawHistoryHook1,8);


	//SetNopCode((PBYTE)0x0046A041,6);
	//SetNopCode((PBYTE)0x0046A054,6);
	//SetNopCode((PBYTE)0x0046A04A,3);

	SetNopCode((PBYTE)pDrawHistoryHook2,8);
	//SetNopCode((PBYTE)0x0046A3F1,1);
	//SetNopCode((PBYTE)0x0046A3F9,8);

	//SetNopCode((PBYTE)0x0046A403,6);

	////


	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pDrawHistoryHook1,_DrawHistoryHook1);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pDrawHistoryHook2,_DrawHistoryHook2);
	DetourTransactionCommit();

	//0046A041


}
void WINAPI DrawChar(wchar_t* string,BYTE*color,RECT inrect)
{
	RECT rct;
	SetRect(&rct, inrect.left,inrect.top, inrect.right, inrect.bottom);
	g_pFont->DrawTextW(NULL,string, -1, &rct, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(color[4], color[3], color[2], color[1]));
}
typedef DWORD (*fnFormatString) (draw_str_t* drawinfo,char* string);
fnFormatString pFormatString=(fnFormatString)0x00469530;
/*
不显示字符问题:
\这个字符是国际编码,第二个是\0 被跳过
判断修改:
movzx bx,xxxx
test bx,bx
jmp x
*/
DWORD newFormatString(draw_str_t* drawinfo,char* string)
{
	static char format_str[1024];
	static wchar_t newstring[1024];
	wchar_t gowchar;
	if(drawinfo && string)
	{
		strcpy(format_str,string);
		int len = strlen(format_str);
		for(int i=0;i<len;i++)
		{
			if(format_str[i] == '\\')
			{
				format_str[i] =' ';
				char c = format_str[i+1];
				switch(c)
				{
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				}
				format_str[i+1] = c;
			}
		}
		int nLen; 
		nLen = strlen(format_str)+1; 
		nLen = MultiByteToWideChar(CP_ACP, 0, format_str, 
			nLen, newstring, nLen); 
	}
	return pFormatString(drawinfo,(char*)&newstring);
}

void* pdraw_first=(void*)0x0046B92C;
void* pdraw_next=(void*)0x0046BCF4;
typedef struct draw_info_s
{
	DWORD unk1;
	DWORD unk2;
	DWORD unk3;
	DWORD color[4];
	DWORD color2[4];
	wchar_t str[2];
	RECT rect;
}draw_info_t;
//白色
void WINAPI draw_first(draw_info_t* data)
{
	RECT rect;
	BYTE color[4];
	color[0] = 255;
	color[1] = 255;
	color[2] = 255;
	color[3] = 255;
	memcpy(&rect,&data->rect,sizeof(data->rect));
	RECT rct;
	SetRect(&rct, rect.left,rect.top, rect.right, rect.bottom);

	g_pTextSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );

	//rct.left -= 1;
	//rct.top +=1;
	g_pFont->DrawTextW(g_pTextSprite,data->str, -1, &rct, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 255, 255));

	//rct.left += 4;
	//rct.top += 4;
	//g_pFont->DrawTextW(g_pTextSprite,data->str, -1, &rct, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(255, 255, 255, 255));

	g_pTextSprite->End();
}
void WINAPI draw_next(draw_info_t* data)
{
	RECT rect;
	BYTE color[4];
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	color[3] = 255;
	memcpy(&rect,&data->rect,sizeof(data->rect));

	RECT rct;
	SetRect(&rct, rect.left,rect.top, rect.right, rect.bottom);

	g_pTextSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );
	g_pFont->DrawTextW(g_pTextSprite,data->str, -1, &rct, DT_LEFT | DT_NOCLIP, D3DCOLOR_ARGB(255,0,0,0));
	g_pTextSprite->End();
}

__declspec(naked)void _draw_first()
{
	__asm
	{
		pushad
		push esi
		call draw_first
		popad
		jmp pdraw_first
	}
}
__declspec(naked)void _draw_next()
{
	__asm
	{
		pushad
			push esi
			call draw_next
			popad
			jmp pdraw_next
	}
}

__declspec(dllexport)void WINAPI Init()
{

}

IDirect3D9 * funcAPI;
typedef IDirect3D9 * (WINAPI* fnDirect3DCreate9)(UINT SDKVersion);
fnDirect3DCreate9 pDirect3DCreate9;
bool done=false;
IDirect3D9 * WINAPI newDirect3DCreate9(UINT SDKVersion)
{
	funcAPI =  pDirect3DCreate9(SDKVersion);
	if(funcAPI && !done)
	{
		done = true;
		pCreateDevice = (fnCreateDevice)*(DWORD*)(*(DWORD*)funcAPI+0x40);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((void**)&pCreateDevice,newCreateDevice);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((void**)&pFormatString,newFormatString);
		DetourTransactionCommit();


		SetNopCode((PBYTE)pdraw_first,14);
		SetNopCode((PBYTE)0x0046B952,2);
		SetNopCode((PBYTE)0x0046BA8F,12);
		SetNopCode((PBYTE)0x0046BCF4,14);


		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((void**)&pdraw_first,_draw_first);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach((void**)&pdraw_next,_draw_next);
		DetourTransactionCommit();

		InstallPatch();
	}
	return funcAPI;
}
void RouteAPI()
{
	//HFONT font = CreateFont(22, 0,0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH|FF_DONTCARE, L"Arial");
	//D3DXCreateFont(m_pD3DDevice, font, &m_pFont);
	//D3DXCreateFont
	HMODULE lib = LoadLibrary(L"d3d9.dll");

	pDirect3DCreate9 = (fnDirect3DCreate9)GetProcAddress(lib,"Direct3DCreate9");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&pDirect3DCreate9,newDirect3DCreate9);
	DetourTransactionCommit();
}
COLORREF rgb;
HWND ghWnd;
HBITMAP hBitmap;
int bitheight;
int bitwidth;
int CALLBACK TimerProc()
{
	static int wndAlp = 0;
	static int flag = 0;

	if(flag)
	{
		if(flag == 1)
		{
			Sleep(1000);
			flag = 2;
		}
		wndAlp-=3;
		if(wndAlp==0)
			DestroyWindow(ghWnd);
		SetLayeredWindowAttributes(ghWnd,-1,wndAlp,LWA_ALPHA);
	}
	else
	{
		wndAlp+=5;
		if(wndAlp==255)
			flag = 1;
		SetLayeredWindowAttributes(ghWnd,-1,wndAlp,LWA_ALPHA);
	}
	return 0;
}
void DrawBmp(HDC hDC, HBITMAP bitmap,int nWidth,int nHeight)
{
	BITMAP			bm;
	HDC hdcImage;
	HDC hdcMEM;
	hdcMEM = CreateCompatibleDC(hDC);
	hdcImage = CreateCompatibleDC(hDC);
	HBITMAP bmp = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
	GetObject(bitmap, sizeof(bm),(LPSTR)&bm);
	SelectObject(hdcMEM, bmp);
	SelectObject(hdcImage, bitmap);
	StretchBlt(hdcMEM, 0, 0, nWidth, nHeight, hdcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	StretchBlt(hDC, 0, 0, nWidth, nHeight, hdcMEM, 0, 0, nWidth, nHeight, SRCCOPY);

	DeleteObject(hdcImage);
	DeleteDC(hdcImage);
	DeleteDC(hdcMEM);
}
LRESULT CALLBACK WindowProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	static HDC compDC=0;
	static RECT rect;
	if(uMsg == WM_CREATE)
	{
		ghWnd = hwnd;
		SetLayeredWindowAttributes(hwnd,-1,0,LWA_ALPHA);
		SetTimer(hwnd,1003,1,(TIMERPROC)TimerProc);

		int scrWidth,scrHeight;

		scrWidth = GetSystemMetrics(SM_CXSCREEN);
		scrHeight = GetSystemMetrics(SM_CYSCREEN);
		GetWindowRect(hwnd,&rect);
		rect.left = (scrWidth-rect.right)/2;
		rect.top = (scrHeight-rect.bottom)/2;
		SetWindowPos(hwnd,HWND_TOP,rect.left,rect.top,rect.right,rect.bottom,SWP_SHOWWINDOW);

		DrawBmp(GetDC(hwnd),hBitmap,454,297);
	}
	if(uMsg == WM_PAINT)
	{
		RECT rect;
		GetWindowRect(hwnd,&rect);

	}
	if(uMsg==WM_CLOSE)
	{
		DestroyWindow(hwnd);
	}
	if(uMsg == WM_DESTROY)
	{
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}
HINSTANCE hInst;

void __stdcall InitHook()
{
	char szFilePath[MAX_PATH];
	DWORD routeAddr;
	DWORD oldProtect;
	GetCurrentDirectoryA(sizeof(szFilePath),szFilePath);
	strcat(szFilePath,"\\fm.dat");

	if(fileStream.Open(szFilePath))
	{
		fileStream.LoadFromStream();
		memory_tree_t * tree = fileStream.GetFile("/readme.bmp");
		if(tree)
		{
			char szTempPath[MAX_PATH];
			char szTempFile[MAX_PATH];
			GetTempPathA(MAX_PATH, szTempPath);
			GetTempFileNameA(szTempPath, "~fm", 0, szTempFile);
			FILE *pfile;

			if(!(pfile = fopen(szTempFile, "wb+")))
			{
				return;
			}
			fwrite(tree->dataOffset, 1, tree->dataSize, pfile);
			fclose(pfile);


			hBitmap = (HBITMAP)LoadImageA(NULL,szTempFile,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE);
		}
		WNDCLASSEXA wcex;
		memset(&wcex,0,sizeof(wcex));
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WindowProc;
		wcex.hInstance = hInst;
		wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
		wcex.lpszClassName	= "fmLogo";
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.cbWndExtra = DLGWINDOWEXTRA;
		rgb = 0xFFFFFFFF;
		RegisterClassExA(&wcex);
		HWND hWnd = CreateWindowExA(WS_EX_LAYERED|WS_EX_TOPMOST,"fmLogo","fmLogo",WS_POPUP | WS_SYSMENU | WS_SIZEBOX ,0, 0, 454, 297, NULL, NULL, hInst, NULL);
		ShowWindow(hWnd,SW_SHOW);
		UpdateWindow(hWnd);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		RouteAPI();

	}


}

BOOL APIENTRY DllMain( HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = (HINSTANCE)hModule;
		InitHook();
		
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

