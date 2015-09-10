// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include <stdint.h>
#include "detours.h"
#include <map>
#include "crc.h"
#include "Package.h"
#include "compress.h"
using namespace std;

struct HG3HDR {
  unsigned char signature[4]; // "HG-3"
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
  unsigned long entry_count;
};

struct HG3TAG {
  char          signature[8];
  unsigned long offset_next;
  unsigned long length;
};

struct HG3STDINFO {
  unsigned long width;
  unsigned long height;
  unsigned long depth;
  unsigned long offset_x;
  unsigned long offset_y;
  unsigned long total_width;
  unsigned long total_height;
  unsigned long unknown1;
  unsigned long unknown2;
  unsigned long unknown3;
};

class PreItem
{
public:
	map <int,int> n_OffList;

	char szPreName[260];
	unsigned char* img_buf;
	unsigned int img_length;
};


typedef struct img_info_s
{
	uint32_t width; //?
	uint32_t height;//?
	uint32_t bits; 
	uint32_t unk1;
	uint32_t unk2;
	uint32_t unk_width;
	uint32_t unk_height;
	uint32_t unk3;
	uint32_t unk4;
	uint32_t unk5;
}img_info_t;

map <int,PreItem*> g_PreDataList;

CRITICAL_SECTION FixImgCritical;
typedef HFONT (WINAPI *fnCreateFontIndirectA)(LOGFONTA* lplf);
fnCreateFontIndirectA pCreateFontIndirectA;
HFONT WINAPI newCreateFontIndirectA(LOGFONTA* lplf)
{
	if(lplf)
	{
		lplf->lfCharSet = ANSI_CHARSET;
		strcpy(lplf->lfFaceName,"黑体");
	}
	return pCreateFontIndirectA(lplf);
}
char szPreName[260];
int hashCode;
unsigned char* img_buf;
unsigned int img_length;
char* image_name;
void (*pPreLoad)()=(void (__cdecl *)(void))0x004278D7;
void (*pRecordData)()=(void (__cdecl *)(void))0x0042793C;
void (*pUpdateBuf)=(void (__cdecl *)(void))0x00427A7E;

void* pfixed_img_begin = (void*)0x00426BB0;
void* pfixed_img_end = (void*)0x00426C7D;

void* pclear_img_info = (void*)0x0045D480;
int fixed_img_flag=0;
__declspec(naked)void fixed_img_begin()
{
	__asm
	{
		mov fixed_img_flag,1
		jmp pfixed_img_begin
	}
}
__declspec(naked)void clear_img_info()
{
	__asm
	{
		mov fixed_img_flag,0
		jmp pclear_img_info
	}
}

__declspec(naked)void fixed_img_end()
{
	__asm
	{
		mov fixed_img_flag,0
		jmp pfixed_img_end
	}
}

void* pfkt_copy_img = (void*)0x0045D480;
int fkt_copy_img_flag = 0;
__declspec(naked)void fkt_copy_img()
{
	__asm
	{
		mov fkt_copy_img_flag,1
		jmp pfkt_copy_img
	}
}
void UpdateIndexHash()
{
	//if(fixed_img_flag != 1)
	//	return;
	strcpy(szPreName,image_name);
	hashCode = CriteriaHash(szPreName,strlen(szPreName));
}
__declspec(naked)void PreLoad()
{
	__asm pushad;
	__asm pushfd;
	__asm mov image_name,ecx
	UpdateIndexHash();
	__asm popfd;
	__asm popad;
	__asm jmp pPreLoad
}
__declspec(naked)void RecordData()
{
	__asm
	{
		pushad
		mov img_length,edi
		mov img_buf,eax
		popad
		jmp pRecordData
	}
}
void UpdateBuffer()
{
	//if(fixed_img_flag != 1)
	//	return;
	map <int,PreItem*>::iterator iter = g_PreDataList.find(hashCode);

	if(iter == g_PreDataList.end())
	{
		PreItem* Item = new PreItem;
		Item->img_buf = img_buf;
		Item->img_length = img_length;
		strcpy(Item->szPreName,szPreName);
		//	
		//unsigned char* diff = img_buf + sizeof(HG3HDR);

		//HG3TAG* tag = (HG3TAG*)diff;

		//int index = 0;
		//

		//for(index = 0;true;index++)
		//{
		//	if(!strcmp(tag->signature,"stdinfo"))
		//	{

		//	}
		//}



		////n_OffList
		g_PreDataList[hashCode] = Item;
	}
	else
	{
		iter->second->img_buf = img_buf;
		iter->second->img_length = img_length;
	}
}

