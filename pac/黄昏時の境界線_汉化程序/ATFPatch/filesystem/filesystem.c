#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"
#include "zlib.h"



FILE* fd;
file_header_t f_header;
file_node_t *f_node;
file_writer_node_t* f_wrtnode;

int fs_open(const char* path){
	int node_size;

	fd = fopen(path,"rb");

	if(!fd){
		return FS_FILE_NOTFOUND;
	}

	if(fread(&f_header,sizeof(f_header),1,fd) != 1){
		fclose(fd);
		fd = NULL;
		return FS_HEADER_FAILED;
	}

	if(strcmp(f_header.header,"ATField FileSystem")!=0){
		fclose(fd);
		fd = NULL;
		return FS_HEADER_FAILED;
	}

	node_size = sizeof(file_node_t) * f_header.f_count;
	f_node = (file_node_t *)malloc(node_size);

	if(fread(f_node,1,node_size,fd) != node_size){
		fclose(fd);
		fd = NULL;
		return FS_HEADER_FAILED;
	}

	return FS_SUCCESS;
}

int fs_create(const char* path){
	fd = fopen(path,"wb+");
	if(!fd){
		return FS_CREATE_FAILED;
	}
	strcpy(f_header.header,"ATField FileSystem");
	f_header.f_count = 0;
	f_wrtnode = (file_writer_node_t *)malloc(0);

	return FS_SUCCESS;
}

unsigned long fs_disk_flength(const char* path){
	FILE* nfd;
	unsigned long f_length;


	nfd = fopen(path,"rb");
	if(!nfd){
		return 0;
	}

	fseek(nfd,0,SEEK_END);
	f_length = ftell(nfd);
	fclose(nfd);

	return f_length;
}
void fs_putfile(const char* path,const char* f_name){
	unsigned long f_length;
	

	if(f_wrtnode){
		f_length = fs_disk_flength(path);
		if(f_length>0){
			f_wrtnode = (file_writer_node_t *)realloc(f_wrtnode,sizeof(file_writer_node_t) * (f_header.f_count + 1));

			strcpy(f_wrtnode[f_header.f_count].f_diskpath,path);
			strcpy(f_wrtnode[f_header.f_count].f_name,f_name);
			f_wrtnode[f_header.f_count].source_size = f_length;

			f_header.f_count++;
		}
	}
}
void fs_compr_readdisk(const char* path,void** f_comprbuf,unsigned long* f_comprlength){
	FILE* nfd;
	unsigned char* compr_buf;
	unsigned char* source_buf;
	unsigned long source_length;
	unsigned long compr_length;

	nfd = fopen(path,"rb");
	if(nfd){
		fseek(nfd,0,SEEK_END);
		source_length = ftell(nfd);
		fseek(nfd,0,SEEK_SET);

		source_buf = (unsigned char*)malloc(source_length);
		compr_length = compressBound(source_length);
		compr_buf = (unsigned char*)malloc(compr_length);

		fread(source_buf,source_length,1,nfd);


		if(compress(compr_buf,&compr_length,source_buf,source_length) == Z_OK){
			*f_comprbuf = compr_buf;
			*f_comprlength = compr_length;

			free(source_buf);
		}else{
			free(compr_buf);
			free(source_buf);
		}

		fclose(nfd);
	}
}
void fs_save(){
	unsigned long f_startpos;
	int i;
	file_node_t* f_builder;
	void* f_comprbuf = NULL;
	unsigned long f_comprlength;

	if(f_wrtnode){
		f_builder = (file_node_t*)malloc(sizeof(file_node_t) * f_header.f_count);
		memset(f_builder,0,sizeof(file_node_t) * f_header.f_count);

		f_startpos = sizeof(file_header_t) + (sizeof(file_node_t) * f_header.f_count);
		fseek(fd,0,SEEK_SET);

		fwrite(&f_header,sizeof(f_header),1,fd);

		for(i = 0;i<f_header.f_count;i++){
			fs_compr_readdisk(f_wrtnode[i].f_diskpath,&f_comprbuf,&f_comprlength);

			if(f_comprbuf && f_comprlength){
				fseek(fd,f_startpos,SEEK_SET);

				strcpy(f_builder[i].f_name,f_wrtnode[i].f_name);
				f_builder[i].position = f_startpos;
				f_builder[i].source_size = f_wrtnode[i].source_size;
				f_builder[i].compr_size = f_comprlength;
				
				fwrite(f_comprbuf,f_comprlength,1,fd);
				f_startpos += f_comprlength;
			}

			if(f_comprbuf){
				free(f_comprbuf);
				f_comprbuf = NULL;
			}
		}


		fseek(fd,sizeof(file_header_t),SEEK_SET);
		fwrite(f_builder,(sizeof(file_node_t) * f_header.f_count),1,fd);
		fflush(fd);
	}
}
void fs_close(){
	if(fd){
		fclose(fd);
		fd = NULL;
	}

	if(f_node){
		free(f_node);
		f_node = NULL;
	}

	if(f_wrtnode){
		free(f_wrtnode);
		f_wrtnode = NULL;
	}
}

int fs_havefile(const char* f_name){
	unsigned int i;
	if(!f_node) return 0;

	for(i=0;i<f_header.f_count;i++){
		if(_stricmp(f_node[i].f_name,f_name) == 0){
			return 1;
		}
	}
	return 0;
}

unsigned int fs_filesize(const char* f_name){
	unsigned int i;
	if(!f_node) return 0;

	for(i=0;i<f_header.f_count;i++){
		if(_stricmp(f_node[i].f_name,f_name) == 0){
			return f_node[i].source_size;
		}
	}
	return 0;
}

void fs_readbuffer(file_node_t* node,void** data,unsigned long* f_size){
	unsigned char* compr_buf;
	unsigned char* uncompr_buf;
	unsigned long uncompr_length;

	fseek(fd,node->position,SEEK_SET);

	compr_buf = (unsigned char*)malloc(node->compr_size);
	uncompr_buf = (unsigned char*)malloc(node->source_size);
	uncompr_length = node->source_size;
	fread(compr_buf,node->compr_size,1,fd);

	if(uncompress(uncompr_buf,&uncompr_length,compr_buf,node->compr_size) == Z_OK){
		*data = uncompr_buf;
		*f_size = uncompr_length;

		free(compr_buf);
	}
	else{
		free(compr_buf);
		free(uncompr_buf);

		*data = NULL;
		*f_size = 0;
	}
}

void fs_readfile(const char* f_name,void** data,unsigned long* f_size){
	unsigned int i;
	if(!f_node) return ;

	for(i=0;i<f_header.f_count;i++){
		if(_stricmp(f_node[i].f_name,f_name) == 0){
			fs_readbuffer(&f_node[i],data,f_size);
			return;
		}
	}
}