#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "detours.h"
#include <vector>
#include <string>
using namespace std;


#define DEBUG_PATCH
typedef HFONT (WINAPI* fnCreateFontIndirectW)(LOGFONTW *lplf);
fnCreateFontIndirectW pCreateFontIndirectW;

HFONT WINAPI newCreateFontIndirectW(LOGFONTW *lplf)
{
	LOGFONTW lf;
	memcpy(&lf,lplf,sizeof(LOGFONTW));
	if(lf.lfCharSet == SHIFTJIS_CHARSET)
	{
		lf.lfCharSet = DEFAULT_CHARSET;

		wcscpy(lf.lfFaceName,L"黑体");
	}
	
	return pCreateFontIndirectW(&lf);
}

HDC GetDrawDC()
{
	HDC ReturnDC;
	__asm
	{
		mov eax,0x7F30E8
		mov eax,[eax]
		mov eax,[eax]
		mov ReturnDC,eax
	}
	return ReturnDC;
}
DWORD GetTextWidth(LONG c)
{
	wchar_t wchar[10];
	SIZE s;
	TEXTMETRICW tmw;
	GetTextMetricsW(GetDrawDC(),&tmw);

	memcpy(wchar,&c,sizeof(LONG));

	GetTextExtentPoint32W(GetDrawDC(),wchar,wcslen(wchar),&s);

	return s.cx;
}
DWORD GetTextHeight(LONG c)
{
	wchar_t wchar[10];
	SIZE s;
	TEXTMETRICW tmw;
	GetTextMetricsW(GetDrawDC(),&tmw);

	memcpy(wchar,&c,sizeof(LONG));

	GetTextExtentPoint32W(GetDrawDC(),wchar,wcslen(wchar),&s);

	return s.cy;
}
int g_width;
__declspec(naked)void fix_width()
{
	__asm
	{
		pushad
		push esi
		call GetTextWidth
		add esp,4
		mov g_width,eax
		popad
		mov eax,g_width
		push 0x004D68AC
		retn
	}
}
void write_jmp(void* src,void* dst)
{
	DWORD oldProtect;
	VirtualProtect(src,0,PAGE_EXECUTE_READWRITE,&oldProtect);
	__asm
	{
		mov eax,src
		mov ecx,dst
		sub ecx,eax
		sub ecx,5
		mov byte ptr[eax],0xE9
		inc eax
		mov dword ptr[eax],ecx
	}
}
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

enum g00_type_info
{
	type_24bit,
	type_8bit,
	type_dir
};
#pragma pack(1)
typedef struct g00_header_s
{
	uint8_t type;
	uint16_t width;
	uint16_t height;
}g00_header_t;
#pragma pack()
typedef struct g00_24bit_header_s
{
	uint32_t compress_length;
	uint32_t decompress_length;
}g00_24bit_header_t;

typedef struct g02_info_s{
	uint32_t orig_x;			// 原点在屏幕中的位置
	uint32_t orig_y;
	uint32_t end_x;			// 终点在屏幕中的位置
	uint32_t end_y;
	uint32_t unknown[2];		// 0
} g02_info_t;


BOOLEAN g00Reader=FALSE;
void (*pg00_enter_reader)();
void (*pg00_leave_reader)();
void (*pg00_open_file)();
void (*pg00_record_buffer)();
void (*pg00_analysis_file)();
void (*pg00_set_indexes)();


unsigned char* g00_buf;
size_t buf_length;

uint8_t g00_type;

wchar_t szFileName[MAX_PATH];


void  g00_enter_reader()
{
	g00Reader = TRUE;
	pg00_enter_reader();
}

void g00_leave_reader()
{
	g00Reader = FALSE;
	pg00_leave_reader();
}
void record_file_name()
{
	wchar_t* pwcs_reader_name;
	__asm
	{
		mov eax,[ecx]
		mov pwcs_reader_name,eax
	}
	wcscpy(szFileName,pwcs_reader_name);

	wcslwr(szFileName);
	if(wcsstr(szFileName,L"_menu_btn_sub.g00")!=NULL)
	{
		MessageBoxA(NULL,"Debug","",MB_OK);
	}

}
//0058F2C0
__declspec(naked) void g00_open_file()
{
	__asm pushad;
	if(g00Reader) record_file_name();
	__asm popad;
	__asm jmp pg00_open_file
}

