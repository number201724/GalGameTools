#include "CatSystem2.h"
#include "zlib/zlib.h"

INIT_CONSOLE_HANDLER

CCatSystem2Unpacker::CCatSystem2Unpacker()
{
    Char  up, low;
    PChar pUpStart, pUpEnd, pLowStart, pLowEnd;

    pUpStart  = m_CharTable;
    pUpEnd    = m_CharTableReverse + countof(m_CharTableReverse) - 1;
    pLowStart = pUpStart + 26;
    pLowEnd   = pUpEnd - 26;

    up  = 'A';
    low = 'a';
    for (SizeT i = 26; i; --i)
    {
        *pUpStart++  = up;
        *pUpEnd--    = up;
        *pLowStart++ = low;
        *pLowEnd--   = low;

        ++up;
        ++low;
    }
}

Bool CCatSystem2Unpacker::Open(PCWChar pszFileName)
{
    ReleaseAll();

    if (!m_file.Open(pszFileName))
        return False;

    CS2IntHeader header;

    if (!m_file.Read(&header, sizeof(header)))
        return False;

    if (header.Magic != CS2_INT_HEADER_MAGIC ||
        header.dwFileNum == 0 ||
        StrICompareA(header.KeyName, "__key__.dat"))
    {
        return False;
    }

    m_Key = header.dwKey;

    return InitIndex(pszFileName, --header.dwFileNum);
}

Bool CCatSystem2Unpacker::InitIndex(PCWChar pszFileName, SizeT FileNum)
{
    Int32 FileNameHash;
    CS2IntEntry *pEntry, *pIntEntry;
    MyCS2IntEntry *pMyEntry;

    UNUSED_VARIABLE(pszFileName);

    pEntry = (CS2IntEntry *)Alloc(FileNum * sizeof(*pEntry));
    if (pEntry == NULL)
        return False;

    if (!m_file.Read(pEntry, FileNum * sizeof(*pEntry)))
    {
        Free(pEntry);
        return False;
    }

    m_Index.pEntry = (MY_FILE_ENTRY_BASE *)(MyCS2IntEntry *)Alloc(FileNum * sizeof(*m_Index.pEntry));
    if (m_Index.pEntry == NULL)
    {
        Free(pEntry);
        return False;
    }

    m_Index.cbEntrySize = sizeof(*m_Index.pEntry);
    m_Index.FileCount.QuadPart = FileNum;
    SetSeed(m_Key);
    m_Key = m_MTwist.GetRandom();
    m_BlowFish.Initialize((PByte)&m_Key, sizeof(m_Key));

    FileNameHash = HashString(GetKeyCode());
    pIntEntry = pEntry;
    pMyEntry = m_Index.pEntry;
    for (SizeT i = 1; FileNum; ++i, --FileNum)
    {
        DecryptFileName(pIntEntry->FileName, FileNameHash + i);
        pIntEntry->dwOffset += i;
        m_BlowFish.Decode((PByte)&pIntEntry->dwOffset, (PByte)&pIntEntry->dwOffset, 8);
        pMyEntry->Offset.QuadPart = pIntEntry->dwOffset;
        pMyEntry->Size.QuadPart   = pIntEntry->dwSize;
        MultiByteToWideChar(CP_ACP, 0, pIntEntry->FileName, -1, pMyEntry->FileName, countof(pMyEntry->FileName));
        ++pIntEntry;
        ++pMyEntry;
    }

    Free(pEntry);

    return True;
}

