#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>
#include <direct.h>
#include <vector>
using namespace std;

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

typedef struct cat_string_s
{
	uint32_t index;
	char* str;
	uint32_t length;
	bool select;
}cat_string_t;

byte* sce_data;
cat_system_t cat_system_save;
vector <cat_string_t*> cat_string_list;

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

void add_string(uint32_t index,byte* ptr,bool select=false)
{
	cat_string_t* item = new cat_string_t;
	item->index = index;
	item->length = strlen((char*)ptr) + 1;
	item->str = new char[item->length];
	strcpy(item->str,(char*)ptr);
	item->select = select;
	cat_string_list.push_back(item);
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



int main(int argc,char** args)
{
	FILE* f;
	size_t file_length;
	f = fopen("H:\\nanacan\\妹調教日記ファンディスク\\nanacan\\intextract\\scene\\him07.cst","rb");

	fseek(f,0,SEEK_END);
	file_length = ftell(f);
	fseek(f,0,SEEK_SET);
	sce_data = new byte[file_length];
	fread(sce_data,file_length,1,f);
	fclose(f);

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
		case 0x21:
			add_string(n,p);
			break;
		//脚本/选择支
		case 0x30:
			process_sub_item(n,p);
			break;
		default:
			break;
		}
	}

	return 0;
}