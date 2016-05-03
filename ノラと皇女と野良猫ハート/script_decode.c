#include <stdio.h>
#include <string.h>
#include "zlib.h"
#include <stdint.h>
#include <stdlib.h>


#pragma pack(1)

typedef union I64Integer {
	struct {
		uint32_t LowPart;
		uint32_t HighPart;
	};
	struct {
		uint32_t LowPart;
		uint32_t HighPart;
	} u;
	uint64_t QuadPart;
} I64Integer;

struct FileNode
{
	I64Integer hash;
	unsigned char type;
	uint32_t offset;
	uint32_t compr_size;
	uint32_t uncompr_size;
};

#pragma pack()

#define DECODE_KEY 0x26ACA46E
#define GAME_HASH 0x51994705

typedef struct crc64_s{
	I64Integer hash;
  uint64_t polynomial;
} crc64_t;


void CRC64_Init(crc64_t* crc) {
    crc->hash.QuadPart = 0x0;
    crc->polynomial = 0x42F0E1EBA9EA3693;
}

void CRC64_Update(crc64_t* crc, char* message, uint32_t length) {
    uint32_t i, j;
    uint64_t k;
    for(i = 0; i < length; i++) {
        crc->hash.QuadPart ^= ((uint64_t)message[i] << 56);
        for(j = 0; j < 8; j++) {
            k = crc->hash.QuadPart & 0x8000000000000000;
            crc->hash.QuadPart <<= 1;
            if(k) {
                crc->hash.QuadPart ^= crc->polynomial;
            }
        }
    }
}
void decode_nodes(uint32_t count, struct FileNode *nodes)
{
	uint32_t i;
	
	for(i = 0; i < count; i++)
	{
		nodes[i].offset ^= nodes[i].hash.LowPart;
		nodes[i].compr_size ^= nodes[i].hash.LowPart;
		nodes[i].uncompr_size ^= nodes[i].hash.LowPart;
		
		printf("%d %08x %08x %08x \n",i,nodes[i].offset,nodes[i].compr_size,nodes[i].uncompr_size );
	}
}

void uncompr_buffer(FILE *fp, uint32_t index, struct FileNode node)
{
	unsigned char *data, *uncompr_data;
	uint32_t *xor_buf;
	uint32_t i,xor_count;
	unsigned long dst_size;
	FILE *out;
	char s[128];
	
	fseek(fp,node.offset,SEEK_SET);
	
	data = malloc(node.compr_size);
	xor_buf = (uint32_t*)data;
	
	fread(data,node.compr_size,1,fp);
	xor_count = (node.compr_size / 4);
	
	printf("size:%08x   %08x    %08x\n",node.compr_size,node.uncompr_size,node.uncompr_size*2);
	printf("xor_count:%08x\n",xor_count);
	printf("xor:%08x\n",(GAME_HASH ^ node.hash.LowPart));
	
	for( i = 0; i < xor_count; i++)
	{
		xor_buf[i] ^= (GAME_HASH ^ node.hash.LowPart);
	}
	
	dst_size = node.uncompr_size * 2;
	uncompr_data = malloc((uint32_t)dst_size);
	
	if(uncompress(uncompr_data,&dst_size,data,node.compr_size) == Z_OK)
	{
		printf("ok %08lx\n",dst_size);
		sprintf(s,"script/%d.txt",index);
		out = fopen(s,"wb");
		if(out){
			fwrite(uncompr_data,dst_size,1,out);
			fclose(out);
		}
	}
	else
	{
		printf("failed\n");
	}
	
	free(data);
	free(uncompr_data);
	
	
	
}
int main()
{
	FILE *fp;
	uint32_t file_count;
	uint32_t i;
	struct FileNode *nodes;
	
	
	fp = fopen("script.dat","rb");
	
	if(!fp){
		printf("file not found\n");
		return 1;
	}
	fread(&file_count,4,1,fp);
	
	file_count ^= DECODE_KEY;
	
	nodes = malloc(sizeof(struct FileNode) * file_count);
	
	for(i = 0; i < file_count; i++ )
	{
		fread(&nodes[i],sizeof(struct FileNode),1,fp);
	}
	
	decode_nodes(file_count,nodes);
	
	for(i = 0; i < file_count; i++ )
	{
		uncompr_buffer(fp, i, nodes[i]);
	}

	
	return 0;
}