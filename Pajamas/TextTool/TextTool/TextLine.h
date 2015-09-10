#pragma once

class CTextLine
{
private:
	PBYTE m_Data;
	DWORD m_Start;
	DWORD m_Size;
public:
	CTextLine(PBYTE Data,DWORD Size);
	virtual ~CTextLine(void);
	char* ReadLine();
	char* GetLineData(char* LineString);
	char* GetLineName(char* LineString);
};
