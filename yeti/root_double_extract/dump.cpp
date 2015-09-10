#pragma warning(disable:4786)
#pragma warning(disable:2679)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <windows.h>

#include <direct.h>
#include <vector>
#include <map>
using namespace std;
typedef struct text_data_s
{
	unsigned int offsets;
	wchar_t* texts;
}text_data_t;
vector <text_data_t*> g_TextData;

int CovtShiftJISToGB(const char* JISStr, char* GBBuf, int nSize) 
{ 
	static wchar_t wbuf[2048];
	int nLen; 

	nLen = strlen(JISStr)+1; 
	if (wbuf==NULL) return false; 
	nLen = MultiByteToWideChar(932, 0, JISStr, 
		nLen, wbuf, nLen); 
	if (nLen > 0) 
		nLen = WideCharToMultiByte(936, 0, wbuf, 
		nLen, GBBuf, nSize, NULL, 0); 
	return nLen; 
}
int CovtGbkToShiftJIS(const char* JISStr, char* GBBuf, int nSize) 
{ 
	static wchar_t wbuf[2048];
	int nLen; 

	nLen = strlen(JISStr)+1; 
	if (wbuf==NULL) return false; 
	nLen = MultiByteToWideChar(936, 0, JISStr, 
		nLen, wbuf, nLen); 
	if (nLen > 0) 
		nLen = WideCharToMultiByte(932, 0, wbuf, 
		nLen, GBBuf, nSize, NULL, 0); 
	return nLen; 
}
char* GetGBKChars(char* instr)
{
	static char szText[1024];
	CovtShiftJISToGB(instr,szText,sizeof(szText));
	return szText;
}
wchar_t *GetUnicodeChars(char* instr)
{
	static wchar_t wbuf[2048];
	memset(&wbuf,0,sizeof(wbuf));
	int nLen; 

	nLen = strlen(instr)+1; 
	if (wbuf==NULL) return false; 
	nLen = MultiByteToWideChar(932, 0, instr, 
		nLen, wbuf, nLen); 

	return (wchar_t*)&wbuf;
}
int main(int argc,char** argv)
{
	FILE* f;
	size_t length;
	size_t count;
	unsigned char* buf;
	char* file_name;
	char txtfile_name[128];

	if(argc > 1)
	{
		file_name = argv[1];
	}
	else
	{
		return 0;
	}
	f = fopen(file_name,"rb");
	fseek(f,0,SEEK_END);
	length = ftell(f);
	fseek(f,0,SEEK_SET);
	buf = new unsigned char[length];
	fread(buf,length,1,f);
	fclose(f);

	for(size_t x=0;x<length;x++)
	{
		unsigned char* p = &buf[x];
		if(p[0] == 0x45 && p[1] == 0xFF && p[2] == 0xFF)	//
		{
			text_data_t* data = new text_data_t;
			char* pstr = (char*) &p [5];
			data->offsets = (unsigned int)((p + 5) - buf);
			wchar_t* new_str = GetUnicodeChars(pstr);
			int slen = wcslen(new_str);
			data->texts = new wchar_t[slen+1];
			wcscpy(data->texts,new_str);
			g_TextData.push_back(data);
		}
		if(p[0] == 0x47 && p[1] == 0x0D && p[2] == 0x00)
		{
			text_data_t* data = new text_data_t;
			char* pstr = (char*) &p [3];
			data->offsets = (unsigned int)((p + 3) - buf);
			wchar_t* new_str = GetUnicodeChars(pstr);
			int slen = wcslen(new_str);
			data->texts = new wchar_t[slen+1];
			wcscpy(data->texts,new_str);
			g_TextData.push_back(data);
		}
	}

	sprintf(txtfile_name,"%s.txt",file_name);

	FILE* txt = fopen(txtfile_name,"wb");
	fwrite("\xFF\xFE",2,1,txt);
	
	for(size_t n=0;n<g_TextData.size();n++)
	{
		fwprintf(txt, L"<%d>",g_TextData[n]->offsets);
		fwprintf(txt, L"\r\n//");
		fwprintf(txt, L"%ls", g_TextData[n]->texts);
		fwprintf(txt, L"\r\n");
		fwprintf(txt, L"%ls", g_TextData[n]->texts);
		fwprintf(txt, L"\r\n");
	}
	fclose(f);
	

	return 0;
}