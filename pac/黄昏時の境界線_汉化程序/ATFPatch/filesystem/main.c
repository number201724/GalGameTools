#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "filesystem.h"
#include "zlib.h"


void add_files()
{
	HANDLE hFind;
	WIN32_FIND_DATAA findData;
	char szDirectory[MAX_PATH];
	char szTmpFileName[MAX_PATH];
	char szFindDir[MAX_PATH];

	GetCurrentDirectoryA(sizeof(szDirectory),szDirectory);
	strcpy(szFindDir,szDirectory);
	strcat(szFindDir,"\\bmp\\*.*");
	hFind = FindFirstFileA(szFindDir,&findData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		return;
	}

	do
	{
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			strcpy(szTmpFileName,szDirectory);
			strcat(szTmpFileName,"\\bmp\\");
			strcat(szTmpFileName,findData.cFileName);

			fs_putfile(szTmpFileName,findData.cFileName);

			//printf("packet file:%s\n",findData.cFileName);
		}
	}while(FindNextFileA(hFind,&findData));

	fs_putfile("TEXT.DAT","TEXT.DAT");
}



int main(int argc,char** argv)
{
	void* f_buf;
	unsigned long f_length;



	if(fs_create("ATField.dat")){
		printf("file open failed\n");
		return 1;
	}

	add_files();

	fs_save();

	fs_close();

	//if(fs_open("ATField.dat")){
	//	printf("file open failed\n");
	//	return 1;
	//}

	//fs_readfile("TEXT.DAT",&f_buf,&f_length);



	return 0;
}