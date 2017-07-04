
#ifndef _PACKAGE_H_
#define _PACKAGE_H_

BOOL WINAPI CreatePackage(char* Name);
unsigned char* WINAPI GetPackageFileData(ULONG Index,ULONG *Length);
VOID WINAPI SetPackageFileCount(UINT Count);
void WINAPI PutPackageFile(CHAR* FileName,char* PackageName);
ULONG WINAPI GetPackageFileCount();
unsigned char* WINAPI GetPackageFileData(ULONG Index,ULONG *Length);
char* WINAPI GetPackageFileName(ULONG Index);
BOOL WINAPI PackageFileClose();


#endif