DWORD r_save;
__declspec(naked) void g00_record_buffer()
{
	__asm
	{
		mov r_save,eax
		
	}
	if(g00Reader)
	{
		__asm
		{
			mov eax,[esp+4]
			mov buf_length,eax
			mov g00_buf,ecx
			
		}
	}
	__asm mov eax,r_save
	__asm jmp pg00_record_buffer
}

typedef struct g02_patchs_index_s
{
	g02_info_t info;
	uint8_t * buf;
}g02_patchs_index_t;
DWORD anal_eax;
void* extract_buf;
void* extract_length;
vector <g02_patchs_index_t> g02_list;

uint8_t* get_bmp_dib(uint8_t * b)
{
	uint8_t * bmp_data;
	BITMAPFILEHEADER* Header;
	BITMAPINFOHEADER* HeaderInfo;
	
	
	Header = (BITMAPFILEHEADER*)b;
	HeaderInfo = (BITMAPINFOHEADER*)(b+sizeof(BITMAPFILEHEADER));

	bmp_data = &b[Header->bfOffBits];

	return bmp_data;
}
uint32_t get_bmp_length(uint8_t * b)
{
	uint8_t * bmp_data;
	BITMAPFILEHEADER* Header;
	BITMAPINFOHEADER* HeaderInfo;
	
	
	Header = (BITMAPFILEHEADER*)b;
	HeaderInfo = (BITMAPINFOHEADER*)(b+sizeof(BITMAPFILEHEADER));

	int nAlignWidth = (HeaderInfo->biWidth*32+31)/32;;

	return 4 * nAlignWidth * HeaderInfo->biHeight;;
}
wchar_t* get_str_name(wchar_t* str)
{
	static wchar_t str_name[256];
	uint32_t len = wcslen(str);
	wcscpy(str_name,wcsrchr(str,L'\\')+1);

	*wcsrchr(str_name,L'.') = 0;
	return str_name;

}
uint8_t* get_data_buf(wchar_t* str,uint32_t index)
{
	wchar_t pic_dir[MAX_PATH];
	wchar_t pic_name[MAX_PATH/2];
	uint8_t* buf=0;
	uint32_t length;
	GetCurrentDirectoryW(sizeof(pic_dir),pic_dir);
	wcscat(pic_dir,L"\\");
	wcscat(pic_dir,get_str_name(str));
	wcscat(pic_dir,L"\\");
	swprintf(pic_name,L"%04d.bmp",index);
	wcscat(pic_dir,pic_name);

	FILE* f = _wfopen(pic_dir,L"rb");
	if(f)
	{
		fseek(f,0,SEEK_END);
		length = ftell(f);
		fseek(f,0,SEEK_SET);
		buf = new uint8_t[length];
		fread(buf,length,1,f);
		fclose(f);
	}
	return buf;
}

