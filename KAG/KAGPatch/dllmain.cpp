// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "detours.h"

typedef HFONT (WINAPI* fnCreateFontIndirectA)(LOGFONTA *lplf);
fnCreateFontIndirectA pCreateFontIndirectA;

HFONT WINAPI newCreateFontIndirectA(LOGFONTA *lplf)
{
	LOGFONTA lf;
	memcpy(&lf,lplf,sizeof(LOGFONTA));
	if(lf.lfCharSet == SHIFTJIS_CHARSET)
	{
		lf.lfCharSet = DEFAULT_CHARSET;
	}

	lf.lfWeight = FW_BOLD;
	strcpy(lf.lfFaceName,"黑体");
	
	return pCreateFontIndirectA(&lf);
}
void* g_pImmGetContext;
void* g_pImmReleaseContext;
void* g_pImmSetCompositionWindow;

__declspec(naked)void ImmGetContext()
{
	__asm jmp g_pImmGetContext;
}
__declspec(naked)void ImmReleaseContext()
{
	__asm jmp g_pImmReleaseContext;
}
__declspec(naked)void ImmSetCompositionWindow()
{
	__asm jmp g_pImmSetCompositionWindow;
}
FARPROC GetProcAddressEx(HANDLE hModule, char *lpProcName);

//===================================================================================
FARPROC GetProcAddress_FromExportOrdinal(PIMAGE_NT_HEADERS pNTHdr, DWORD Ordinal)
{
	PIMAGE_EXPORT_DIRECTORY pExpDir;
	DWORD *pFunctionArray;
	DWORD ImageBase;
	DWORD ExportDirSize;
	char szForwardedModule[MAX_PATH];
	char *pszForwardedFunctionName;
	HANDLE hForwardedMod;
	FARPROC Result;

	ExportDirSize = pNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	if (!ExportDirSize) return NULL;

	ImageBase = pNTHdr->OptionalHeader.ImageBase;

	pExpDir = (PIMAGE_EXPORT_DIRECTORY)(pNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + ImageBase);

	pFunctionArray = (DWORD*)(ImageBase + (DWORD)pExpDir->AddressOfFunctions);

	if (pExpDir->NumberOfFunctions <= (Ordinal - pExpDir->Base)) return NULL;

	Result = (FARPROC)*(DWORD**)((DWORD)pFunctionArray + (Ordinal - pExpDir->Base) * sizeof(DWORD));

	if (Result) Result = (FARPROC)((DWORD)Result + ImageBase);

	if (((DWORD)Result >= (DWORD)pExpDir) && ((DWORD)Result < ((DWORD)pExpDir + ExportDirSize))) 
	{
		lstrcpyA(szForwardedModule, (LPCSTR)Result);
		pszForwardedFunctionName = (char*)szForwardedModule;
		while (*(char*)pszForwardedFunctionName != '.') 
		{
			pszForwardedFunctionName ++;
		}
		*(char*)pszForwardedFunctionName = 0;

		pszForwardedFunctionName ++;

		hForwardedMod = GetModuleHandleA(szForwardedModule);

		return GetProcAddressEx(hForwardedMod, pszForwardedFunctionName);
	}

	return Result;
}

//===================================================================================
FARPROC GetProcAddress_FromExportName(PIMAGE_NT_HEADERS pNTHdr, char *lpszProcName)
{
	PIMAGE_EXPORT_DIRECTORY pExpDir;
	DWORD ImageBase;
	DWORD *pNamesArray;
	DWORD *pNameOrdinalsArray;
	char *pszCurName;  
	DWORD NameOrdinal, index;

	if (!pNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size) return NULL;

	ImageBase = pNTHdr->OptionalHeader.ImageBase;

	pExpDir = (PIMAGE_EXPORT_DIRECTORY)(pNTHdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + ImageBase);

	if (!pExpDir->NumberOfNames) return NULL;

	pNamesArray = (DWORD*)(ImageBase + (DWORD)pExpDir->AddressOfNames);

	pNameOrdinalsArray = (DWORD*)(ImageBase + (DWORD)pExpDir->AddressOfNameOrdinals);

	for (index = 0;index < pExpDir->NumberOfNames;index ++)
	{
		pszCurName = (char*)(ImageBase + *(DWORD*)((DWORD)pNamesArray + sizeof(DWORD) * index));

		if (!strcmp(pszCurName, lpszProcName)) break;
	}

	if (index == pExpDir->NumberOfNames) return NULL;

	NameOrdinal = *(WORD*)((DWORD)pNameOrdinalsArray + sizeof(WORD) * index);
	return GetProcAddress_FromExportOrdinal(pNTHdr, NameOrdinal + pExpDir->Base);
}

//===================================================================================
FARPROC GetProcAddressEx(HANDLE hModule, char *lpProcName)
{
	PIMAGE_DOS_HEADER DosHeaderPt;
	PIMAGE_NT_HEADERS NtHeadersPt;

	if (!hModule) return NULL;
	DosHeaderPt = (PIMAGE_DOS_HEADER)hModule;

	if (DosHeaderPt->e_magic != IMAGE_DOS_SIGNATURE) return NULL;

	NtHeadersPt = (PIMAGE_NT_HEADERS)(int(DosHeaderPt) + DosHeaderPt->e_lfanew);

	if (NtHeadersPt->Signature != IMAGE_NT_SIGNATURE) return NULL;

	if (!HIWORD((DWORD)lpProcName)) 
	{
		return GetProcAddress_FromExportOrdinal(NtHeadersPt, (DWORD)lpProcName);
	}
	else if (!IsBadStringPtrA(lpProcName, 0))
	{
		return GetProcAddress_FromExportName(NtHeadersPt, lpProcName);
	}
	return NULL;
}

