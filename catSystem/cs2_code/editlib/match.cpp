#pragma warning(disable:4786)
#pragma warning(disable:2679)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <windows.h>

#include <direct.h>
#include <vector>
#include <map>
#include "zlib.h"
using namespace std;

#pragma comment(lib,"zlib.lib")

typedef unsigned int uint32_t;
typedef unsigned char byte;

typedef struct cat_sce_header_s
{
	uint32_t data_length;
	uint32_t unknown;
	uint32_t offset_index_table;
	uint32_t offset_res_table;
}cat_sce_header_t;

typedef struct cat_system_s
{
	uint32_t * opcode_table;
	uint32_t * index_table;
	byte * res_table;

	uint32_t index_count;
}cat_system_t;

typedef struct cat_scene_header_s
{
	char magic[8]; // CatScene
	uint32_t compress_length;
	uint32_t decompress_length;
}cat_scene_header_t;

typedef struct cat_string_s
{
	uint32_t index;
	char* str;
	char* newstr;
	uint32_t length;
	bool select;
}cat_string_t;

byte* sce_data;
uint32_t sce_length;
cat_system_t cat_system_save;
vector <cat_string_t*> cat_string_list;
map <unsigned char*,cat_string_t*> cat_tree;
void make_struct(byte* stream)
{
	cat_sce_header_t* header;

	header = (cat_sce_header_t*)stream;

	stream += sizeof(cat_sce_header_t);

	cat_system_save.opcode_table = (uint32_t*)stream;
	cat_system_save.index_table = (uint32_t *)&stream[header->offset_index_table];
	cat_system_save.res_table = &stream[header->offset_res_table];

	cat_system_save.index_count = ((uint32_t)cat_system_save.res_table - (uint32_t)cat_system_save.index_table) / sizeof(uint32_t);
}
/*
01B2B530                                   81 45                      丒俈
*/
void add_string(uint32_t index,byte* ptr,bool select=false)
{
	
	cat_string_t* item = new cat_string_t;
	item->index = index;
	item->newstr = 0;
	item->length = strlen((char*)ptr) + 1;
	item->str = new char[item->length];
	strcpy(item->str,(char*)ptr);
	item->select = select;
	cat_string_list.push_back(item);

	cat_tree[ptr] = item;
}
byte* get_data(uint32_t index)
{
	uint32_t ptr = cat_system_save.index_table[index];
	return &cat_system_save.res_table[ptr];
}
void process_sub_item(uint32_t& index,byte* ptr)
{
	if(!strncmp((char*)ptr,"select",6) || !strncmp((char*)ptr,"fselect",7))
	{
		while(true)
		{
			index++;
			if(index >= cat_system_save.index_count)
				break;
			byte* p = get_data(index);
			p++;
			if(*p++ == 0x30)
			{
				add_string(index,p,true);
			}
			else
			{
				break;
			}
		}
	}
}
uint32_t MatchCompressFile()
{
	cat_scene_header_t* cat_scene;
	byte* cat_new_buf;
	uint32_t cat_new_buf_length = sce_length;
	if(strncmp((char *)sce_data,"CatScene",8)==0)
	{
		cat_scene = (cat_scene_header_t*)sce_data;
		cat_new_buf = new byte[cat_scene->decompress_length];
		if(uncompress(cat_new_buf,(unsigned long *)&cat_scene->decompress_length,(sce_data+sizeof(cat_scene_header_t)),cat_scene->compress_length)!=Z_OK)
		{
			delete cat_new_buf;
			return -1;
		}
		delete sce_data;
		sce_data = cat_new_buf;
		cat_new_buf_length = cat_scene->decompress_length;
	}

	sce_length = cat_new_buf_length;


	return cat_new_buf_length;
}
extern "C" BOOL WINAPI Match(char* szFileName)
{
	FILE* f = fopen(szFileName,"rb");
	if(!f) return FALSE;
	size_t file_length;

	fseek(f,0,SEEK_END);
	file_length = ftell(f);
	fseek(f,0,SEEK_SET);
	sce_data = new byte[file_length];
	fread(sce_data,file_length,1,f);
	fclose(f);

	


	sce_length = file_length;
	file_length = MatchCompressFile();
	if(file_length == -1)
	{
		delete sce_data;
		return FALSE;
	}

	make_struct(sce_data);

	for(uint32_t n=0;n<cat_system_save.index_count;n++)
	{
		byte* p = get_data(n);
		p++;
		switch(*p++)
		{
		case 2:
			break;
		//单项文本
		case 0x20:
		//case 0x21:
			
			add_string(n,p);
			break;
		//脚本/选择支
		case 0x30:
			//MessageBoxA(NULL,"Hello","",MB_OK);
			process_sub_item(n,p);
			break;
		default:
			break;
		}
	}

	return TRUE;
}

extern "C" VOID WINAPI Release()
{
	delete sce_data;

	for(size_t n=0;n<cat_string_list.size();n++)
	{
		delete cat_string_list[n]->str;
		if(cat_string_list[n]->newstr)
			delete cat_string_list[n]->newstr;
		delete cat_string_list[n];
	}
	cat_string_list.clear();
}
extern "C" UINT WINAPI GetCount()
{
	return cat_string_list.size();
}

