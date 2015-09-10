#include "StdAfx.h"
#include "DataStream.h"

CDataStream::CDataStream(void)
{
	pOrigIndex = 0;
	pOrigStringData = 0;
}

CDataStream::~CDataStream(void)
{

}
int CDataStream::CovtShiftJISToGB(const char* JISStr, char* GBBuf, int nSize) 
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
int CDataStream::CovtGbkToShiftJIS(const char* JISStr, char* GBBuf, int nSize) 
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

void CDataStream::InsertAndConvData(PBYTE StringData,PBYTE IndexData,DWORD offset)
{
	static char szConvText[1024];
	offset_string_t * pOffsetString;
	char* pszOrgString_NoConv;
	int newlen;
	PDWORD offsetData =(PDWORD) &IndexData[offset];

	pszOrgString_NoConv = (char*)&StringData[*offsetData];
	if(pszOrgString_NoConv[-1] == 0)
	{
		newlen = CovtShiftJISToGB(pszOrgString_NoConv,szConvText,sizeof(szConvText));
		if(newlen)
		{
			pOffsetString = new offset_string_t;
			pOffsetString->indexoffset = offset;
			//pOffsetString->nToOffset = *offsetData;
			pOffsetString->Text.pOrigText = new char[newlen+1];
			strcpy(pOffsetString->Text.pOrigText,szConvText);
			pOffsetString->Text.pNewText = NULL;

			List.push_back(pOffsetString);
		}
	}
}
void CDataStream::InsertAndConvTitle(PBYTE StringData,PBYTE IndexData,DWORD offset)
{
	static char szConvText[1024];
	char* pszOrgString_NoConv;
	DWORD strOffset = *(DWORD*)(IndexData + offset);
	DWORD ListSize = TitleList.size();
	for(DWORD i=0;i<ListSize;i++)
	{
		if(TitleList[i]->nToOffset == strOffset)
		{
			TitleList[i]->OffsetList.push_back(offset);
			return;
		}
	}
	pszOrgString_NoConv = (char*)&StringData[strOffset];
	if(pszOrgString_NoConv[-1] == 0)
	{
		int newlen = CovtShiftJISToGB(pszOrgString_NoConv,szConvText,sizeof(szConvText));
		if(newlen)
		{
			offset_title_t* title_data = new offset_title_t;
			title_data->nToOffset = strOffset;
			title_data->OffsetList.push_back(offset);
			title_data->Text.pOrigText = new char[newlen+1];
			strcpy(title_data->Text.pOrigText,szConvText);
			title_data->Text.pNewText = NULL;
			TitleList.push_back(title_data);
		}
	}
}
void CDataStream::FormatToEditList()
{
	text_edit_t* m_Data;
	int listsize = TitleList.size();
	for(int i=0;i<listsize;i++)
	{
		m_Data = new text_edit_t;
		m_Data->nType = sig_title;
		m_Data->Data = &TitleList[i]->Text;
		EditList.push_back(m_Data);
	}
	listsize = List.size();

	for(int i=0;i<listsize;i++)
	{
		m_Data = new text_edit_t;
		m_Data->nType = sig_text;
		m_Data->Data = &List[i]->Text;
		EditList.push_back(m_Data);
	}				
}
bool CDataStream::AnalyaisData(PBYTE IndexData,DWORD IndexSize,PBYTE StringData,DWORD StringSize)
{
	bool retstatus = false;
	//PDWORD IndexListStart = (PDWORD)IndexData;
	//DWORD IndexCount = IndexSize / 4; //4字节对齐


	try
	{
		for(DWORD i = 0;i<IndexSize;i++)
		{
			DWORD CodeData = *(DWORD*)&IndexData[i];
			PDWORD IndexListStart = (PDWORD)&IndexData[i];
			//analysis opcode index
			switch(CodeData)
			{
			case 0x06010104:
				{
					//i++; //pass unknown value
					if(IndexListStart[1])
					{
						InsertAndConvTitle(StringData,IndexData, (DWORD)&IndexListStart[1] - (DWORD)IndexData);
					}
					//i++; //pass unknown value
					//i++; //pass unknown value
					if(IndexListStart[3])
					{
						InsertAndConvData(StringData,IndexData, (DWORD)&IndexListStart[3] - (DWORD)IndexData);
					}
					break;
				}
			case 0x80000307:
				{
					//i++; //pass unknown value
					//i++; //redirect to text names
					//解析说话人的名字
					if(IndexListStart[2])
					{
						InsertAndConvTitle(StringData,IndexData, (DWORD)&IndexListStart[2] - (DWORD)IndexData);
					}
					//解析文本内容
					//i++;
					if(IndexListStart[3])
					{
						InsertAndConvData(StringData,IndexData, (DWORD)&IndexListStart[3] - (DWORD)IndexData);
					}
					break;
				}
			case 0x80000406:
				{
					//i += 3;
					//解析文本内容
					if(IndexListStart[3])
					{
						InsertAndConvData(StringData,IndexData, (DWORD)&IndexListStart[3] - (DWORD)IndexData);
					}
					break;
				}
			case 0x01002803:
			case 0x03000303:
				{
					//i += 2; //定位到string
					if(IndexListStart[2])
					{
						InsertAndConvData(StringData,IndexData, (DWORD)&IndexListStart[2] - (DWORD)IndexData);
					}
					break;
				}
			}
		}
	}catch(...){};


	FormatToEditList();

	return retstatus;
}
bool CDataStream::ReadStreamForData(char* pszFileName)
{
	static char* IndexFileSign = "PJADV_SF0001";
	//static char* IndexFileSign = "PJADV_SF0001";
	HANDLE hFile;
	DWORD nReadSize;
	static char szTextDataFilePath[MAX_PATH];
	int nPathLen;


	PBYTE pIndexData;
	PBYTE pTextData;
	DWORD nIndexSize;
	DWORD nTextDataSize;

	bool retstatus = false;
	hFile = CreateFileA(pszFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		nIndexSize = GetFileSize(hFile,&nIndexSize);
		pIndexData = new BYTE[nIndexSize];
		if(ReadFile(hFile,(LPVOID)pIndexData,nIndexSize,&nReadSize,NULL))
		{
			CloseHandle(hFile);
			if(memcmp(pIndexData,IndexFileSign,strlen(IndexFileSign))!=0)
			{
				delete pIndexData;
				return false;
			}
			strcpy(szTextDataFilePath,pszFileName);
			nPathLen = strlen(szTextDataFilePath);
			for(int i=nPathLen;i>=0;i--)
			{
				if(szTextDataFilePath[i] == '\\')
				{
					szTextDataFilePath[i+1] = 0;
					break;
				}
			}
			strcat(szTextDataFilePath,"textdata.bin");
			hFile = CreateFileA(szTextDataFilePath,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if(hFile != INVALID_HANDLE_VALUE)
			{
				nTextDataSize = GetFileSize(hFile,&nTextDataSize);
				pTextData = new BYTE[nTextDataSize];

				if(ReadFile(hFile,(LPVOID)pTextData,nTextDataSize,&nReadSize,NULL))
				{
					CloseHandle(hFile);
					retstatus = AnalyaisData(pIndexData,nIndexSize,pTextData,nTextDataSize);
				}
				else
				{
					CloseHandle(hFile);
				}
			}
			else
			{
				delete pIndexData;
			}

		}
		else
		{
			CloseHandle(hFile);
			delete pIndexData;
		}
	}
	return retstatus;
}