uint32_t get_g02_index_entries(g00_header_t* pheader)
{
	uint8_t * pbuf = (uint8_t*)pheader;

	pbuf += sizeof(g00_header_t); 
	return *(uint32_t*)pbuf;
}
void g02_create_vector(g00_header_t *pheader,vector <g02_patchs_index_t>& vec)
{
	uint8_t * pbuf = (uint8_t*)pheader + sizeof(g00_header_t) + sizeof(uint32_t);

	uint32_t index_entries = get_g02_index_entries(pheader);

	
	for(uint32_t start = 0;start < index_entries;start++)
	{
		g02_info_t * g02_info = &vec[start].info;
		memcpy(g02_info,pbuf,sizeof(g02_info_t));
		pbuf += sizeof(g02_info_t);
		vec[start].buf = get_data_buf(szFileName,start);
	}
}
void pre_g02_release()
{
	for(uint32_t x=0;x<g02_list.size();x++)
	{
		if(g02_list[x].buf) delete g02_list[x].buf;
	}
	g02_list.clear();
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


BOOL ScreenSaveFile(const char *file, int width, int height, void *data)
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

void pre_g02_file_info(g00_header_t *pheader)
{
	
	uint32_t index_entries;

	index_entries = get_g02_index_entries(pheader);

	pre_g02_release();
	vector <g02_patchs_index_t> g02_tmp (index_entries);


	g02_create_vector(pheader,g02_tmp);

	g02_list = g02_tmp;

}

void my_write_bmp(void* data)
{
	ScreenSaveFile("c:\\1.bmp",0x480,0x2d0,data);
}

void g00_get_file_info()
{
	g00_header_t *pheader = (g00_header_t*)g00_buf;
	g00_type = pheader->type;
	if(pheader->type == type_dir)
	{
		pre_g02_file_info(pheader);	
	}
	if(pheader->type == type_24bit)
	{
		
	}
}
__declspec(naked)void g00_analysis_file()
{
	__asm
	{
		pushad
	}

	if(g00Reader)
	{
		g00_get_file_info();
	}

	__asm
	{
		popad
		jmp pg00_analysis_file
	}
}
uint32_t pic_index;
//0055F352
__declspec(naked)void g00_set_indexes()
{
	__asm pushad;

	if(g00Reader)
	{
		__asm
		{
			push eax;
			mov eax,[ebp-0x14];
			mov pic_index,eax;
			pop eax;

		}
	}
	__asm popad;
	__asm jmp pg00_set_indexes ;
}
uint8_t *rect_buf;
void (*pg00_copy_rect)() = (void (__cdecl *)(void))0x0055EFA9;

void copy_rect()
{
	if(g00Reader)
	{
		//uint8_t* b = g02_list[pic_index].buf;
		//if(b)
		//{
		if(wcsstr(szFileName,L"_caution1.g00")!=0)
			my_write_bmp(rect_buf);
			//memcpy(rect_buf,get_bmp_dib(b),get_bmp_length(b));
		//}
	}
}
__declspec(naked)void g00_copy_rect()
{
	__asm
	{
		push eax
		mov eax,[ebp-0x84]
		mov rect_buf,eax
		pop eax
		pushad
		call copy_rect
		popad
		jmp pg00_copy_rect
	}
}
//004D68A3
void BeginDetour()
{
	
	pCreateFontIndirectW = (fnCreateFontIndirectW)GetProcAddress(GetModuleHandle("gdi32.dll"),"CreateFontIndirectW");
	DetourTransactionBegin();
	DetourAttach((void**)&pCreateFontIndirectW,newCreateFontIndirectW);
	DetourTransactionCommit();


	pg00_enter_reader = (void (__cdecl *)(void))0x0055F150;
	pg00_leave_reader = (void (__cdecl *)(void))0x0055F3B4;
	pg00_open_file = (void (__cdecl *)(void))0x0058F2C0;

	pg00_record_buffer = (void (__cdecl *)(void))0x0058F4D0;
	pg00_analysis_file = (void (__cdecl *)(void))0x0055F221;
	pg00_set_indexes = (void (__cdecl *)(void))0x0055F352;
	


	DetourTransactionBegin();
	DetourAttach((void**)&pg00_enter_reader,g00_enter_reader);
	DetourTransactionCommit();
	DetourTransactionBegin();
	DetourAttach((void**)&pg00_leave_reader,g00_leave_reader);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&pg00_open_file,g00_open_file);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&pg00_record_buffer,g00_record_buffer);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&pg00_analysis_file,g00_analysis_file);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&pg00_set_indexes,g00_set_indexes);
	DetourTransactionCommit();

	DetourTransactionBegin();
	DetourAttach((void**)&pg00_copy_rect,g00_copy_rect);
	DetourTransactionCommit();

	//
}

void NumberPatch(){}
BOOL WINAPI DllMain(HMODULE hModule,DWORD dwReason,LPVOID lpReserved)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:

		BeginDetour();
		break;
	default:
		break;
	}
	return TRUE;
}