int CovtShiftJISToGB(const char* JISStr, char* GBBuf, int nSize) 
{ 
	static wchar_t wbuf[2048];
	int nLen; 

	nLen = strlen(JISStr)+1; 
	if (wbuf==NULL) return false; 
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
	if (wbuf==NULL) return false; 
	nLen = MultiByteToWideChar(936, 0, JISStr, 
		nLen, wbuf, nLen); 
	if (nLen > 0) 
		nLen = WideCharToMultiByte(932, 0, wbuf, 
		nLen, GBBuf, nSize, NULL, 0); 
	return nLen; 
}
char* GetGBKChars(char* instr)
{
	static char szText[1024];
	CovtShiftJISToGB(instr,szText,sizeof(szText));
	return szText;
}
extern "C" char* WINAPI GetString(ULONG Index)
{
	return GetGBKChars(cat_string_list[Index]->str);
}
extern "C" char* WINAPI GetNewString(ULONG Index)
{
	if(cat_string_list[Index]->newstr)
	{
		return cat_string_list[Index]->newstr;
	}
	return "";
}
extern "C" VOID WINAPI ModifyString(ULONG Index,char* newString)
{
	if(!newString) return;
	if(cat_string_list[Index]->newstr)
		delete cat_string_list[Index]->newstr;
	cat_string_list[Index]->newstr = new char[strlen(newString)+1];
	strcpy(cat_string_list[Index]->newstr,newString);
}

void OnProcessText(byte*& new_buf,uint32_t& offs,uint32_t index,char* new_str)
{
	byte* stream;
	cat_sce_header_t* header;
	cat_system_t sys_tree;
	
	//0x1 0x20(0x21)
	uint32_t opcode_length = 3 + strlen(new_str);
	new_buf = (byte*)realloc(new_buf,offs + opcode_length);
	byte* org_buf = get_data(index);
	unsigned char* dst_buf = &new_buf[offs];

	memcpy(dst_buf,org_buf,2);
	dst_buf += 2;
	strcpy((char*)dst_buf,new_str);
	dst_buf -= 2;


	stream = new_buf;
	header = (cat_sce_header_t*)stream;
	stream += sizeof(cat_sce_header_t);

	sys_tree.opcode_table = (uint32_t*)stream;
	sys_tree.index_table = (uint32_t *)&stream[header->offset_index_table];
	sys_tree.res_table = &stream[header->offset_res_table];

	sys_tree.index_count = ((uint32_t)cat_system_save.res_table - (uint32_t)cat_system_save.index_table) / sizeof(uint32_t);

	sys_tree.index_table[index] = (uint32_t)offs;

	sys_tree.index_table[index] = (uint32_t)(dst_buf - sys_tree.res_table);

	offs += opcode_length;
}
/*
void process_sub_item(uint32_t& index,byte* ptr)
{
	if(!strncmp((char*)ptr,"select",6) || !strncmp((char*)ptr,"fselect",7))
	{
		while(true)
		{
			index++;
			if(index >= cat_system_save.index_count)
				break;
			byte* p = get_data(index);
			p++;
			if(*p++ == 0x30)
			{
				add_string(index,p,true);
			}
			else
			{
				break;
			}
		}
	}
}
*/
void OnProcessSelectText(byte*& new_buf,uint32_t& offsets,uint32_t& index,byte* ptr)
{
	if(!strncmp((char*)(ptr+2),"select",6) || !strncmp((char*)(ptr+2),"fselect",7))
	{
		while(true)
		{
			index++;
			if(index >= cat_system_save.index_count)
				break;
			byte* p = get_data(index);
			p++;
			if(*p++ == 0x30)
			{
				map<unsigned char*,cat_string_t*>::iterator iter;
				iter = cat_tree.find(p);
				if(iter != cat_tree.end())
				{
					if(iter->second->newstr)
					{
						OnProcessText(new_buf,offsets,index,iter->second->newstr);
					}
					else
					{
						OnProcessText(new_buf,offsets,index,GetGBKChars(iter->second->str));
					}
				}
			}
			else
			{
				break;
			}
		}
	}
}
extern "C" VOID WINAPI PackerData(char* szNewFileName)
{
	byte* new_buf = (byte*)malloc(sce_length);
	uint32_t offsets = sce_length;
	memcpy(new_buf,sce_data,sce_length);


	for(uint32_t n=0;n<cat_system_save.index_count;n++)
	{
		byte* p = get_data(n);
		p++;
		switch(*p++)
		{
		case 2:
			break;
		//单项文本
		case 0x20:
		//case 0x21:
			{
				map<unsigned char*,cat_string_t*>::iterator iter;
				iter = cat_tree.find(p);
				if(iter != cat_tree.end())
				{
					if(iter->second->newstr)
					{
						OnProcessText(new_buf,offsets,n,iter->second->newstr);
					}
					else
					{
						OnProcessText(new_buf,offsets,n,GetGBKChars(iter->second->str));
					}
				}
			}
			break;
		//脚本/选择支
		case 0x30:
			OnProcessSelectText(new_buf,offsets,n,p-2);
			break;
		default:
			break;
		}
	}
	*(unsigned long*)new_buf = (unsigned long)(offsets - sizeof(cat_scene_header_t));
	unsigned long compress_length = compressBound(offsets);

	byte * zbuf = new byte[compress_length];

	if(compress(zbuf,&compress_length,new_buf,offsets) == Z_OK)
	{
		cat_scene_header_t header;
		byte* f_buf = new byte[sizeof(cat_scene_header_t) + compress_length];
		
		strncpy(header.magic,"CatScene",8);
		header.compress_length = compress_length;
		header.decompress_length = offsets;
		memcpy(f_buf,&header,sizeof(cat_scene_header_t));
		memcpy(&f_buf[sizeof(cat_scene_header_t)],zbuf,compress_length);

		FILE* f = fopen(szNewFileName,"wb");
		fwrite(f_buf,sizeof(cat_scene_header_t) + compress_length,1,f);
		fclose(f);
		delete f_buf;
		

	}
	delete zbuf;
	free(new_buf);
}