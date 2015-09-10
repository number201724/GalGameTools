#pragma once

#pragma pack(1)
typedef struct file_node_s
{
	char f_name[260];
	unsigned int position;
	unsigned int compr_size;
	unsigned int source_size;
	char f_reserved[0x20];
}file_node_t;

typedef struct file_header_s
{
	char header[0x32];
	unsigned int f_count;
}file_header_t;
#pragma pack()

typedef struct file_writer_node_s
{
	char f_name[260];
	char f_diskpath[260];
	unsigned int source_size;
}file_writer_node_t;

typedef enum
{
	FS_SUCCESS,
	FS_FILE_NOTFOUND,
	FS_HEADER_FAILED,
	FS_CREATE_FAILED,
}FileSystemErrno;


int fs_open(const char* path);
int fs_create(const char* path);

void fs_close();

//return 1 have 0 not have
int fs_havefile(const char* f_name);

//read file
void fs_readfile(const char* f_name,void** data,unsigned long* f_size);

//put a files
void fs_putfile(const char* path,const char* f_name);

void fs_save();