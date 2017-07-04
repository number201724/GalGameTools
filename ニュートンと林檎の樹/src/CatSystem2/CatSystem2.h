#ifndef _CATSYSTEM2_H_
#define _CATSYSTEM2_H_

#include "pragma_once.h"
#include <Windows.h>
#include "my_headers.h"
#include "gal_common.h"
#include "blowfish.h"
#include "MTwister.h"

#define CS2_INT_HEADER_MAGIC    TAG3('KIF')
#define CS2_SCENE_MAGIC         TAG8('CatS', 'cene')
#define CS2_FES_MAGIC           TAG3('FES')
#define CS2_HG2_MAGIC           TAG4('HG-2')
#define CS2_HG3_MAGIC           TAG4('HG-3')

#pragma pack(1)

typedef struct
{
    UInt32 Magic;
    UInt32 dwFileNum;
    Char   KeyName[0x40];
    UInt32 dwUnknown;
    UInt32 dwKey;
} CS2IntHeader;

typedef struct
{
    Char   FileName[0x40];
    UInt32 dwOffset;
    UInt32 dwSize;
} CS2IntEntry;

typedef struct
{
    UInt64 Magic;
    UInt32 CompressedSize;
    UInt32 UncompressedSize;
    Byte   CompressedData[1];
} *PCS2SceneHeader;

typedef struct
{
    UInt32 Magic;
    UInt32 CompressedSize;
    UInt32 UncompressedSize;
    UInt32 Reserve;
    Byte   CompressedData[1];
} *PCS2FESHeader;

typedef struct
{
    UInt32 Magic;
    UInt32 HeadSize;
    UInt32 Version;
} CS2HG3Header;

typedef struct
{
    UInt64 Tag;
    UInt32 AtomSize;
    UInt32 InfoSize;
} CS2HG3AtomHeader;

typedef struct
{
    UInt32 Width;
    UInt32 Heigth;
    UInt32 BitsPerPixel;
} CS2HG3AtomStdInfo;

typedef struct
{
    UInt32 Unknown;
    UInt32 Height;
    UInt32 CompressedSizeRGB;
    UInt32 DecompressedSizeRGB;
    UInt32 CompressedSizeAlpha;
    UInt32 DecompressedSizeAlpha;
} CS2HG3AtomImg;

typedef MY_FILE_ENTRY_BASE MyCS2IntEntry;

#pragma pack()

class CCatSystem2Unpacker : public CUnpackerImpl<CCatSystem2Unpacker>
{
protected:
    UInt32    m_Key;
    CMTwister m_MTwist;
    CBlowFish m_BlowFish;
    CFileDisk m_file;
    Char      m_CharTable[26 * 2];
    Char      m_CharTableReverse[26 * 2];

public:
    CCatSystem2Unpacker();

    Bool Open(PCWChar pszFileName);
    Bool GetFileData(SFileInfo *pFileInfo, const MY_FILE_ENTRY_BASE *pEntry);

//    DEFINE_AUTO_METHOD();

protected:
    Bool  InitIndex(PCWChar pszFileName, SizeT FileNum);
    Int32 HashString(PChar pszName);
    Void  DecryptFileName(PChar pszFileName, UInt32 FileNameHash);
    Void  SetSeed(UInt32 Seed);
    PChar GetKeyCode();
};

#endif /* _CATSYSTEM2_H_ */