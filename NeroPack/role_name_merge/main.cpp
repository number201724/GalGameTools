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

char* ptr_trim(char *s)
{
	while (*s == '\t' || *s == ' ' || *s == '\r' || *s == '\n') {
		s++;
	}

	return s;
}

void remove_crlf(wchar_t *str)
{
	wchar_t *p = str;

	while (*p)
	{
		if (*p == L'\r')
		{
			wcscpy(p, p + 0x1);
			continue;
		}
		if (*p == L'\n')
		{
			wcscpy(p, p + 0x1);
			continue;
		}

		p++;
	}
	
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

struct RoleNameInfo
{
	std::wstring jp_name;
	std::wstring cn_name;
};


std::vector<RoleNameInfo> g_roles;
int get_role_name_index(std::wstring name)
{
	for (size_t i = 0; i < g_roles.size(); i++)
	{
		if (g_roles[i].jp_name.compare(name) == 0)
		{
			return i;
		}
	}

	return -1;
}


void insert_to_script_name_label(char* line, char* name)
{
	char temp[512];
	char* start = &line[1];
	char *end = strstr(start, ":");
	if (!end)
	{
		fprintf(stderr, "wtf\n");
		return;
	}


	strcpy(temp, ":");
	strcat(temp, name);
	strcat(temp, end);

	strcpy(line, temp);
}


void process_text(std::string texts)
{
	char line[512];
	wchar_t unicode_str[256];

	FILE *temp = fopen("temp.txt", "wb");
	FILE* fscript = fopen(texts.c_str(), "rb");


	if (!fscript || !temp)
	{
		printf("open fail\n");
		getchar();
		exit(1);
	}


	while (fgets(line, sizeof(line), fscript))
	{
		char*s = ptr_trim(line);

		if (s[0] == ':')
		{
			memset(unicode_str, 0, sizeof(unicode_str));
			MultiByteToWideChar(932, 0, (LPCCH)&line, strlen(line), unicode_str, sizeof(unicode_str));

			wchar_t *next = wcsstr(&unicode_str[1], L":");
			*next = L'\0';
			wcscpy(unicode_str, &unicode_str[1]);
			std::wstring newStr = unicode_str;

			int index = get_role_name_index(newStr);
			if (index != -1)
			{
				char cn_name[256] = { 0 };
				g_roles[index].cn_name.c_str();

				WideCharToMultiByte(936, 0, g_roles[index].cn_name.c_str(), g_roles[index].cn_name.length(), cn_name, sizeof(cn_name), NULL, NULL);

				insert_to_script_name_label(line, cn_name);

			}
			else
			{
				printf("unkname found\n");
				getchar();
				exit(1);
			}


		}

		fwrite(line, strlen(line), 1, temp);
	}


	fflush(temp);
	fclose(temp);
	fclose(fscript);

	remove(texts.c_str());

	MoveFileA("temp.txt", texts.c_str());
}

void update_scripts(std::string dir)
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
		printf("process:%s\n", ffd.cFileName);
		process_text(new_file);

		
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);
}


void load_script()
{
	wchar_t line[256];

	FILE *fp = _wfopen(L"name.txt", L"rb");
	if (!fp) {
		fprintf(stderr, "open name.txt fail\n");
		exit(1);
	}
	fseek(fp, 2, SEEK_SET); //ignore unicode magic

	bool in_cast = false;
	RoleNameInfo NameInfo;

	while (fgetws(line, ARRAYSIZE(line), fp))
	{
		remove_crlf(line);

		if (!wcsncmp(line, L"[JP]", 4))
		{
			NameInfo.jp_name = &line[4];
			in_cast = true;
		}

		if (in_cast && !wcsncmp(line, L"[CN]", 4))
		{
			NameInfo.cn_name = &line[4];
			in_cast = false;

			g_roles.push_back(NameInfo);
		}
	}
}
int main(int argc, char *argv[])
{
	load_script();

	//setlocale(0, "Japanese");
	update_scripts("C:\\data\\games\\Endless Dungeon\\str_merge\\output");

	//process_text("021003.txt");
	return 0;
}