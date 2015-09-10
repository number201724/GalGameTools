#pragma once
#include <vector>
using namespace std;

typedef struct text_list_s
{
	//原始文本数据
	char* pOrigText;
	//修改后的文本数据
	char* pNewText;
}text_list_t;
typedef struct offset_string_s
{
	text_list_t Text;
	//索引文件指向textdata的偏移数据的偏移
	DWORD nToOffset;
	DWORD indexoffset;
}offset_string_t;
typedef struct offset_title_s
{
	text_list_t Text;
	DWORD nToOffset;
	vector <DWORD> OffsetList;
}offset_title_t;

#define sig_title 1
#define sig_text 2
typedef struct text_edit_s
{
	int nType;
	text_list_t* Data;
}text_edit_t;
class CDataStream
{
private:
	
	PBYTE pOrigIndex;
	PBYTE pOrigStringData;
public:
	//保存汉化信息的列表
	vector <offset_string_t*> List;
	vector <offset_title_s*> TitleList;
	vector <text_edit_t*> EditList;

	CDataStream(void);
	virtual ~CDataStream(void);
	//pszFileName 索引文件路径
	bool ReadStreamForData(char* pszFileName);
	//分析索引文件数据
	bool AnalyaisData(PBYTE IndexData,DWORD IndexSize,PBYTE StringData,DWORD StringSize);
	

	int CovtShiftJISToGB(const char* JISStr, char* GBBuf, int nSize);
	int CovtGbkToShiftJIS(const char* JISStr, char* GBBuf, int nSize);

	void InsertAndConvData(PBYTE StringData,PBYTE IndexData,DWORD offset);
	void InsertAndConvTitle(PBYTE StringData,PBYTE IndexData,DWORD offset);

	void EncryptAndDecrypt(BYTE *Data,size_t Size);
	bool PacketToFile(char* pszIndexFileName);

	void FormatToEditList();
};
