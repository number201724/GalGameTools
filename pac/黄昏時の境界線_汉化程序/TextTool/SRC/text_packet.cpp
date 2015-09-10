#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <vector>
using namespace std;

#define HEAD_STRING "$TEXT_LIST__"
typedef unsigned char byte;
typedef unsigned int uint;

vector <char*> text_list;
int main()
{
	char gets_text[4096];
	byte * data;
	size_t str_count;
	
	
	FILE* f;
	FILE* txt_f;
	
	remove("ATField.DAT");
	
	f = fopen("ATField.DAT","wb+");
	
	if(!f)
	{
		printf("open file error\n");
		return -1;
	}
	
	txt_f = fopen("sens.txt","r");
	
	while(fgets(gets_text,sizeof(gets_text),txt_f))
	{
		int len = strlen(gets_text);
		gets_text[len-1] = 0;
		char* push_text = new char[len+1];
	
		strcpy(push_text,gets_text);
		text_list.push_back(push_text);
	}
	
	str_count = text_list.size();
	fwrite(HEAD_STRING,sizeof(HEAD_STRING)-1,1,f);
	fwrite((char*)&str_count,4,1,f);
	for(size_t i=0;i<text_list.size();i++)
	{
		fwrite(&i,4,1,f);
		fwrite(text_list[i],strlen(text_list[i])+1,1,f);
	}
	
	fclose(f);
	return 1;
}