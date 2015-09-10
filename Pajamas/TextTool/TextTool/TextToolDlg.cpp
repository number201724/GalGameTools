#include "stdafx.h"
#include "TextTool.h"
#include "TextToolDlg.h"
#include "DataStream.h"
#include "resource.h"
#include "ByteStream.h"
#include "TextLine.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDataStream DataStream;

int CurrentLine = 0;
int MaxLine = 0;

CTextToolDlg::CTextToolDlg(CWnd* pParent /*=NULL*/)
: CDialog(CTextToolDlg::IDD, pParent)
, m_OrigText(_T(""))
, m_NewText(_T(""))
, m_IndexEdit(_T(""))
, m_MaxCount(_T(""))
, m_CountValue(_T("1"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTextToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT3, m_OrigText);
	DDX_Text(pDX, IDC_EDIT2, m_NewText);
	DDX_Control(pDX, IDC_SPIN1, m_Spin_Count);

	DDX_Text(pDX, IDC_STATIC7, m_MaxCount);
	DDX_Control(pDX, IDC_EDIT1, m_CountEdit);
	DDX_Text(pDX, IDC_EDIT1, m_CountValue);
}

BEGIN_MESSAGE_MAP(CTextToolDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CTextToolDlg::OnDeltaposSpin1)
	//	ON_EN_CHANGE(IDC_EDIT5, &CTextToolDlg::OnEnChangeEdit5)
	ON_BN_CLICKED(IDC_BUTTON5, &CTextToolDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON1, &CTextToolDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT1, &CTextToolDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON7, &CTextToolDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON3, &CTextToolDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CTextToolDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON6, &CTextToolDlg::OnBnClickedButton6)
END_MESSAGE_MAP()

BOOL CTextToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_Spin_Count.SetBuddy((CWnd*)&m_CountEdit);
	m_Spin_Count.SetRange32(2147483647,1);

	return TRUE;
}

void CTextToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTextToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTextToolDlg::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	//	char tracelog[32];
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	CurrentLine  = pNMUpDown->iPos + pNMUpDown->iDelta;
	if(CurrentLine == 0)
		CurrentLine = 1;
	if(CurrentLine == MaxLine)
		CurrentLine = MaxLine - 1;

	GetTextData();

	*pResult = 0;
}

void CTextToolDlg::OnEnChangeEdit5()
{

}

void CTextToolDlg::GetTextData()
{
	if(DataStream.EditList.size() > 0 )
	{
		int idx = CurrentLine - 1;
		if(idx < 0)
		{
			idx = 0;
		}
		if(idx >= MaxLine)
		{
			idx = MaxLine - 1;
		}
		UpdateData(TRUE);

		m_OrigText = DataStream.EditList[idx]->Data->pOrigText;

		if(DataStream.EditList[idx]->Data->pNewText)
		{
			m_NewText = DataStream.EditList[idx]->Data->pNewText;
		}
		else
		{
			m_NewText = "";
		}
		UpdateData(FALSE);
	}
}
void CTextToolDlg::OnOK()
{

}
void CTextToolDlg::SetTextData()
{
	int idx = CurrentLine - 1;
	if(idx < 0)
	{
		idx = 0;
	}
	if(idx >= MaxLine)
	{
		idx = MaxLine - 1;
	}
	UpdateData(TRUE);

	if(DataStream.EditList[idx]->Data->pNewText)
	{
		delete DataStream.EditList[idx]->Data->pNewText;
		DataStream.EditList[idx]->Data->pNewText = NULL;
	}
	if(m_NewText.GetLength())
	{
		DataStream.EditList[idx]->Data->pNewText = new char[m_NewText.GetLength()+1];
		strcpy(DataStream.EditList[idx]->Data->pNewText,m_NewText.GetBuffer());
	}
	UpdateData(FALSE);
}
void CTextToolDlg::OnBnClickedButton5()
{
	static char FormatText[128];
	CFileDialog FileData(TRUE);
	if(DataStream.EditList.size() > 0)
	{
		MessageBox("文件数据已经载入,请不要重复打开!");
		return;
	}
	if(FileData.DoModal()==IDOK)
	{
		DataStream.ReadStreamForData(FileData.GetPathName().GetBuffer());
		int dataCount = DataStream.EditList.size();
		sprintf(FormatText,"%d行",dataCount);
		m_MaxCount = FormatText;
		UpdateData(FALSE);

		MaxLine = dataCount;
		CurrentLine = 1;
		GetTextData();

	}
}

