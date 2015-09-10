/*
MajiroArc V3.0
code by 201724
QQ:527806988
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
unsigned int get_file_size(FILE* fp)
{
	unsigned int size;
	if(fp==NULL)
	{        
			return 0;
	}    
	fseek( fp, 0L, SEEK_END );
	size=ftell(fp);
	return size;
}
void format_file_name(char* name,int len)
{
	static char newname[256];
	int i;
	int x=0;
	for(i=0;i<len;i++)
	{
		if(name[i] == '?')
		{
			newname[x] = 0xA1;
			x++;
			newname[x] = 0xA4;
			x++;
		}
		else
		{
			newname[x] = name[i];
			x++;
		}
	}
	newname[x] = 0;
	strcpy(name,newname);
}

void write_to_file(char* path,char* name,void* data,size_t size)
{
	char szNewName[MAX_PATH];
	FILE* f;
	sprintf(szNewName,"%s%s",path,name);
	f = fopen(szNewName,"wb+");
	if(f)
	{
		fwrite(data,size,1,f);
		fclose(f);
	}
}
int main(int argc, char *argv[], char *envp[ ])
{
	unsigned char* file_data;
	unsigned int file_size;

	arc_idx_t* idx_list;
	
	arc_header_t file_header;
	int idx_list_size;

	int file_tell;
	int i,x;
	int len;
	void* buf;
	char newfilename[128];
	char origfilename[128];
	
	char szDirPath[MAX_PATH];
	
	__int64 crc_value;
	
	FILE* f;

	printf("arc v3.0 tool by 201724\n");
	if(!(argc > 1))
	{
		printf("input file name invalid!!!\n");
		return 0;
	}
	
	GetCurrentDirectory(sizeof(szDirPath),szDirPath);
	strcat(szDirPath,"\\output\\");
	CreateDirectory(szDirPath,NULL);
	f = fopen(argv[1],"rb");
	if(f)
	{
		file_size = get_file_size(f);
		if(file_size)
		{
				fseek(f,0,SEEK_SET);
				if(fread((void*)&file_header,sizeof(file_header),1,f))
				{
					if(strcmp(file_header.sign,"MajiroArcV3.000"))
					{
						printf("unknown input file\n");
						fclose(f);
						return -1;
					}
					idx_list_size = file_header.filecount * sizeof(arc_idx_t);
					idx_list = (arc_idx_t*)malloc(idx_list_size);
					if(!fread((void*)idx_list,idx_list_size,1,f))
					{
						printf("read file index failed\n");
						fclose(f);
						return -1;
					}
					fseek(f,file_header.stroff,SEEK_SET);
					for(i=0;i<file_header.filecount;i++)
					{
						file_tell = ftell(f);
						
						fread((void*)&origfilename,sizeof(origfilename),1,f);
						
						file_tell += strlen(origfilename) + 1;
						
						crc_value = CRC64_String(origfilename);
						
						len = CovtShiftJISToGB(origfilename,newfilename,sizeof(newfilename));
						
						for(x=0;x<file_header.filecount;x++)
						{
							if(idx_list[x].crc64 == crc_value)
							{
								format_file_name(newfilename,len);
								printf("filename:%s offset:%X size:%X\n",newfilename,idx_list[x].dataoffs,idx_list[x].size);
								fseek(f,idx_list[x].dataoffs,SEEK_SET);
								buf = malloc(idx_list[x].size);
								fread(buf,idx_list[x].size,1,f);
								write_to_file(szDirPath,newfilename,buf,idx_list[x].size);
								free(buf);
								break;
							}
						}
						fseek(f,file_tell,SEEK_SET);
					}
				}
				fclose(f);
		}
	}
}