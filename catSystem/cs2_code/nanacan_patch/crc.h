/* crc.h */
#ifndef INC_CRC_H
#define INC_CRC_H
// MD5 Hash

#include "HashList.h"

typedef struct
{
	unsigned int buf[4];
    unsigned int bits[2];
    unsigned char in[64];
} MD5Context_t;

#define false FALSE
#define true TRUE
typedef unsigned char byte;

typedef uint32_t CRC32_t;
void CRC32_Init(CRC32_t *pulCRC);
CRC32_t CRC32_Final(CRC32_t pulCRC);
void CRC32_ProcessBuffer(CRC32_t *pulCRC, void *p, int len);
void CRC32_ProcessByte(CRC32_t *pulCRC, unsigned char ch);
BOOL CRC_File(CRC32_t *crcvalue, char *pszFileName);

void MD5Init(MD5Context_t *context);
void MD5Update(MD5Context_t *context, unsigned char const *buf,
               unsigned int len);
void MD5Final(unsigned char digest[16], MD5Context_t *context);
void Transform(unsigned int buf[4], unsigned int const in[16]);
BOOL MD5_Hash_File(unsigned char digest[16], char *pszFileName, BOOL bUsefopen = FALSE, BOOL bSeed = FALSE, unsigned int seed[4] = NULL);
char *MD5_Print(unsigned char hash[16]);
BOOL MD5_Hash_CachedFile(unsigned char digest[16], unsigned char *pCache, int nFileSize, BOOL bSeed = FALSE, unsigned int seed[4] = NULL);
__forceinline uint32_t CriteriaHash( const char *value, int length )
{
	CRC32_t val;
	CRC32_Init( &val );
	CRC32_ProcessBuffer(&val, (void *)value, length );
	return CRC32_Final( val );
}
#endif
