
#ifndef _PACKAGE_H_
#define _PACKAGE_H_
VOID WINAPI SetPackageMode(BOOL vReadOrWrite);
BOOL WINAPI CreatePackage(char* Name);
unsigned char* WINAPI GetPackageFileData(ULONG Index,ULONG *Length);
VOID WINAPI SetPackageFileCount(UINT Count);
void WINAPI PutPackageFile(CHAR* FileName,char* PackageName);
ULONG WINAPI GetPackageFileCount();
unsigned char* WINAPI GetPackageFileData(ULONG Index,ULONG *Length);
char* WINAPI GetPackageFileName(ULONG Index);
BOOL WINAPI PackageFileClose();
unsigned char* WINAPI FindFileDataByName(char* Name,ULONG* Length);
unsigned char* WINAPI FindImageDataByName(ULONG CrcValue,ULONG Index,ULONG* Length);
ULONG WINAPI FindFileLengthByName(char* Name);
ULONG WINAPI FindImageLengthByName(ULONG CrcValue,ULONG Index);
#endif