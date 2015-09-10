// Package.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "compress.h"
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
#include "Package.h"

#pragma comment(lib,"../compress.lib")

typedef struct SPackageHeader
{
	char Magic[24];
	unsigned int FileCount;
	unsigned int Data_Start;
}TPackageHeader;

typedef struct SFileItem
{
	char FileName[64];
	ULONG NodeHash;
	ULONG NodeIndex;
	ULONG PackerLength;
	ULONG ExtractLength;
	ULONG DataOffsets;
}TFileItem;

char PackageName[MAX_PATH];
UINT PackageFileCount;
FILE* PackageFile;
class CTreeItem
{
public:
	map <ULONG,TFileItem*> subItem;
};

map <ULONG,CTreeItem*> globalIndexFindList;

map <ULONG,TFileItem*> globalHashFindList;

TPackageHeader PackageHeader;
ULONG nWriteOffsets;
ULONG ProcessIndex;
TFileItem* FileList;
BOOL ReadOrWrite=TRUE;
VOID WINAPI SetPackageMode(BOOL vReadOrWrite)
{
	ReadOrWrite = vReadOrWrite;
}
VOID MakeFindTree();
BOOL WINAPI CreatePackage(char* Name)
{
	if(ReadOrWrite == TRUE)
	{
		strcpy(PackageHeader.Magic,"脸肿汉化组");
		strcpy(PackageName,Name);
		nWriteOffsets = 0;
		PackageFile = fopen(PackageName,"wb+");
		if(!PackageFile)
			return FALSE;
	}
	else
	{
		PackageFile = fopen(Name,"rb");
		if(!PackageFile)
			return FALSE;
		fread((void*)&PackageHeader,sizeof(PackageHeader),1,PackageFile);
		if(strcmp(PackageHeader.Magic,"脸肿汉化组")!=0)
		{
			fclose(PackageFile);
			return FALSE;
		}
		FileList = new TFileItem[PackageHeader.FileCount];
		fread(FileList,sizeof(TFileItem) * PackageHeader.FileCount,1,PackageFile);
		ProcessIndex = 0;
		MakeFindTree();
	}
	return TRUE;
}

VOID MakeFindTree()
{
	map <ULONG,CTreeItem*>::iterator Iter;
	if(!FileList)
	{
		return;
	}

	for(ULONG i=0;i<PackageHeader.FileCount;i++)
	{
		if(FileList[i].NodeIndex != -1)
		{
			Iter = globalIndexFindList.find(FileList[i].NodeHash);
			if(Iter == globalIndexFindList.end())
			{
				CTreeItem* Item = new CTreeItem;
				globalIndexFindList[FileList[i].NodeHash] = Item;
				Item->subItem[FileList[i].NodeIndex] = &FileList[i];
			}
			else
			{
				Iter->second->subItem[FileList[i].NodeIndex] = &FileList[i];
			}
		}
		else
		{
			int nStrLen = strlen(FileList[i].FileName);
			ULONG crcvalue = crc((unsigned char *)&FileList[i].FileName,nStrLen);
			globalHashFindList[crcvalue] = &FileList[i];
		}
	}
}

VOID FreeFindTree()
{
	map <ULONG,CTreeItem*>::iterator Iter;


	for(Iter = globalIndexFindList.begin();Iter != globalIndexFindList.end();Iter++)
	{
		delete Iter->second;
	}
	globalIndexFindList.clear();
	globalHashFindList.clear();
}

unsigned char* WINAPI FindImageDataByName(ULONG CrcValue,ULONG Index,ULONG* Length)
{
	map <ULONG,CTreeItem*>::iterator Iter;
	map <ULONG,TFileItem*>::iterator SubNodeIter;
	Iter = globalIndexFindList.find(CrcValue);

	if(Iter != globalIndexFindList.end())
	{
		CTreeItem* SubNode = Iter->second;
		SubNodeIter = SubNode->subItem.find(Index);
		if(SubNodeIter != SubNode->subItem.end())
		{
			ULONG Index = (ULONG)((PBYTE)SubNodeIter->second - (PBYTE)FileList) / sizeof(TFileItem);
			return GetPackageFileData(Index,Length);
		}
	}

	return NULL;
}
ULONG WINAPI FindImageLengthByName(ULONG CrcValue,ULONG Index)
{
	map <ULONG,CTreeItem*>::iterator Iter;
	map <ULONG,TFileItem*>::iterator SubNodeIter;
	Iter = globalIndexFindList.find(CrcValue);

	if(Iter != globalIndexFindList.end())
	{
		CTreeItem* SubNode = Iter->second;
		SubNodeIter = SubNode->subItem.find(Index);
		if(SubNodeIter != SubNode->subItem.end())
		{
			return SubNodeIter->second->ExtractLength;
		}
	}

	return NULL;
}
unsigned char* WINAPI FindFileDataByName(char* Name,ULONG* Length)
{
	map <ULONG,TFileItem*>::iterator Iter;
	int nStrLen = strlen(Name);
	ULONG crcvalue = crc((unsigned char *)Name,nStrLen);
	Iter = globalHashFindList.find(crcvalue);
	if(Iter != globalHashFindList.end())
	{
		ULONG Index = (ULONG)((PBYTE)Iter->second - (PBYTE)FileList) / sizeof(TFileItem);
		return GetPackageFileData(Index,Length);
	}
	return NULL;
}
ULONG WINAPI FindFileLengthByName(char* Name)
{
	map <ULONG,TFileItem*>::iterator Iter;
	int nStrLen = strlen(Name);
	ULONG crcvalue = crc((unsigned char *)Name,nStrLen);
	Iter = globalHashFindList.find(crcvalue);
	if(Iter != globalHashFindList.end())
	{
		return Iter->second->ExtractLength;
	}
	return NULL;
}

