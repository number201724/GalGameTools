// savepkg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
extern "C"
{
	#include "writestream.h"
};


typedef unsigned int uint;
typedef unsigned char byte;
typedef struct pkg_header_s
{
	uint file_size;
	uint file_count;
}pkg_header_t;
typedef struct pkg_file_s
{
	char file_name[116];
	uint file_size;
	uint file_offset;
	uint file_offset_high;
}pkg_file_t;
uint xor_key_buffer[] = 
{
	0xA13BB527,
	0x879FDA11,
	0x72FDADBC,
	0x1004A4D3,
	0x03A0FFB2,
	0x21CC32BA,
	0x973A2B1C,
	0xF7E8E667,
	/*0x00006425,
	0x30727563,
	0x7275632E,
	0x00000000,
	0x30727563,
	0x75632E31,
	0x00000072,
	0x48727563,
	0x2E646E61,
	0x00696E61,
	0x4F525245,
	0x00000052,
	0x63656843,
	0x7469426B,
	0x61727241,
	0x81C58279,
	0x977A9441,
	0x92F082F1,
	0x82A682B4,
	0x837283E9,
	0x92678362,
	0x8EF0826C,
	0x82C68F51,
	0x00BD82B5,
	0x4F525245,
	0x00000052,
	0x42746553,
	0x72417469,
	0x82796172,
	0x944181C5,
	0x82F1977A,
	0x82B492F0,
	0x83E982A6,
	0x83628372,
	0x826C9267,
	0x92778EF0,
	0x82B582E8,
	0x000000BD,
	0x00000000,
	0x00000000*/
};
int CovtShiftJISToGB(const char* JISStr, char* GBBuf, int nSize) 
{ 
	static wchar_t wbuf[2048];
	int nLen; 

	nLen = strlen(JISStr)+1; 
	if (wbuf==NULL) return 0; 
	nLen = MultiByteToWideChar(932, 0, JISStr, 
		nLen, wbuf, nLen); 
	if (nLen > 0) 
		nLen = WideCharToMultiByte(936, 0, wbuf, 
		nLen, GBBuf, nSize, NULL, 0); 
	return nLen; 
}
int CovtGbkToShiftJIS(const char* JISStr, char* GBBuf, int nSize) 
{ 
	static wchar_t wbuf[2048];
	int nLen; 

	nLen = strlen(JISStr)+1; 
	if (wbuf==NULL) return 0; 
	nLen = MultiByteToWideChar(936, 0, JISStr, 
		nLen, wbuf, nLen); 
	if (nLen > 0) 
		nLen = WideCharToMultiByte(932, 0, wbuf, 
		nLen, GBBuf, nSize, NULL, 0); 
	return nLen; 
}
write_stream_t index_list;
FILE* pTmpFile;
void init_temp_file()
{
	char szTempFileName[MAX_PATH];
	GetCurrentDirectory(sizeof(szTempFileName),szTempFileName);
	strcat(szTempFileName,"\\cache.tmp");
	DeleteFile(szTempFileName);
	pTmpFile = fopen(szTempFileName,"wb+");
	fseek(pTmpFile,0,SEEK_SET);
}
unsigned int write_to_temp_file(unsigned char* buf,size_t size)
{
	int offs;
	offs = ftell(pTmpFile);
	fwrite(buf,size,1,pTmpFile);
	return offs;
}