__declspec(naked)void UpdateBuf()
{
	__asm
	{
		pushad
		call UpdateBuffer
		popad
		jmp pUpdateBuf
	}
}
void* pGetImageInfo = (void*)0x00426C0B;
void* pGetImageIndex = (void*)0x004A9847;

PreItem* ImageDescriptor;
img_info_t* img_block_info;
int PackerIndex=0;
void* pRecordImageBlock = (void*)0x004B77C6;
void* pUpdateImageData = (void*)0x004B7E52;
void* pClearInfo = (void*)0x004B8014;
unsigned char* data_buffer;
void* penter_flag = (void*)0x00427860;
void* pleave_flag = (void*)0x00427AAC;
int img_flag2=0;

__declspec(naked)void enter_flag()
{
	__asm
	{
		mov img_flag2,1
		jmp penter_flag
	}
}
__declspec(naked)void leave_flag()
{
	__asm
	{
		mov img_flag2,0
		jmp pleave_flag
	}
}
void WINAPI FindImageDescriptor(char* name)
{
	if(fixed_img_flag != 1)
		return;
	map <int,PreItem*>::iterator iter;
	int MyHashCode = CriteriaHash(name,strlen(name));
	iter = g_PreDataList.find(MyHashCode);
	if(iter == g_PreDataList.end())
	{
		ImageDescriptor = NULL;
	}
	else
	{
		ImageDescriptor = iter->second;
	}
}

void CXXClearInfo()
{
	if(fixed_img_flag != 1)
		return;
	PackerIndex = 0;
	memset(szPreName,0,sizeof(szPreName));
	hashCode = 0;
}
__declspec(naked)void ClearInfo()
{
	__asm
	{
		pushad
		call CXXClearInfo
		popad
		jmp pClearInfo
	}
}

__declspec(naked)void GetImageIndex()
{
	__asm
	{
		mov PackerIndex,edx
		jmp pGetImageIndex
	}
}
__declspec(naked) void GetImageInfo()
{
	__asm
	{
		pushad;
		push edi
		call FindImageDescriptor;
		popad;
		jmp pGetImageInfo;
	}
}

__declspec(naked)void RecordImageBlock()
{
	__asm
	{
		mov img_block_info,eax
		//add img_block_info,4
		jmp pRecordImageBlock
	}
}


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


int o = 0;

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

	BMP_TO_DIB(DIB_NewData,HeaderInfo->biWidth,HeaderInfo->biHeight,32);

	//ScreenSaveFile("c:\\xxx.bmp",HeaderInfo->biWidth,HeaderInfo->biHeight,DIB_NewData);
	return DIB_NewData;
}
void* block_name_filter=(void*)0x00489550;
int name_filter=0;

__declspec(naked)void _block_name_filter()
{
	__asm
	{
		mov name_filter,1
		jmp block_name_filter
	}
}
const char* get_image_name()
{
	static char name[260];
	char tmp_name[260];
	GetCurrentDirectoryA(sizeof(name),name);
	strcat(name,"\\");
	strcat(name,ImageDescriptor->szPreName);
	strcat(name,"\\");

	sprintf(tmp_name,"img%04d.bmp",PackerIndex);
	strcat(name,tmp_name);

	return name;
}

size_t GetNameCRC()
{
	__asm finit;
	return crc((unsigned char*)&ImageDescriptor->szPreName,strlen(ImageDescriptor->szPreName));
}
char* ScriptName=(char*)0x6BF158;

void* update_script_length = (void*)0x0055E604;
int find_ok;
size_t new_script_len;

