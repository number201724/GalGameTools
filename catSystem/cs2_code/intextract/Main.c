#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>
#include <direct.h>
#include "zlib.h"
typedef unsigned char byte;


typedef struct file_pair_s
{
	char file_name[64];
	uint32_t offset;
	uint32_t length;
}file_pair_t;

typedef struct cat_scene_header_s
{
	char magic[8]; // CatScene
	uint32_t compress_length;
	uint32_t decompress_length;
}cat_scene_header_t;
typedef struct fes_header_s
{
	char magic[4]; // CatScene
	uint32_t compress_length;
	uint32_t decompress_length;
	uint32_t zero;
}fes_header_t;

void cat_scene_decompress(char* extract_path,byte* data,size_t length)
{
	byte* decompress_data;
	cat_scene_header_t* header;
	FILE* wf;

	header = data;
	decompress_data = malloc(header->decompress_length);
	
	data += sizeof(cat_scene_header_t);

	if(uncompress(decompress_data,&header->decompress_length,data,header->compress_length) == Z_OK)
	{
		wf = fopen(extract_path,"wb+");

		fwrite(decompress_data,header->decompress_length,1,wf);
		fclose(wf);
	}
	else
	{
		printf("error uncompress\n");
		getchar();
	}

	free(decompress_data);
}
void fes_decompress(char* extract_path,byte* data,size_t length)
{
	byte* decompress_data;
	fes_header_t* header;
	FILE* wf;

	header = data;
	decompress_data = malloc(header->decompress_length);

	data += sizeof(fes_header_t);

	if(uncompress(decompress_data,&header->decompress_length,data,header->compress_length) == Z_OK)
	{
		wf = fopen(extract_path,"wb+");

		fwrite(decompress_data,header->decompress_length,1,wf);
		fclose(wf);
	}
	else
	{
		printf("error uncompress\n");
		getchar();
	}

	free(decompress_data);
}

int main(int argc,char** args)
{
	char magic[4];
	file_pair_t* pair_list;
	uint32_t read_length;
	uint32_t file_count;
	uint32_t extract_index;
	FILE* wf ;

	char extract_path[260];
	char extract_dir[260];
	size_t malloc_length;
	byte* temp_buffer;
	FILE* f; 

	size_t string_length;
	
	if(argc < 2)
	{
		printf("input file name!!!\n");
		return -1;
	}

	string_length = strlen(args[1]);

	for(string_length;string_length != 0;string_length--)
	{
		if(args[1][string_length] == '\\')
		{
			strcpy(extract_dir,&args[1][string_length+1]);
			break;
		}
	}

	string_length = strlen(extract_dir);
	for(string_length;string_length != 0;string_length--)
	{
		if(extract_dir[string_length] == '.')
		{
			extract_dir[string_length] = 0;
			break;
		}
	}


	//printf("%s\n",args[1]);
	
	f = fopen(args[1],"rb");

	if(!f)
	{
		printf("open file failed.......\n");
		return -1;
	}

	fread(magic,1,4,f);

	if(strcmp(magic,"KIF")!=0)
	{
		printf("error file magic\n");
		return -1;
	}

	fread(&file_count,1,4,f);

	read_length = file_count * sizeof(file_pair_t);
	pair_list = malloc(read_length);


	fread(pair_list,read_length,1,f);
	malloc_length = 0;
	temp_buffer = malloc(0);
	for(extract_index=0;extract_index<file_count;extract_index++)
	{
		if(pair_list[extract_index].length > malloc_length)
		{
			free(temp_buffer);
			temp_buffer = malloc(pair_list[extract_index].length);
		}
		fseek(f,pair_list[extract_index].offset,SEEK_SET);

		fread(temp_buffer,pair_list[extract_index].length,1,f);

		_mkdir(extract_dir);

		sprintf(extract_path,"%s\\%s",extract_dir,pair_list[extract_index].file_name);

		if(strncmp(temp_buffer,"CatScene",8)==0)
		{
			cat_scene_decompress(extract_path,temp_buffer,pair_list[extract_index].length);
		}
		else if(strncmp(temp_buffer,"FES",8)==0)
		{
			fes_decompress(extract_path,temp_buffer,pair_list[extract_index].length);
		}
		else
		{
			wf = fopen(extract_path,"wb+");

			fwrite(temp_buffer,pair_list[extract_index].length,1,wf);
			fclose(wf);
		}
		printf("extract file:%s\n",pair_list[extract_index].file_name);
	}

	printf("extract file done\n");

	getchar();
	return 0;
}