VOID WINAPI SetPackageFileCount(UINT Count)
{
	PackageHeader.FileCount = Count;
	fseek(PackageFile,sizeof(TPackageHeader) + Count * sizeof(TFileItem),SEEK_SET);
	nWriteOffsets = ftell(PackageFile);
	PackageHeader.Data_Start = nWriteOffsets;
	FileList = new TFileItem[Count];
	ProcessIndex = 0;
}
ULONG get_file_index(char* name)
{
	ULONG index=-1;
	char* pstr = strrchr(name,'\\');
	if(pstr)
	{
		pstr++;
		if(strncmp(pstr,"img",3)==0)
		{
			sscanf(pstr,"img%04u,bmp",&index);
		}
	}
	return index;
}
char* get_file_name(char* name)
{

	char* pstr = strrchr(name,'\\');
	if(pstr)
	{
		pstr++;
		return pstr;
	}
	return NULL;
}

void WINAPI PutPackageFile(CHAR* FileName,char* PackageName)
{
	size_t file_length;
	size_t compress_length;

	unsigned char* compress_data;
	unsigned char* file_data;
	if(ProcessIndex >= PackageHeader.FileCount)
	{
		return;
	}
	FileList[ProcessIndex].ExtractLength = length_of_file(FileName);
	file_data = get_file_data(FileName,&file_length);

	FileList[ProcessIndex].NodeIndex = get_file_index(FileName);

	char newFileName[64];
	if(strcmp(PackageName,get_file_name(FileName))!=0)
	{
		sprintf(newFileName,"%s/%s",PackageName,get_file_name(FileName));
	}
	else
	{
		strcpy(newFileName,PackageName);
	}
	
	strcpy(FileList[ProcessIndex].FileName,newFileName);

	compress_data = compress(file_data,file_length,&compress_length);

	free_buffer(file_data);

	FileList[ProcessIndex].PackerLength = compress_length;
	FileList[ProcessIndex].DataOffsets = nWriteOffsets;

	if(FileList[ProcessIndex].NodeIndex != -1)
		FileList[ProcessIndex].NodeHash = crc((unsigned char*)PackageName,strlen(PackageName));
	else
		FileList[ProcessIndex].NodeHash = crc((unsigned char*)newFileName,strlen(newFileName));

	fwrite(compress_data,compress_length,1,PackageFile);

	nWriteOffsets = ftell(PackageFile);
	free_buffer(compress_data);
	ProcessIndex++;
}
ULONG WINAPI GetPackageFileCount()
{
	return PackageHeader.FileCount;
}
unsigned char* WINAPI GetPackageFileData(ULONG Index,ULONG *Length)
{
	unsigned char* PackerBuffer;
	unsigned char* decompress_data;
	UINT decompress_length;
	if(Index >= PackageHeader.FileCount)
		return NULL;

	fseek(PackageFile,FileList[Index].DataOffsets,SEEK_SET);
	PackerBuffer = new unsigned char[FileList[Index].PackerLength];
	
	fread(PackerBuffer,FileList[Index].PackerLength,1,PackageFile);

	decompress_data = decompress(PackerBuffer,FileList[Index].PackerLength,&decompress_length);
	
	delete PackerBuffer;
	*Length = decompress_length;
	return decompress_data;
}

char* WINAPI GetPackageFileName(ULONG Index)
{
	return FileList[Index].FileName;
}
BOOL WINAPI PackageFileClose()
{
	if(ReadOrWrite == TRUE)
	{
		fseek(PackageFile,0,SEEK_SET);
		fwrite(&PackageHeader,sizeof(TPackageHeader),1,PackageFile);
		fwrite(FileList,sizeof(TFileItem) * PackageHeader.FileCount,1,PackageFile);
		fflush(PackageFile);
		fclose(PackageFile);
		delete FileList;
	}
	else
	{
		FreeFindTree();
		delete FileList;
		fclose(PackageFile);
	}
	return TRUE;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{

    return TRUE;
}