void CTextToolDlg::OnBnClickedButton1()
{
	SetTextData();
}

void CTextToolDlg::OnEnChangeEdit1()
{
	char newstr[32];
	char rep_str[32];
	int x = 0;
	UpdateData();
	strcpy(newstr,m_CountValue.GetBuffer());
	int len = strlen(newstr);
	for(int i=0;i<len;i++)
	{
		if(newstr[i] != ',')
		{
			rep_str[x] = newstr[i];
			x++;
		}
	}
	rep_str[x] = 0;
	CurrentLine = atoi(rep_str);
	if(CurrentLine >= MaxLine)
	{
		m_CountValue = _ltoa(atoi(rep_str)-1,newstr,10);
		UpdateData(FALSE);
	}

	GetTextData();
	//MessageBox(rep_str);
}
void XorData(BYTE Value,PBYTE data,DWORD size)
{
	BYTE newValue = Value;
	for(DWORD i=0;i<size;i++)
	{
		data[i] ^= newValue;
		newValue+=0x5c;
	}
}
void CTextToolDlg::OnBnClickedButton7()
{
	CFileDialog FileData(TRUE);
	if(FileData.DoModal()==IDOK)
	{
		HANDLE hFile = CreateFileA(FileData.GetPathName().GetBuffer(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			DWORD nReadSize;
			DWORD fileSize;
			fileSize = GetFileSize(hFile,&fileSize);

			PBYTE fileData = new BYTE[fileSize];
			ReadFile(hFile,fileData,fileSize,&nReadSize,NULL);
			XorData(0xC5,fileData,fileSize);
			SetFilePointer(hFile,NULL,NULL,NULL);
			WriteFile(hFile,fileData,fileSize,&nReadSize,NULL);
			CloseHandle(hFile);
			MessageBox("文件加密/解密成功");
		}
	}
}
void CTextToolDlg::OnBnClickedButton3()
{
	DWORD nWrittenByte;
	HANDLE hFile;
	static char m_Buffer[1024];
	CByteStream *Stream = new CByteStream();
	char szSaveFile[MAX_PATH];
	GetCurrentDirectoryA(sizeof(szSaveFile),szSaveFile);
	strcat(szSaveFile,"\\TextData.ini");

	DeleteFile(szSaveFile);
	hFile = CreateFileA(szSaveFile,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
	int listsize = DataStream.EditList.size();
	for(int i=0;i<listsize;i++)
	{
		text_edit_t* m_Item = DataStream.EditList[i];
		switch(m_Item->nType)
		{
		case sig_title:
			{

				sprintf(m_Buffer,"[%d]",i);
				offset_title_t* title_data = (offset_title_t*)m_Item->Data;
				int subsize = title_data->OffsetList.size();
				Stream->WriteLine(m_Buffer);
				sprintf(m_Buffer,"未汉化=%s",title_data->Text.pOrigText);
				Stream->WriteLine(m_Buffer);
				if(title_data->Text.pNewText)
				{
					sprintf(m_Buffer,"汉化=%s",title_data->Text.pNewText);
				}
				else
				{
					sprintf(m_Buffer,"汉化=");
				}
				Stream->WriteLine(m_Buffer);
				Stream->WriteLine("类型=1");

				Stream->WriteString("索引偏移=");
				for(int k=0;k<subsize;k++)
				{
					sprintf(m_Buffer,"%X,",title_data->OffsetList[k]);
					Stream->WriteString(m_Buffer);
				}
				Stream->WriteLine("");

				break;
			}
		case sig_text:
			{
				sprintf(m_Buffer,"[%d]",i);
				offset_string_t* strdata = (offset_string_t*)m_Item->Data;
				Stream->WriteLine(m_Buffer);

				sprintf(m_Buffer,"未汉化=%s",strdata->Text.pOrigText);
				Stream->WriteLine(m_Buffer);
				if(strdata->Text.pNewText)
				{
					sprintf(m_Buffer,"汉化=%s",strdata->Text.pNewText);
				}
				else
				{
					sprintf(m_Buffer,"汉化=");
				}
				Stream->WriteLine(m_Buffer);
				Stream->WriteLine("类型=2");

				sprintf(m_Buffer,"索引偏移=%X,",strdata->indexoffset);
				Stream->WriteLine(m_Buffer);

				break;
			}
		}
	}

	WriteFile(hFile,Stream->GetBuffer(),Stream->GetLength(),&nWrittenByte,NULL);
	CloseHandle(hFile);
	delete Stream;
	MessageBox("汉化信息已经保存....");
}

void CTextToolDlg::OnBnClickedButton4()
{
	static char FormatText[128];
	if(DataStream.EditList.size() > 0)
	{
		MessageBox("信息已经载入,请不要重复读取");
		return;
	}
	char szSaveFile[MAX_PATH];
	GetCurrentDirectoryA(sizeof(szSaveFile),szSaveFile);
	strcat(szSaveFile,"\\TextData.ini");

	HANDLE hFile = CreateFileA(szSaveFile,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return;
	DWORD fileSize;
	DWORD nReadSize;
	fileSize = GetFileSize(hFile,&fileSize);
	BYTE *fileData = new BYTE[fileSize];
	if(!fileData)
	{
		CloseHandle(hFile);
		return;
	}
	if(!ReadFile(hFile,(LPVOID)fileData,fileSize,&nReadSize,NULL))
	{
		delete fileData;
		CloseHandle(hFile);
		return;
	}
	CloseHandle(hFile);
	CTextLine * TextStream = new CTextLine(fileData,fileSize);
	char* LineString = TextStream->ReadLine();

	if(LineString && LineString[0] == '[')
	{
		do
		{
			char* pOrigText = TextStream->ReadLine();
			char* pNewText =  TextStream->ReadLine();
			char* pType = TextStream->ReadLine();
			char* pOffsetText = TextStream->ReadLine();

			if(
				pOrigText &&
				pNewText && 
				pType &&
				pOffsetText &&
				(strcmp(TextStream->GetLineName(pOrigText),"未汉化") == 0) && 
				(strcmp(TextStream->GetLineName(pNewText),"汉化") == 0) && 
				(strcmp(TextStream->GetLineName(pType),"类型") == 0) && 
				(strcmp(TextStream->GetLineName(pOffsetText),"索引偏移") == 0) 
				)
			{
				int nType = atoi(TextStream->GetLineData(pType));
				switch(nType)
				{
				case sig_title:
					{
						offset_title_s* m_title= new offset_title_s;
						m_title->nToOffset = 0;
						int len;
						char* pstr;
						pstr = TextStream->GetLineData(pOrigText);
						len = strlen(pstr);
						m_title->Text.pOrigText = new char[len+1];
						strcpy(m_title->Text.pOrigText,pstr);
						pstr = TextStream->GetLineData(pNewText);
						len = strlen(pstr);
						if(len > 0)
						{
							m_title->Text.pNewText = new char[len+1];
							strcpy(m_title->Text.pNewText,pstr);
						}
						else
						{
							m_title->Text.pNewText = NULL;
						}
						pstr = TextStream->GetLineData(pOffsetText);
						len = strlen(pstr);
						int count = 0;
						int offset = 0;
						for(int i=0;i<len;i++)
						{
							if(pstr[i] == ',')
							{
								pstr[i] = 0;
								count++;
							}
						}
						DWORD mData;
						for(int i=0;i<count;i++)
						{
							offset += strlen(pstr+offset) + 1;
							sscanf(pstr+offset,"%X",&mData);
							m_title->OffsetList.push_back(mData);
						}
						DataStream.TitleList.push_back(m_title);
						break;
					}
				case sig_text:
					{
						offset_string_t* m_string = new offset_string_t;
						int len;
						char* pstr;
						pstr = TextStream->GetLineData(pOrigText);
						len = strlen(pstr);
						m_string->Text.pOrigText = new char[len+1];
						strcpy(m_string->Text.pOrigText,pstr);

						pstr = TextStream->GetLineData(pNewText);
						len = strlen(pstr);
						if(len > 0)
						{
							m_string->Text.pNewText = new char[len+1];
							strcpy(m_string->Text.pNewText,pstr);
						}
						else
						{
							m_string->Text.pNewText = NULL;
						}
						pstr = TextStream->GetLineData(pOffsetText);
						len = strlen(pstr);
						pstr[len-1] = 0;
						sscanf(pstr,"%X",&m_string->indexoffset);
						m_string->nToOffset = 0;
						DataStream.List.push_back(m_string);
						break;
					}
				default:
					MessageBox("无效的类型信息!");
					abort();
				}
				delete pOrigText;
				delete pNewText;
				delete pType;
				delete pOffsetText;
			}
			else
			{
				MessageBox("无效的配置信息!");
				abort();
				break;
			}

			do
			{
				delete LineString;
				LineString = TextStream->ReadLine();
			}while(LineString && LineString[0] != '[');
		}while(LineString);
	}
	DataStream.FormatToEditList();

	int dataCount = DataStream.EditList.size();
	sprintf(FormatText,"%d行",dataCount);
	m_MaxCount = FormatText;
	UpdateData(FALSE);

	MaxLine = dataCount;
	CurrentLine = 1;
	GetTextData();

	delete TextStream;
}

/*将汉化信息保存到数据文件*/
void CTextToolDlg::OnBnClickedButton6()
{
	char dataFileName[MAX_PATH];
	int nPathLen;
	HANDLE hIndexFile;
	HANDLE hDataFile;
	CFileDialog FileData(TRUE);
	if(DataStream.EditList.size() <= 0)
	{
		MessageBox("没有载入数据,请检查!");
		return;
	}
	if(FileData.DoModal()==IDOK)
	{
		strcpy(dataFileName,FileData.GetPathName());
		hIndexFile = CreateFileA(dataFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hIndexFile == INVALID_HANDLE_VALUE)
			return;
		nPathLen = strlen(dataFileName);
		for(int i=nPathLen;i>=0;i--)
		{
			if(dataFileName[i] == '\\')
			{
				dataFileName[i+1] = 0;
				break;
			}
		}
		strcat(dataFileName,"textdata.bin");
		hDataFile = CreateFileA(dataFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hDataFile == INVALID_HANDLE_VALUE){
			CloseHandle(hIndexFile);
			return;
		}

		DWORD indexFileSize;
		DWORD dataFileSize;
		indexFileSize = GetFileSize(hIndexFile,&indexFileSize);
		dataFileSize = GetFileSize(hDataFile,&dataFileSize);
		if(!dataFileSize || !indexFileSize)
		{
			CloseHandle(hIndexFile);
			CloseHandle(hDataFile);
			MessageBox("无效的索引或数据文件");
			return;
		}
		DWORD nReadSize;
		PBYTE pIndexData = new BYTE[indexFileSize];
		PBYTE pStringData = new BYTE[dataFileSize];

		ReadFile(hIndexFile,pIndexData,indexFileSize,&nReadSize,NULL);
		ReadFile(hDataFile,pStringData,dataFileSize,&nReadSize,NULL);
		CloseHandle(hIndexFile);
		CloseHandle(hDataFile);

		CByteStream *StringStream  = new CByteStream();
		StringStream->WriteBytes(pStringData,dataFileSize);

		int listsize = DataStream.EditList.size();
		for(int i=0;i<listsize;i++)
		{
			switch(DataStream.EditList[i]->nType)
			{
			case sig_title:
				{
					offset_title_t* title = (offset_title_t*)DataStream.EditList[i]->Data;
					DWORD newoffset = StringStream->GetLength();
					if(title->Text.pNewText)
					{
						StringStream->WriteString(title->Text.pNewText);
					}
					else
					{
						StringStream->WriteString(title->Text.pOrigText);
					}

					StringStream->WriteByte(0);
					StringStream->WriteByte(0);
					int m_size = title->OffsetList.size();
					for(int i=0;i<m_size;i++)
					{
						*(DWORD*)((DWORD)pIndexData + title->OffsetList[i]) = newoffset;
					}
					break;
				}
			case sig_text:
				{
					offset_string_t* m_string = (offset_string_t*)DataStream.EditList[i]->Data;
					DWORD newoffset = StringStream->GetLength();
					if(m_string->Text.pNewText)
					{
						StringStream->WriteString(m_string->Text.pNewText);
					}
					else
					{
						StringStream->WriteString(m_string->Text.pOrigText);
					}
					StringStream->WriteByte(0);
					StringStream->WriteByte(0);
					*(DWORD*)((DWORD)pIndexData + m_string->indexoffset) = newoffset;
					break;
				}
			default:
				abort();
			}
		}
		GetCurrentDirectory(sizeof(dataFileName),dataFileName);
		strcat(dataFileName,"\\scenario.dat");

		hIndexFile = CreateFile(
			dataFileName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,NULL);


		GetCurrentDirectory(sizeof(dataFileName),dataFileName);
		strcat(dataFileName,"\\textdata.bin");
		hDataFile = CreateFile(
			dataFileName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,NULL);

		WriteFile(hIndexFile,pIndexData,indexFileSize,&nReadSize,NULL);
		WriteFile(hDataFile,StringStream->GetBuffer(),StringStream->GetLength(),&nReadSize,NULL);

		CloseHandle(hDataFile);
		CloseHandle(hIndexFile);

		delete pIndexData;
		delete StringStream;
		delete pStringData;
		MessageBox("数据保存成功!");
	}
}
