#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

typedef struct file_pair_s
{
	unsigned int offset;
	unsigned int length;
	unsigned int unk,unk1;
}file_pair_t;


void extract_buffer(unsigned char* src,unsigned char* dst,size_t length)
{
	unsigned char swap_buffer[0x1000];
	//k = control
	unsigned int d = 0, s = 0, k =0 , b = 0xFEE;
	unsigned int n;
	
	

	memset(&swap_buffer,0,sizeof(swap_buffer));

	while(d < length)
	{
		k = k >> 1;
		if(!(k & 0x100))
		{
			k = (((unsigned int)src[s]) | 0xFF00);
			++s;
		}

		n = src[s];
		if(k & 1)
		{
			swap_buffer[b] = n;
			dst[d] = n;
			s++;
			d++;
			b++;
			b = b & 0xFFF;
			if(&dst[d] >= &dst[length])
			{
				return;
			}
		}
		else
		{
			unsigned int w = src[s+1];
			n |= ((w & 0xF0) << 4); 
			w &= 0xF;

			unsigned int l = w + n + 2;
			s += 2;
			if( n > l)
				continue;
			for(n;n<=l;n++)
			{
				swap_buffer[b] = swap_buffer[n & 0xFFF];
				b++;
				dst[d] = swap_buffer[n & 0xFFF];
				d++;

				b &= 0xFFF;
				if(&dst[d] >= &dst[length])
				{
					return;
				}
			}
		}
	}
}

void writefile(int idx,unsigned char* data,size_t length)
{
	static char szFileName[MAX_PATH];
	char name[32];
	GetCurrentDirectory(sizeof(szFileName),szFileName);
	strcat(szFileName,"\\dat\\");
	CreateDirectory(szFileName,NULL);

	sprintf(name,"%d.dat",idx);
	strcat(szFileName,name);
	FILE* f = fopen(szFileName,"wb");
	if(f)
	{
		fwrite(data,length,1,f);
		fclose(f);
	}
}
int main()
{

	unsigned char* buf;

	unsigned char* decrypt_buf;
	unsigned int decrypt_length;

	unsigned int file_count = 0;
	FILE* f = fopen("sn.bin","rb");
	if(f)
	{
		fseek(f,0,SEEK_END);
		size_t length = ftell(f);
		buf = (unsigned char*)malloc(length);
		fseek(f,0,SEEK_SET);
		fread(buf,length,1,f);
		fclose(f);

		decrypt_length = *(unsigned int*)buf;
		decrypt_buf = (unsigned char*)malloc(decrypt_length);

		try{
			extract_buffer((buf+sizeof(unsigned int)),decrypt_buf,decrypt_length);
		}catch(...){
			printf("extract failed,crashed!!!\n");
			exit(0);
		};
		file_count = *(unsigned int*)decrypt_buf / sizeof(file_pair_t);
		file_pair_t* pair_start = (file_pair_t*)decrypt_buf;
		for(unsigned int x=0;x<file_count;x++)
		{
			writefile(x,&decrypt_buf[pair_start[x].offset],pair_start[x].length);
		}
	}
	return 0;
}