Bool CCatSystem2Unpacker::GetFileData(SFileInfo *pFileInfo, const MY_FILE_ENTRY_BASE *pEntry)
{
    PByte   pbBuffer;
    SizeT   BufferSize;
    PCWChar pszExtension;

    if (!m_file.SeekEx(m_file.FILE_SEEK_BEGIN, pEntry->Offset))
        return False;

    BufferSize = pEntry->Size.LowPart;
    pbBuffer = (PByte)Alloc(BufferSize, 8);
    if (pbBuffer == NULL)
        return False;

    if (!m_file.Read(pbBuffer, BufferSize))
    {
        Free(pbBuffer);
        return False;
    }

    m_BlowFish.Decode((PByte)pbBuffer, (PByte)pbBuffer, BufferSize);

    pszExtension = findextw(pEntry->FileName);
    if (((PCS2SceneHeader)pbBuffer)->Magic == CS2_SCENE_MAGIC)
    {
        PByte pbUncomp;
        ULong OutSize;
        PCS2SceneHeader pSceneHeader = (PCS2SceneHeader)pbBuffer;

        OutSize = pSceneHeader->UncompressedSize;
        pbUncomp = (PByte)Alloc(OutSize);
        if (pbUncomp == NULL)
            goto DEFAULT_PROC;

        if (uncompress(pbUncomp, &OutSize, pSceneHeader->CompressedData, pSceneHeader->CompressedSize) != Z_OK)
        {
            Free(pbUncomp);
            goto DEFAULT_PROC;
        }

        Free(pbBuffer);
        pbBuffer = pbUncomp;
        BufferSize = OutSize;
    }
    else if (((PCS2FESHeader)pbBuffer)->Magic == CS2_FES_MAGIC)
    {
        PByte pbUncomp;
        ULong OutSize;
        PCS2FESHeader pFesHeader = (PCS2FESHeader)pbBuffer;

        OutSize = pFesHeader->UncompressedSize;
        pbUncomp = (PByte)Alloc(OutSize);
        if (pbUncomp == NULL)
            goto DEFAULT_PROC;

        if (uncompress(pbUncomp, &OutSize, pFesHeader->CompressedData, pFesHeader->CompressedSize) != Z_OK)
        {
            Free(pbUncomp);
            goto DEFAULT_PROC;
        }

        Free(pbBuffer);
        pbBuffer = pbUncomp;
        BufferSize = OutSize;
    }

DEFAULT_PROC:
    pFileInfo->FileType = UNPACKER_FILE_TYPE_BIN;
    pFileInfo->BinData.BufferSize = BufferSize;
    pFileInfo->BinData.pbBuffer = pbBuffer;

    return True;
}

Void CCatSystem2Unpacker::DecryptFileName(PChar pszFileName, UInt32 FileNameHash)
{
    UInt32 ch, Index;

    ch = *pszFileName;
    if (ch == 0)
        return;

    SetSeed(FileNameHash);
    Index = m_MTwist.GetRandom();
    Index = ((Index >> 16) + Index + (Index >> 24) + (Index >> 8)) & 0xFF;
    for (; ch; ++Index, ch = *++pszFileName)
    {
        if (!IN_RANGE('A', ch & 0xDF, 'Z'))
            continue;

        for (SizeT j = Index, i = countof(m_CharTableReverse); i; --i)
        {
            if ((UInt32)m_CharTableReverse[j++ % countof(m_CharTableReverse)] == ch)
            {
                *pszFileName = m_CharTable[countof(m_CharTableReverse) - i];
                break;
            }
        }
    }
}

Int32 CCatSystem2Unpacker::HashString(PChar pszName)
{
    Int32 ch, Hash;

    Hash = -1;
    ch = *pszName++;
    if (ch == 0)
        return Hash;

    do
    {
        Hash ^= ch << 24;
        for (SizeT i = 8; i; --i)
        {
            if (Hash < 0)
                Hash = (Hash + Hash) ^ 0x4C11DB7;
            else
                Hash += Hash;
        }

        Hash = ~Hash;
        ch = *pszName++;
    } while (ch);

    return Hash;
}

Void CCatSystem2Unpacker::SetSeed(UInt32 Seed)
{
    m_MTwist.SetSeed(Seed);
}

PChar CCatSystem2Unpacker::GetKeyCode()
{
#ifdef CS2_GRISAIA
    return "FW-6JD55162";
#endif
}