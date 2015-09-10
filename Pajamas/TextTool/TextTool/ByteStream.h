#pragma once

class CByteStream
{
private:
	PBYTE m_Data;
	DWORD m_Size;
public:
	CByteStream(void);
	virtual ~CByteStream(void);
	virtual void WriteBytes(PBYTE Data,size_t Size);
	virtual void WriteByte(BYTE Data);
	virtual void WriteString(char* string);
	virtual void WriteLine(char* string);
	virtual PBYTE GetBuffer();
	virtual DWORD GetLength();
	virtual void Release();

};
