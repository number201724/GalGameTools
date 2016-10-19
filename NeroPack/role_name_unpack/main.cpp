#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <set>
#include <vector>

using namespace std;
struct game_texts
{
	int index;
	std::string src;
	std::string dst;
};

char* trim(char *s)
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
		printf("错误的翻译：%s 在：%s\n", currentFile.c_str(), s);
		getchar();
		exit(0);
	}
	if (strstr(s, "]"))
	{
		printf("错误的翻译：%s 在：%s\n", currentFile.c_str(), s);
		getchar();
		exit(0);
	}

	if (strstr(s, "."))
	{
		printf("错误的翻译：%s 在：%s\n", currentFile.c_str(), s);
		getchar();
		exit(0);
	}

	if (strstr(s, ":"))
	{
		printf("错误的翻译：%s 在：%s\n", currentFile.c_str(), s);
		getchar();
		exit(0);
	}
}

std::vector<std::wstring> g_roles;

bool has_role_name(std::wstring name)
{
	for (size_t i = 0; i < g_roles.size(); i++)
	{
		if (g_roles[i].compare(name) == 0) return true;
	}
	return false;
}

void process_text(std::string texts)
{
	char line[256];
	wchar_t unicode_str[256];
	
	FILE *fp = fopen(texts.c_str(), "rb");

	if (!fp) {
		fprintf(stderr, "open file %s failed\n", texts.c_str());
		return;
	}


	

	while (fgets(line, sizeof(line), fp))
	{
		char* p = trim(line);

		if (p[0] == ':')
		{
			//fprintf(stderr, "%s\n", line);

			memset(unicode_str, 0, sizeof(unicode_str));
			MultiByteToWideChar(932, 0, (LPCCH)&line, strlen(line), unicode_str, sizeof(unicode_str));


			if (unicode_str[0] == L':')
			{
				wchar_t *next = wcsstr(&unicode_str[1], L":");
				*next = L'\0';


				wcscpy(unicode_str, &unicode_str[1]);



				std::wstring newStr = unicode_str;

				if (!has_role_name(newStr))
				{
					g_roles.push_back(newStr);
				}

			}
		}
	}



	fclose(fp);
}

void load_update_files(std::string dir)
{
	HANDLE hFind;
	std::string sfind = dir + "\\*.txt";
	WIN32_FIND_DATAA ffd;

	hFind = FindFirstFileA(sfind.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		cout << "invalid find" << endl;
		exit(0);
	}

	do
	{
		std::string new_file = dir + "\\" + ffd.cFileName;
		process_text(new_file);

		printf("prepare:%s\n", ffd.cFileName);
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);
}


void save_to_script()
{
	unsigned char c[2] = { 0xff,0xfe };
	FILE *namefile = _wfopen(L"name.txt", L"wb");
	if (namefile)
	{
		fwrite(c, sizeof(c), 1, namefile);

		fwprintf(namefile, L"\r\n");
		//fwprintf(namefile, L"\xFF\xFE");
		for (size_t i = 0; i < g_roles.size(); i++)
		{
			fwprintf(namefile, L"[JP]%ls\r\n", g_roles[i].c_str());
			fwprintf(namefile, L"[CN]%ls\r\n", g_roles[i].c_str());
			fwprintf(namefile, L"\r\n");
		}
		fclose(namefile);
	}
}

int main(int argc, char *argv[])
{
	//setlocale(0, "Japanese");
	load_update_files("C:\\data\\games\\Endless Dungeon\\str_merge\\script");
	save_to_script();


	//process_text("021003.txt");
	return 0;
}