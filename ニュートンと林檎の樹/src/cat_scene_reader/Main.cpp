#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>
#include <direct.h>
#include <vector>
#include <locale.h>
#include <Windows.h>
#include <atlconv.h>
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

char filename[256];
char selects_name[256];
char texts_name[256];

size_t count_of_type(bool select)
{
	size_t count = 0;

	for(size_t i = 0; i < cat_string_list.size();i++)
	{
		if(cat_string_list[i]->select == select)
		{
			count++;
		}
	}

	return count;
}

void write_text()
{
	if(cat_string_list.empty()) return;
	size_t count = count_of_type(false);

	if(count > 0)
	{
		FILE *texts = fopen(texts_name,"wb");
		fwrite("\xFF\xFE",2,1,texts);

		size_t index = 0;

		for(size_t i = 0; i < cat_string_list.size();i++)
		{
			if(!cat_string_list[i]->select)
			{
				USES_CONVERSION;

				wchar_t *text = A2W_CP(cat_string_list[i]->str, 932);
				fwprintf(texts,L"<%d>\r\n", index);
				fwprintf(texts,L"//%s\r\n", text);
				fwprintf(texts,L"%s\r\n", text);
				fwprintf(texts,L"\r\n");
				index++;
			}
		}

		fflush(texts);
		fclose(texts);
	}
}

void write_selects()
{
	if(cat_string_list.empty()) return;
	size_t count = count_of_type(true);

	if(count > 0)
	{
		FILE *selects = fopen(selects_name,"wb");
		fwrite("\xFF\xFE",2,1,selects);

		size_t index = 0;

		for(size_t i = 0; i < cat_string_list.size();i++)
		{
			if(cat_string_list[i]->select)
			{
				USES_CONVERSION;

				wchar_t *select_text = A2W_CP(cat_string_list[i]->str, 932);
				fwprintf(selects,L"<%d>\r\n", index);
				fwprintf(selects,L"//%s\r\n", select_text);
				fwprintf(selects,L"%s\r\n", select_text);
				fwprintf(selects,L"\r\n");
				index++;
			}
		}
	}
}
int main(int argc,char** args)
{
	FILE* f;
	size_t file_length;
	if(argc <= 1)
	{
		printf("filename not found\n");
		return 0;
	}
	strcpy(filename,args[1]);
	strcpy(selects_name,args[1]);
	strcpy(texts_name,args[1]);

	char *s = strrchr(texts_name,'.');
	if(s)
	{
		strcpy(s, ".txt");
	}

	s = strrchr(selects_name,'.');

	if(s)
	{
		strcpy(s, ".selects.txt");
	}

	f = fopen(filename,"rb");
	if(!f)
	{
		printf("source file missing.\n");
		return 0;
	}

	fseek(f,0,SEEK_END);
	file_length = ftell(f);
	fseek(f,0,SEEK_SET);
	sce_data = new byte[file_length];
	fread(sce_data,file_length,1,f);
	fclose(f);

	setlocale(0,"Japanese");
	make_struct(sce_data);

	for(uint32_t n=0;n<cat_system_save.index_count;n++)
	{
		byte* p = get_data(n);
		p++;
		//fprintf(stderr, "opcode:%d\n", *p);
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

	write_text();
	write_selects();

	return 0;
}