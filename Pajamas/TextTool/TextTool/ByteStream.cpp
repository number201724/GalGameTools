#include "StdAfx.h"
#include "ByteStream.h"

CByteStream::CByteStream(void)
{
	m_Size = 0;
	m_Data = (PBYTE)malloc(m_Size);
	
}

CByteStream::~CByteStream(void)
{
	free(m_Data);
}

void CByteStream::WriteBytes(PBYTE Data,size_t Size)
{
	m_Data = (PBYTE)realloc(m_Data,m_Size+Size);
	memcpy((void*)&m_Data[m_Size],Data,Size);
	m_Size += Size;
}
void CByteStream::WriteByte(BYTE Data)
{
	m_Data = (PBYTE)realloc(m_Data,m_Size+1);
	memcpy((void*)&m_Data[m_Size],(const void*)&Data,1);

	m_Size++;
}
void CByteStream::WriteLine(char* string)
{
	size_t len = strlen(string);
	m_Data = (PBYTE)realloc(m_Data,m_Size+len);
	memcpy((char*)&m_Data[m_Size],string,len);
	m_Size += len;

	WriteByte('\r');
	WriteByte('\n');
}
void CByteStream::WriteString(char* string)
{
	size_t len = strlen(string);
	m_Data = (PBYTE)realloc(m_Data,m_Size+len);
	memcpy((char*)&m_Data[m_Size],string,len);
	m_Size += len;
	//WriteByte(0);
}
PBYTE CByteStream::GetBuffer()
{
	return (PBYTE)m_Data;
}
DWORD CByteStream::GetLength()
{
	return m_Size;
}
void CByteStream::Release()
{
	free(m_Data);
	m_Size = 0;
	m_Data = (PBYTE)malloc(m_Size);
}