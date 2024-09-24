/*
**********************************************************************
** DXBCChecksum.c                                                   **
**                                                                  **
** - Modified by Seth Sowerby, November 2007                        **
**   Modified to match DXBC checksum algorithm used by Microsoft    **
**                                                                  **
**********************************************************************
*/

#include <string.h>

#include "md5.h"

typedef unsigned char BYTE;
typedef unsigned long DWORD;

static const DWORD dwHashOffset = 0x14;

void CalculateDXBCChecksum(BYTE* pData, DWORD dwSize, DWORD dwHash[4])
{
    const unsigned char PADDING[64] = { 0x80 };

    MD5_CTX md5Ctx;
    MD5Init(&md5Ctx);

    // Skip the start of the shader header
    dwSize -= dwHashOffset;
    pData += dwHashOffset;

    DWORD dwNumberOfBits = dwSize * 8;

    // First we hash all the full chunks available
    DWORD dwFullChunksSize = dwSize & 0xffffffc0;
    MD5Update(&md5Ctx, pData, dwFullChunksSize);

    DWORD dwLastChunkSize = dwSize - dwFullChunksSize;
    DWORD dwPaddingSize = 64  - dwLastChunkSize;
    BYTE* pLastChunkData = pData + dwFullChunksSize;

    if (dwLastChunkSize >= 56)
    {
        MD5Update(&md5Ctx, pLastChunkData, dwLastChunkSize);

        /* Pad out to 56 mod 64 */
        MD5Update(&md5Ctx, PADDING, dwPaddingSize);

        // Pass in the number of bits
        UINT4 in[16];
        memset(in, 0, sizeof(in));
        in[0] = dwNumberOfBits;
        in[15] = (dwNumberOfBits >> 2) | 1;

        MD5Transform(md5Ctx.buf, in);
    }
    else
    {
        // Pass in the number of bits
        MD5Update(&md5Ctx, (unsigned char*) &dwNumberOfBits, 4);

        if (dwLastChunkSize)
        {
            MD5Update(&md5Ctx, pLastChunkData, dwLastChunkSize);
        }

        // Adjust for the space used for dwNumberOfBits
        dwLastChunkSize += sizeof(DWORD);
        dwPaddingSize -= sizeof(DWORD);

        /* Pad out to 56 mod 64 */
        memcpy(&md5Ctx.in[dwLastChunkSize], PADDING, dwPaddingSize);

        ((UINT4*)md5Ctx.in)[15] = (dwNumberOfBits >> 2) | 1;

        UINT4 in[16];
        memcpy(in, md5Ctx.in, 64);

        MD5Transform(md5Ctx.buf, in);
    }

    memcpy(dwHash, md5Ctx.buf, 4 * sizeof(DWORD));
}
