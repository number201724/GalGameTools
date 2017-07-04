#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>
#include <direct.h>

typedef unsigned char byte;

#include "zlib.h"

typedef struct cat_scene_header_s
{
	char magic[8]; // CatScene
	uint32_t compress_length;
	uint32_t decompress_length;
}cat_scene_header_t;
int main(int argc,char** args)
{
	FILE* f;
	uint32_t file_length;
	byte* buffer;
	byte* decompress_data;
	cat_scene_header_t* header;
	
	
	f = fopen("H:\\nanacan\\ÃÃÕ{½ÌÈÕÓ›¥Õ¥¡¥ó¥Ç¥£¥¹¥¯\\scene\\him01.cst","rb");
	fseek(f,0,SEEK_END);
	file_length = ftell(f);
	fseek(f,0,SEEK_SET);
	buffer = malloc(file_length);
	fread(buffer,file_length,1,f);

	header = buffer;

	if(strncmp(header->magic,"CatScene",8)!=0)
	{
		printf("unknown file\n");
		return -1;
	}
	decompress_data = malloc(header->decompress_length);

	buffer += sizeof(cat_scene_header_t);

	if(uncompress(decompress_data,&header->decompress_length,buffer,header->compress_length) == Z_OK)
	{
		
	}
	return 0;
}