/*
MajiroArc V3.0
code by 201724
QQ:527806988
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "writestream.h"

typedef struct arc_header_s
{
	char sign[16];
	unsigned int filecount;
	unsigned int stroff;
	unsigned int dataoff;
}arc_header_t;
typedef struct arc_idx_s
{
	__int64 crc64;
	unsigned int dataoffs;
	unsigned int size;
}arc_idx_t;
typedef struct ArcObject
{
	__int64 n;
	arc_idx_t* record;
}ArcObject;
FILE* pTmpFile;
write_stream_t string_stream;
write_stream_t index_stream;
#define __PAIR__(high, low) (((unsigned __int64)(high)<<sizeof(high)*8) | low)
__int64 CRC64_String(char* str)
{
	int i,j,x;
	int hashidx;
	int string_length;
	static int init_crc64 = 0;
	static unsigned __int64 crc64_table[256];
	unsigned __int64 hashvalue = 0;
	unsigned int retvlow,retvhig;
	
	if(!init_crc64)
	{
		init_crc64 = 1;
		for(i=0;i<256;i++)
		{
			hashvalue = i;
			for(j=0;j<8;j++)
			{
				if(hashvalue & 1)
				{
						hashvalue ^= 0x85E1C3D753D46D27;
				}
				hashvalue = hashvalue >> 1;
			}
			crc64_table[i] = hashvalue;
		}
	}
	retvlow = -1;
	retvhig = -1;
	string_length = strlen(str);
	for(x=0;x<string_length;x++)
	{
		hashidx = (int)((unsigned char)retvlow ^ (unsigned char)str[x]);
		hashvalue = ((__PAIR__(retvhig,retvlow)) >> 8);
		retvlow = hashvalue ^ crc64_table[hashidx];
		retvhig = (hashvalue ^ crc64_table[hashidx]) >> 32;
	}
	hashvalue = ~(__PAIR__(retvhig,retvlow));
	return hashvalue;
}
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

unsigned int get_file_size(FILE* fp)
{
	unsigned int size;
	if(fp==NULL)
	{        
			return 0;
	}    
	fseek( fp, 0L, SEEK_END );
	size=ftell(fp);
	fseek( fp, 0L, SEEK_SET );
	return size;
}

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
void pack_to_data(char* filename,char* name)
{
	FILE* f;
	unsigned int file_size;
	unsigned char* buf;
	unsigned int offs;
	arc_idx_t m_idx;
	char szJISName[MAX_PATH];
	
	f = fopen(filename,"rb");
	if(f)
	{
		file_size = get_file_size(f);
		buf = (unsigned char*)malloc(file_size);
		fread(buf,file_size,1,f);
		fclose(f);
		offs = write_to_temp_file(buf,file_size);
		CovtGbkToShiftJIS(name,szJISName,sizeof(szJISName));
		m_idx.crc64 = CRC64_String(szJISName);
		
		m_idx.size = file_size;
		m_idx.dataoffs = offs;

		WS_WriteBytes(&index_stream,(char*)&m_idx,sizeof(m_idx));
		WS_WriteString(&string_stream,szJISName);
	}
}

void binSort(ArcObject * pvector) {
	__int64 i, j, left, mid, right;
	arc_idx_t temp;
	arc_idx_t *data = pvector->record;
	for( i = 1; i < pvector->n; i++ )
	{         temp = data[i];
	left = 0; 
	right = i-1;
	while (left <= right)
	{             
		mid = (left + right)/2;     
		if (temp.crc64 < data[mid].crc64)                 
			right = mid-1;     
		else left = mid+1;
	}         
	for (j = i-1;   j >= left;   j--)             
		data[j+1] = data[j];               
	if (left != i) 
		data[left] = temp;
	}
}
void tree_node(int count,arc_idx_t* orig)
{
	ArcObject ArcObj;
	
	ArcObj.n = count;
	ArcObj.record = orig;
	binSort(&ArcObj);
	
}
void PackToFormatFile(FILE* f,int fileCount)
{
	unsigned char* m_buf;
	unsigned int tmp_file_size;
	unsigned int cur_file_tell;
	unsigned int for_count;
	arc_idx_t * arc_idx;
	arc_header_t arc_header;
	size_t length;
	strcpy(arc_header.sign,"MajiroArcV3.000");
	arc_header.filecount = (index_stream.size / sizeof(arc_idx_t));
	arc_idx = (arc_idx_t *)index_stream.buf;
	arc_header.stroff = index_stream.size + sizeof(arc_header_t);
	arc_header.dataoff = arc_header.stroff + string_stream.size;
	fwrite((void*)&arc_header,sizeof(arc_header),1,f);
	for(for_count=0;for_count<arc_header.filecount;for_count++)
	{
		arc_idx[for_count].dataoffs += arc_header.dataoff;
	}
	tree_node(arc_header.filecount,arc_idx);
	fwrite(index_stream.buf,index_stream.size,1,f);
	fwrite(string_stream.buf,string_stream.size,1,f);
	m_buf = malloc(10240000);
	fseek( pTmpFile, 0L, SEEK_SET );
	tmp_file_size = get_file_size(pTmpFile);
	do
	{
		length = fread(m_buf,1,10240000,pTmpFile);
		
		if(length)
		{
			fwrite(m_buf,length,1,f);
		}
	}while(length);
}
void main()
{
	WIN32_FIND_DATA findData;
	HANDLE hFind;
	char szTmpFileName[MAX_PATH];
	char szDirectory[MAX_PATH];
	char szJIFName[MAX_PATH];
	char szPackDirectory[MAX_PATH];
	char szPackfileName[MAX_PATH];
	
	FILE* f;
	FILE* fr;
	
	unsigned char* file_buf;
	unsigned int file_size;
	
	arc_idx_t idx;
	
	int filecount;
	
	__int64 CRC;
	
	GetCurrentDirectory(sizeof(szDirectory),szDirectory);
	
	strcpy(szPackfileName,szDirectory);
	strcpy(szPackDirectory,szDirectory);
	
	strcat(szPackDirectory,"\\output\\*.*");
	strcat(szPackfileName,"\\new.arc");
	
	
	f = fopen(szPackfileName,"wb+");
	if(!f)
	{
		printf("Can't Create new file!\n");
		return;
	}
	hFind = FindFirstFileA(szPackDirectory,&findData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		return;
	}
	filecount = 0;
	init_temp_file();
	WS_Init(&string_stream);
	WS_Init(&index_stream);
	do
	{
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			strcpy(szTmpFileName,szDirectory);
			strcat(szTmpFileName,"\\output\\");
			strcat(szTmpFileName,findData.cFileName);
			filecount++;
			pack_to_data(szTmpFileName,findData.cFileName);
			printf("packet file:%s\n",findData.cFileName);
		}
	}while(FindNextFile(hFind,&findData));
	
	PackToFormatFile(f,filecount);
	
	free_temp_file();
	MessageBox(NULL,"文件打包成功,新文件new.arc!","",MB_OK);
}