#include "StdAfx.h"
#include "TextLine.h"

CTextLine::CTextLine(PBYTE Data,DWORD Size)
{
	m_Data = Data;
	m_Start = 0;
	m_Size = Size;
}

CTextLine::~CTextLine(void)
{
	delete m_Data;
}

char* CTextLine::ReadLine()
{
	if(m_Start >= m_Size)
	{
		return NULL;
	}
	bool done=false;
	int length = 0;
	char* m_pszNextText = (char*)&m_Data[m_Start];
	do
	{
		length++;
		if(m_pszNextText[length] == '\r' && m_pszNextText[length+1] == '\n')
		{
			done = true;
			break;
		}
	}while(m_Start < m_Size);
	if(!done)
		return NULL;
	if(length > 0)
	{
		char* m_OutText = new char[length+1];
		memcpy(m_OutText,m_pszNextText,length);
		m_OutText[length] = 0;
		m_Start += length;
		m_Start += 2;
		return m_OutText;
	}
	return NULL;
}

char* CTextLine::GetLineData(char* LineString)
{
	int len = strlen(LineString);
	for(int i=0;i<len;i++)
	{
		if(LineString[i] == '=')
		{
			return &LineString[i+1];
		}
	}
	return NULL;
}
char* CTextLine::GetLineName(char* LineString)
{
	static char name[256];

	int len = strlen(LineString);
	for(int i=0;i<len;i++)
	{
		if(LineString[i] == '=')
		{
			memcpy(name,LineString,i);
			name[i] = 0;
			return (char*)&name;
			//return &LineString[i+1];
		}
	}
	return NULL;

}