void free_temp_file()
{
	char szTempFileName[MAX_PATH];
	fclose(pTmpFile);
	GetCurrentDirectory(sizeof(szTempFileName),szTempFileName);
	strcat(szTempFileName,"\\cache.tmp");
	DeleteFile(szTempFileName);
}
uint get_file_size(FILE* fp)
{
	uint size;
	if(fp==NULL)
	{        
			return 0;
	}    
	fseek( fp, 0L, SEEK_END );
	size=ftell(fp);
	fseek( fp, 0L, SEEK_SET );
	return size;
}
void xor_buf(byte* buf,uint size)
{
	uint key_max = 0;

	key_max = (size >> 2) & 7;
	for(uint i=0;i<size;i+=4)
	{
			*(DWORD*)(buf+i) ^= xor_key_buffer[(i / 4) & key_max];
	}
}
void xor_index(byte* buf)
{
	for(uint xor_count = 0;xor_count < (sizeof(pkg_file_t)/4);xor_count++)
	{
		((DWORD*)buf)[xor_count] ^= xor_key_buffer[xor_count & 7];
	}
}
bool pack_to_data(char* disk_file_name,char* pack_name)
{
	pkg_file_t pkg_item;
	FILE* f;
	uint file_size;
	byte* file_buf;
	
	f = fopen(disk_file_name,"rb");
	if(f)
	{
		printf("packet file:%s\n",pack_name);
		file_size = get_file_size(f);
		file_buf = new byte[file_size+10];
		if(fread(file_buf,1,file_size,f)!=file_size)
			abort();
		xor_buf(file_buf,file_size);
		CovtGbkToShiftJIS(pack_name,pkg_item.file_name,sizeof(pkg_item.file_name));
		pkg_item.file_size = file_size;
		pkg_item.file_offset_high = 0;
		pkg_item.file_offset = write_to_temp_file(file_buf,file_size+10);

	
		WS_WriteBytes(&index_list,(byte*)&pkg_item,sizeof(pkg_item));
		fclose(f);
		return true;
	}
	return false;
}
void fix_index_offset()
{
	pkg_file_t * pkg_item;
	uint addin_offset = sizeof(pkg_header_t) + index_list.size;
	uint listsize = index_list.size / sizeof(pkg_file_t);

	for(uint i=0;i<listsize;i++)
	{
		pkg_item = (pkg_file_t*)(index_list.buf + i*sizeof(pkg_file_t));
		pkg_item->file_offset += addin_offset;
		xor_index((byte*)pkg_item);
	}
}
int main(int argc, char* argv[])
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	
	char szTmpFileName[MAX_PATH];
	char szDirectory[MAX_PATH];
	char szPackDirectory[MAX_PATH];
	char szPackfileName[MAX_PATH];

	pkg_header_t file_header;
	uint data_size;
	uint file_size;
	
	FILE* f;
	byte* m_buf;
	
	int filecount;
	
	GetCurrentDirectory(sizeof(szDirectory),szDirectory);
	
	strcpy(szPackfileName,szDirectory);
	strcpy(szPackDirectory,szDirectory);
	
	strcat(szPackDirectory,"\\output\\*.*");
	strcat(szPackfileName,"\\new.pkg");
	
	WS_Init(&index_list);
	init_temp_file();

	f = fopen(szPackfileName,"wb+");
	if(!f)
	{
		printf("Can't Create new file!\n");
		return 0;
	}
	hFind = FindFirstFileA(szPackDirectory,&findData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	filecount = 0;
	do
	{
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			strcpy(szTmpFileName,szDirectory);
			strcat(szTmpFileName,"\\output\\");
			strcat(szTmpFileName,findData.cFileName);
			
			if(pack_to_data(szTmpFileName,findData.cFileName))
			{
				filecount++;
			}
		}
	}while(FindNextFile(hFind,&findData));

	data_size = get_file_size(pTmpFile);
	file_size = data_size + index_list.size + sizeof(file_header);
	fix_index_offset();

	file_header.file_count = filecount ^ 0xA13BB527;
	file_header.file_size = file_size ^ 0xA13BB527;
	//写入文件头
	fwrite(&file_header,sizeof(file_header),1,f);
	//写入索引文件
	fwrite(index_list.buf,index_list.size,1,f);

	m_buf = (byte*)malloc(10240000);
	fseek( pTmpFile, 0L, SEEK_SET );
	uint length;
	do
	{
		length = fread(m_buf,1,10240000,pTmpFile);
		
		if(length)
		{
			fwrite(m_buf,length,1,f);
		}
	}while(length);

	fclose(f);
	free(m_buf);
	free_temp_file();
	WS_Release(&index_list);

	return 0;
}
