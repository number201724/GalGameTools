#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdint.h>
#include <direct.h>
#include "zlib.h"
typedef struct hg3_header_s
{
	char magic[4];
	uint32_t version; // 0xC
	uint32_t unknown1;
	uint32_t unknown2;
}hg3_header_t;

typedef struct tree_node_s
{
	char magic[8]; // stdinfo
	uint32_t next_block;
	uint32_t node_length;
}tree_node_t;
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
typedef struct hash_table_s
{
	uint32_t hash[4];
}hash_table_t;

//cptype
tree_node_t* find_node_item(tree_node_t* head_dat,char* name)
{
	tree_node_t* node = head_dat;
	
	do
	{
		if(strcmp(node->magic,name)==0)
		{
			return node;
		}
		node = (tree_node_t*)((uint8_t*)node + node->next_block);
	}while(node->next_block);
	return NULL;
}

uint8_t* get_node_data(tree_node_t* node)
{
	return (uint8_t*)&node[1];
}
int main()
{
	FILE* f;
	size_t length;
	uint8_t* hg3_buf;

	uint8_t* hgreader;
	
	f = fopen("H:\\nanacan\\妹調教日記ファンディスク\\cg_t0.hg3","rb");
	if(!f)
	{
		printf("can't open file\n");
		return 0;
	}
	fseek(f,0,SEEK_END);
	length = ftell(f);
	fseek(f,0,SEEK_SET);
	hg3_buf = new uint8_t[length];
	fread(hg3_buf,length,1,f);
	fclose(f);

	if(*(uint32_t*)hg3_buf != 0x332D4748) // HG-3
	{
		printf("错误的文件,啊喂!!!\n");
		return 0;
	}

	hgreader = hg3_buf;

	hgreader += sizeof(uint32_t);
	hgreader += sizeof(hash_table_t);


	tree_node_t* node = find_node_item((tree_node_t*)hgreader,"stdinfo");
	
	if(!node)
	{
		printf("无效的树结构........喂喂喂\n");
		return 0;
	}

	img_info_t* img_info = (img_info_t*)get_node_data(node);


	//if(find_node_item((tree_node_t*)hgreader,"img_jpg"))
	//{

	//}
	







	return 0;
}