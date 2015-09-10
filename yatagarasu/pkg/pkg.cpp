#include <windows.h>
#include <stdio.h>

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
	uint encode_size;
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

void write_to_file(char* file_name,byte* buf,uint size)
{
	static char szOutputPath[MAX_PATH];
	static bool init_file_path = false;

	char szFileName[MAX_PATH];

	FILE* f;
	if(!init_file_path)
	{
		GetCurrentDirectory(sizeof(szOutputPath),szOutputPath);
		strcat(szOutputPath,"\\output\\");
		CreateDirectory(szOutputPath,NULL);
		init_file_path = true;
	}
	printf("expans:%s\n",file_name);
	sprintf(szFileName,"%s%s",szOutputPath,file_name);
	f = fopen(szFileName,"wb+");
	if(f)
	{
		fwrite(buf,size,1,f);
		fclose(f);
	}
}
void expans_file(HANDLE hMap,pkg_file_t* fileinfo)
{
	char gb_name[128];
	byte* file_buffer;
	byte* map_buffers;
	uint map_offset;
	uint map_size;
	uint map_read_offset;
	uint key_max = 0;

	CovtShiftJISToGB((const char*)fileinfo->file_name,gb_name,sizeof(gb_name));
	map_offset = fileinfo->file_offset & 0xFFFF0000;
	map_read_offset = fileinfo->file_offset - map_offset;
	map_size = fileinfo->file_size + map_read_offset;

	file_buffer = new byte[fileinfo->file_size+10];

	map_buffers = (byte*)MapViewOfFile(hMap,FILE_MAP_READ,0,map_offset,map_size);
	memcpy(file_buffer,map_buffers+map_read_offset,fileinfo->file_size);
	UnmapViewOfFile(map_buffers);


	uint xor_size;
	uint new_xor_size;
	uint xor_count = 0;
	uint xor_offfset = 0;
	if(fileinfo->encode_size > 0 && fileinfo->encode_size < fileinfo->file_size)
	{
		xor_size = fileinfo->encode_size;
	}
	else
	{
		xor_size = fileinfo->file_size;
	}

	if(fileinfo->encode_size > 0 && fileinfo->encode_size <= fileinfo->file_size)
	{
		new_xor_size = fileinfo->encode_size;
	}
	else
	{
		new_xor_size = fileinfo->file_size - 4;
	}
	key_max = (fileinfo->file_size >> 2) & 7;
	if(new_xor_size)
	{
		do
		{
			*(DWORD*)(file_buffer+xor_count) ^= xor_key_buffer[(xor_count / 4) & key_max];
			xor_offfset = xor_count + 3;
			xor_count += 4;
		}while(xor_count < new_xor_size);
	}

	if(xor_size != fileinfo->encode_size)
	{
		uint sub_xor_size = xor_size-xor_offfset;
		uint sub_xor_count = 0;
		uint xor_buf_offset;
		if(sub_xor_size > 1)
		{
			sub_xor_size--;
			xor_buf_offset = xor_key_buffer[(xor_count / 4) & key_max];
			do
			{
				file_buffer[sub_xor_count + xor_offfset + 1] ^= (byte)(xor_buf_offset >> (sub_xor_count * 8));
				sub_xor_count++;
			}while(sub_xor_count < sub_xor_size);

		}
	}


	write_to_file(gb_name,file_buffer,fileinfo->file_size);
	delete file_buffer;
}
int main(int argc, char* argv[])
{
	HANDLE hFile;
	HANDLE hMap;
	DWORD fileSize;
	PVOID pMapData;
	pkg_header_t header;
	
	byte* file_name_start;
	pkg_file_t file_info_buffer;

	byte* file_idx_info;
	

	hFile = CreateFile("G:\\ÐÇ²Ê¤Î¥ì¥¾¥Ê¥ó¥¹\\reso000_000.pkg",
						GENERIC_READ,
						0,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	hMap = CreateFileMapping(hFile,NULL,PAGE_READONLY,0,0,NULL);
	
	if(!hMap)
	{
		printf("%d",GetLastError());
		getchar();
	}
	fileSize = GetFileSize(hFile,NULL);
	
	if(fileSize < 0xA00000)
	{
		pMapData = MapViewOfFile(hMap,FILE_MAP_READ,0,0,fileSize);
		file_idx_info = new byte[fileSize];
		memcpy(file_idx_info,pMapData,fileSize);
	}
	else
	{
		pMapData = MapViewOfFile(hMap,FILE_MAP_READ,0,0,0xA00000);
		file_idx_info = new byte[0xA00000];
		memcpy(file_idx_info,pMapData,0xA00000);
	}
	UnmapViewOfFile(pMapData);

	memcpy(&header,file_idx_info,8);
	header.file_count ^= 0xA13BB527;
	header.file_size ^= 0xA13BB527;
	
	file_name_start = file_idx_info + sizeof(header);
	for(uint file_count = 0;file_count<header.file_count;file_count++)
	{
		memcpy(&file_info_buffer,(file_name_start + (sizeof(pkg_file_t) * file_count)),sizeof(pkg_file_t));
		for(uint xor_count = 0;xor_count < (sizeof(pkg_file_t)/4);xor_count++)
		{
			((DWORD*)&file_info_buffer)[xor_count] ^= xor_key_buffer[xor_count & 7];
		}
		expans_file(hMap,&file_info_buffer);
	}


	CloseHandle(hMap);
	CloseHandle(hFile);
	delete file_idx_info;
	return 0;
}
