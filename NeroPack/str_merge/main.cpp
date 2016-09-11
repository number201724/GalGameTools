#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <Windows.h>


using namespace std;
struct game_texts
{
	int index;
	std::string src;
	std::string dst;
};

char* skip_str(char *s)
{
	while (*s == '\t' || *s == ' ' || *s == '\r' || *s == '\n') {
		s++;
	}

	return s;
}

std::string currentFile;

void process_str(char *s)
{
	char *n1 = strstr(s, ">");
	if (n1) {
		n1++;
		strcpy(s, n1);
	}

	char* n2 = strstr(s, "[");
	if (n2) {
		char* n3 = strstr(s, "]");
		if (n3) {
			n3++;
			strcpy(s, n3);
		}
	}

	if (strstr(s, "["))
	{
		printf("´íÎóµÄ·­Òë£º%s ÔÚ£º%s\n", currentFile.c_str(), s);
		getchar();
		exit(0);
	}
	if (strstr(s, "]"))
	{
		printf("´íÎóµÄ·­Òë£º%s ÔÚ£º%s\n", currentFile.c_str(), s);
		getchar();
		exit(0);
	}
}
void process_text(std::string texts)
{
	currentFile = texts;
	std::vector<std::string> loadtext;
	char s[512];
	std::string str_texts = "texts/" + texts;
	std::string str_script = "script/" + texts;
	std::string str_output = "output/" + texts;

	FILE *ftexts = fopen(str_texts.c_str(), "rb");
	FILE *fscript = fopen(str_script.c_str(), "rb");
	FILE *foutput = fopen(str_output.c_str(), "wb");

	while (fgets(s, sizeof(s), ftexts))
	{
		if (s[0] == '*')
		{
			//char *x = strstr(s, ">");
			process_str(s);
			loadtext.push_back(s);
			//if (x) {
			//	x++;
				
				
			//	loadtext.push_back(x);
				cout << s << endl;
			//}
		}
	}
	int index = 0;
	bool isSelect = false;

	while (fgets(s, sizeof(s), fscript))
	{
		char *x = skip_str(s);

		if (!strncmp(x, ".select",7)) {
			isSelect = true;
		}

		if (!strncmp(x, ".end",7)) {
			isSelect = false;
		}
		
		if (!isSelect && *x && x[0] != '@' && x[0] != '.' && x[0] != ':' && x[0] != ';')
		{
			
			fwrite(loadtext[index].c_str(), loadtext[index].length(), 1, foutput);
			index++;
			//fflush(foutput);
		}
		else
		{
			fwrite(s, strlen(s), 1, foutput);
			//fflush(foutput);
		}
	}

	fclose(ftexts);
	fclose(fscript);
	fclose(foutput);
}

void load_update_files(std::string dir)
{
	HANDLE hFind;
	std::string sfind = dir + "/*.txt";
	WIN32_FIND_DATAA ffd;

	hFind = FindFirstFileA(sfind.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		cout << "invalid find" << endl;
		exit(0);
	}

	do
	{
		process_text(ffd.cFileName);

		printf("prepare:%s\n", ffd.cFileName);
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);
}


int main(int argc, char *argv[])
{
	load_update_files("texts");

	//process_text("021003.txt");
	return 0;
}