#pragma pack(1)
typedef struct StartConfig
{
	BYTE fill;
	DWORD unknow;	//0x0
	DWORD pagecode;	//0x3A4
	DWORD pagecode2;	//0x411
	DWORD unk2;		//0xFFFFFDE4
	DWORD unk3;		//0x64
}StartConfig;
#pragma pack()
DWORD threadId;
HANDLE hWtEvent;
DWORD WINAPI LoadDllThread(VOID* param)
{
	BYTE b[] = {0xE9,0x90};
	LoadLibraryA("ntleah.dll");

	Sleep(100);


	WaitForSingleObject(hWtEvent,INFINITE);
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS,FALSE,threadId);
	SuspendThread(hThread);


	
	DWORD op;
	VirtualProtect((LPVOID)0x00401000,2,PAGE_EXECUTE_READWRITE,&op);
	WriteProcessMemory(GetCurrentProcess(),(LPVOID)0x00401000,&b,sizeof(b),NULL);
	FlushInstructionCache(GetCurrentProcess(),(LPVOID)0x401000,2);

	ResumeThread(hThread);
	return 0;
}

void ConvertJIS()
{
	HANDLE hMap;
	
	PVOID pBuf;
	BYTE* pBuffer;
	wchar_t ntlea_path[MAX_PATH];

	threadId = GetCurrentThreadId();

	hWtEvent = CreateEventA(NULL,FALSE,FALSE,"RcpEvent000");

	hMap = CreateFileMappingA(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,4,"RcpFileMap000");

	pBuf = MapViewOfFile(hMap,FILE_MAP_ALL_ACCESS,0,0,0x4);

	GetCurrentDirectory(sizeof(ntlea_path),ntlea_path);

	wcscat(ntlea_path,L"\\ntleah.dll");


	pBuffer = (BYTE *)VirtualAlloc(NULL,0x1000,MEM_COMMIT,PAGE_READWRITE);

	wcscpy((WCHAR*)pBuffer,ntlea_path);

	DWORD offsetA = (wcslen(ntlea_path) + 1 ) * sizeof(wchar_t);
	StartConfig cfg;

	cfg.pagecode = 0x3A4;
	cfg.pagecode2 = 0x411;
	cfg.unknow = 0x0;
	cfg.unk2 = 0xFFFFFDE4;
	cfg.unk3 = 0x64;
	cfg.fill = 0;

	memcpy(&pBuffer[offsetA],&cfg,sizeof(cfg));

	*(DWORD*)pBuf = (DWORD)pBuffer;

	BYTE b[] = {0xEB,0xFE};
	DWORD op;
	VirtualProtect((LPVOID)0x00401000,2,PAGE_EXECUTE_READWRITE,&op);
	WriteProcessMemory(GetCurrentProcess(),(LPVOID)0x00401000,&b,sizeof(b),NULL);
	FlushInstructionCache(GetCurrentProcess(),(LPVOID)0x401000,2);
	CreateThread(NULL,NULL,LoadDllThread,NULL,NULL,NULL);
}
VOID ReplaceCall(void* begin,void* end,int length)
{
	DWORD op;

	VirtualProtect(begin,length,PAGE_EXECUTE_READWRITE,&op);

	memset(begin,0x90,length);

	*(BYTE*)begin = 0xE8;
	*(DWORD*)((DWORD)begin + 1) = (DWORD)end - (DWORD)begin - 5;
}

VOID ReplaceStringFont()
{
	DWORD ReplaceAddress [] = {
	0x0049CB2A,
	0x004AE1B2,
	0x004AE1F8,
	0x004AE20F,
	0x004B51E2,
	0x004CFD0E,
	0x004D6D6C,
	0x004DC5E3,
	0x00514789,
	0x005147C6,
	0x00656806
	};


	for(int i=0;i<ARRAYSIZE(ReplaceAddress);i++)
	{
		ReplaceCall((void*)ReplaceAddress[i],(void*)&newCreateFontIndirectA,5);
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
		*(FARPROC*)&g_pImmGetContext = GetProcAddress(LoadLibraryA("imm32.dll"),"ImmGetContext");
		*(FARPROC*)&g_pImmReleaseContext = GetProcAddress(LoadLibraryA("imm32.dll"),"ImmReleaseContext");
		*(FARPROC*)&g_pImmSetCompositionWindow = GetProcAddress(LoadLibraryA("imm32.dll"),"ImmSetCompositionWindow");
		*(FARPROC*)&pCreateFontIndirectA = GetProcAddressEx(GetModuleHandleA("GDI32.dll"),"CreateFontIndirectA");
		//DetourTransactionBegin();
		//DetourAttach((void**)&pCreateFontIndirectA,newCreateFontIndirectA);
		//DetourTransactionCommit();

		ReplaceStringFont();

		ConvertJIS();
		//CreateThread(NULL,NULL,LoadDllThread,NULL,NULL,NULL);
		
		
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