char szScriptName[MAX_PATH];
char* get_file_name(char* name)
{

	char* pstr = strrchr(name,'/');
	if(pstr)
	{
		pstr++;
		return pstr;
	}
	return NULL;
}
void find_script()
{
	find_ok = 0;
	sprintf(szScriptName,"%s/%s","scene",get_file_name(ScriptName));
	ULONG Length = FindFileLengthByName(szScriptName);
	if(Length != 0)
	{
		find_ok = 1;
		new_script_len = Length;
	}
}
__declspec(naked)void new_update_script_length()
{
	__asm
	{
		pushad
		call find_script
		popad
		cmp find_ok,1
		jnz RetVal
		mov ebp,new_script_len
RetVal:
		jmp update_script_length
	}
}
void* pCopyScriptData = (void*)0x0055E635;
void copy_script_data(unsigned char* buf)
{
	ULONG Length;
	unsigned char* buffer = FindFileDataByName(szScriptName,&Length);
	memcpy(buf,buffer,Length);
	free_buffer(buffer);
}
__declspec(naked)void CopyScriptData()
{
	__asm
	{
		pushad
		push ebx
		call copy_script_data
		add esp,4
		popad
		jmp pCopyScriptData
	}
}
//0055E635
void CopyData()
{
	if(img_flag2 != 1)
		return;
	if(fkt_copy_img_flag == 1)
	{
		fkt_copy_img_flag = 0;
		return;
	}
	if(name_filter == 1)
	{
		name_filter = 0;
		return;
	}
	if(ImageDescriptor)
	{
		ULONG DataLength;
		unsigned char* Data;
		Data = FindImageDataByName(GetNameCRC(),PackerIndex,&DataLength);
		if(Data)
		{
			PBYTE new_data = GetBMPData(Data);
			int nNewAlignWidth = (img_block_info->width*32+31)/32;

			memcpy(data_buffer,new_data,nNewAlignWidth*4*img_block_info->height);
			delete new_data;
			free_buffer(Data);
		}
		//if(o == 1)
		//{
		//	char szFileName[MAX_PATH];
		//	sprintf(szFileName,"c:\\img%04d.bmp",PackerIndex);
		//	ScreenSaveFile(szFileName,img_block_info->unk_width,img_block_info->unk_height,data_buffer);
		//}

		/*FILE* f= fopen(get_image_name(),"rb");
		if(f)
		{
			fseek(f,0,SEEK_END);
			size_t length = ftell(f);
			unsigned char* data = new unsigned char[length];
			fseek(f,0,SEEK_SET);
			fread(data,length,1,f);
			fclose(f);

			PBYTE new_data = GetBMPData(data);
			delete data;


			int nNewAlignWidth = (img_block_info->width*32+31)/32;

			memcpy(data_buffer,new_data,nNewAlignWidth*4*img_block_info->height);

			delete new_data;
		}*/
		//ImageDescriptor
	}
}


__declspec(naked)void UpdateImageData()
{
	__asm
	{
		mov eax,dword ptr [esp+0x1C]
		add eax,edx
		mov data_buffer,eax
		pushad
		call CopyData
		popad
		jmp pUpdateImageData
	}
}

void Hook(void** n,void*d)
{
	DetourTransactionBegin();
	DetourAttach(n,d);
	DetourTransactionCommit();
}
VOID OnAttach()
{
	pCreateFontIndirectA = (fnCreateFontIndirectA)GetProcAddress(GetModuleHandleA("gdi32.dll"),"CreateFontIndirectA");
	DetourTransactionBegin();
	DetourAttach((void**)&pCreateFontIndirectA,newCreateFontIndirectA);
	DetourTransactionCommit();
	Hook((void**)&pPreLoad,PreLoad);
	Hook((void**)&pRecordData,RecordData);
	Hook((void**)&pUpdateBuf,UpdateBuf);

	Hook((void**)&pGetImageInfo,GetImageInfo);
	Hook((void**)&pGetImageIndex,GetImageIndex);
	Hook((void**)&pRecordImageBlock,RecordImageBlock);
	Hook((void**)&pUpdateImageData,UpdateImageData);

	Hook((void**)&pClearInfo,ClearInfo);


	Hook((void**)&pfixed_img_begin,fixed_img_begin);
	Hook((void**)&pfixed_img_end,fixed_img_end);	
	Hook((void**)&pfixed_img_end,clear_img_info);	

	Hook((void**)&penter_flag,enter_flag);	
	Hook((void**)&penter_flag,enter_flag);	
	Hook((void**)&pfkt_copy_img,fkt_copy_img);	
	Hook((void**)&block_name_filter,_block_name_filter);

	Hook((void**)&pCopyScriptData,CopyScriptData);

	Hook((void**)&update_script_length,new_update_script_length);


	
	SetPackageMode(FALSE);
	if(!CreatePackage("pkg.npk"))
	{
		MessageBoxA(NULL,"缺少文件pkg.npk","错误",MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(),0);
	}
}
VOID OnDetach()
{
	DetourTransactionBegin();
	DetourDetach((void**)&pCreateFontIndirectA,newCreateFontIndirectA);
	DetourTransactionCommit();
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		InitializeCriticalSection(&FixImgCritical);
		DisableThreadLibraryCalls(hModule);
		OnAttach();
		break;
	case DLL_PROCESS_DETACH:
		OnDetach();
		break;
	}
	return TRUE;
}

