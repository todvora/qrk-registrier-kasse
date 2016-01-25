/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2016 Christian Kvasny <chris@ckvsoft.at>
 *
 * Based on Base32 code from Graham Bull.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "base32encode.h"
#include <assert.h>

/****************************** Base32 Encoding ******************************/

/*
 * output 8 bytes for every 5 input:
 *
 *                 inputs: 1        2        3        4        5
 * outputs: 1 = ---11111 = 11111---
 *          2 = ---222XX = -----222 XX------
 *          3 = ---33333 =          --33333-
 *          4 = ---4XXXX =          -------4 XXXX----
 *          5 = ---5555X =                   ----5555 X-------
 *          6 = ---66666 =                            -66666--
 *          7 = ---77XXX =                            ------77 XXX-----
 *          8 = ---88888 =                                     ---88888
 */

static const size_t BASE32_INPUT = 5;
static const size_t BASE32_OUTPUT = 8;
static const char* const BASE32_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567=";

size_t Base32Encode::GetLength(size_t srcBytes)
{
  return (((srcBytes + BASE32_INPUT - 1) / BASE32_INPUT) * BASE32_OUTPUT) + 1; /*plus terminator*/
}

size_t Base32Encode::Encode(char* dest, const void* src, size_t srcBytes)
{
  if (dest && src)
  {
    unsigned char* pSrc = (unsigned char*)src;
    char* pDest = dest;
    size_t dwSrcSize = srcBytes;
    size_t dwDestSize = 0;
    size_t dwBlockSize;
    unsigned char n1, n2, n3, n4, n5, n6, n7, n8;

    while (dwSrcSize >= 1)
    {
      /* Encode inputs */
      dwBlockSize = (dwSrcSize < BASE32_INPUT ? dwSrcSize : BASE32_INPUT);
      n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = 0;
      switch (dwBlockSize)
      {
        case 5:
          n8 = (pSrc[4] & 0x1f);
          n7 = ((pSrc[4] & 0xe0) >> 5);
        case 4:
          n7 |= ((pSrc[3] & 0x03) << 3);
          n6 = ((pSrc[3] & 0x7c) >> 2);
          n5 = ((pSrc[3] & 0x80) >> 7);
        case 3:
          n5 |= ((pSrc[2] & 0x0f) << 1);
          n4 = ((pSrc[2] & 0xf0) >> 4);
        case 2:
          n4 |= ((pSrc[1] & 0x01) << 4);
          n3 = ((pSrc[1] & 0x3e) >> 1);
          n2 = ((pSrc[1] & 0xc0) >> 6);
        case 1:
          n2 |= ((pSrc[0] & 0x07) << 2);
          n1 = ((pSrc[0] & 0xf8) >> 3);
          break;

        default:
          assert(0);
      }
      pSrc += dwBlockSize;
      dwSrcSize -= dwBlockSize;

      /* Validate */
      assert(n1 <= 31);
      assert(n2 <= 31);
      assert(n3 <= 31);
      assert(n4 <= 31);
      assert(n5 <= 31);
      assert(n6 <= 31);
      assert(n7 <= 31);
      assert(n8 <= 31);

      /* Padding */
      switch (dwBlockSize)
      {
        case 1: n3 = n4 = 32;
        case 2: n5 = 32;
        case 3: n6 = n7 = 32;
        case 4: n8 = 32;
        case 5:
          break;

        default:
          assert(0);
      }

      /* 8 outputs */
      *pDest++ = BASE32_TABLE[n1];
      *pDest++ = BASE32_TABLE[n2];
      *pDest++ = BASE32_TABLE[n3];
      *pDest++ = BASE32_TABLE[n4];
      *pDest++ = BASE32_TABLE[n5];
      *pDest++ = BASE32_TABLE[n6];
      *pDest++ = BASE32_TABLE[n7];
      *pDest++ = BASE32_TABLE[n8];
      dwDestSize += BASE32_OUTPUT;
    }
    *pDest++ = '\x0'; /*append terminator*/

    return dwDestSize;
  }
  else
    return 0; /*ERROR - null pointer*/
}

size_t Base32Encode::Encode(wchar_t* dest, const void* src, size_t srcBytes)
{
  if (dest && src)
  {
    unsigned char* pSrc = (unsigned char*)src;
    wchar_t* pDest = dest;
    size_t dwSrcSize = srcBytes;
    size_t dwDestSize = 0;
    size_t dwBlockSize;
    unsigned char n1, n2, n3, n4, n5, n6, n7, n8;

    while (dwSrcSize >= 1)
    {
      /* Encode inputs */
      dwBlockSize = (dwSrcSize < BASE32_INPUT ? dwSrcSize : BASE32_INPUT);
      n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = 0;
      switch (dwBlockSize)
      {
        case 5:
          n8 = (pSrc[4] & 0x1f);
          n7 = ((pSrc[4] & 0xe0) >> 5);
        case 4:
          n7 |= ((pSrc[3] & 0x03) << 3);
          n6 = ((pSrc[3] & 0x7c) >> 2);
          n5 = ((pSrc[3] & 0x80) >> 7);
        case 3:
          n5 |= ((pSrc[2] & 0x0f) << 1);
          n4 = ((pSrc[2] & 0xf0) >> 4);
        case 2:
          n4 |= ((pSrc[1] & 0x01) << 4);
          n3 = ((pSrc[1] & 0x3e) >> 1);
          n2 = ((pSrc[1] & 0xc0) >> 6);
        case 1:
          n2 |= ((pSrc[0] & 0x07) << 2);
          n1 = ((pSrc[0] & 0xf8) >> 3);
          break;

        default:
          assert(0);
      }
      pSrc += dwBlockSize;
      dwSrcSize -= dwBlockSize;

      /* Validate */
      assert(n1 <= 31);
      assert(n2 <= 31);
      assert(n3 <= 31);
      assert(n4 <= 31);
      assert(n5 <= 31);
      assert(n6 <= 31);
      assert(n7 <= 31);
      assert(n8 <= 31);

      /* Padding */
      switch (dwBlockSize)
      {
        case 1: n3 = n4 = 32;
        case 2: n5 = 32;
        case 3: n6 = n7 = 32;
        case 4: n8 = 32;
        case 5:
          break;

        default:
          assert(0);
      }

      /* 8 outputs */
      *pDest++ = BASE32_TABLE[n1];
      *pDest++ = BASE32_TABLE[n2];
      *pDest++ = BASE32_TABLE[n3];
      *pDest++ = BASE32_TABLE[n4];
      *pDest++ = BASE32_TABLE[n5];
      *pDest++ = BASE32_TABLE[n6];
      *pDest++ = BASE32_TABLE[n7];
      *pDest++ = BASE32_TABLE[n8];
      dwDestSize += BASE32_OUTPUT;
    }
    *pDest++ = L'\x0'; /*append terminator*/

    return dwDestSize;
  }
  else
    return 0; /*ERROR - null pointer*/
}

size_t Base32Encode::EncodeBlock(char* dest, const void* src)
{
  if (dest && src)
  {
    unsigned char* pSrc = (unsigned char*)src;
    char* pDest = dest;
    unsigned char n1, n2, n3, n4, n5, n6, n7, n8;

    /* Encode inputs */
    n8 = (pSrc[4] & 0x1f);
    n7 = ((pSrc[4] & 0xe0) >> 5);
    n7 |= ((pSrc[3] & 0x03) << 3);
    n6 = ((pSrc[3] & 0x7c) >> 2);
    n5 = ((pSrc[3] & 0x80) >> 7);
    n5 |= ((pSrc[2] & 0x0f) << 1);
    n4 = ((pSrc[2] & 0xf0) >> 4);
    n4 |= ((pSrc[1] & 0x01) << 4);
    n3 = ((pSrc[1] & 0x3e) >> 1);
    n2 = ((pSrc[1] & 0xc0) >> 6);
    n2 |= ((pSrc[0] & 0x07) << 2);
    n1 = ((pSrc[0] & 0xf8) >> 3);

    /* Validate */
    assert(n1 <= 31);
    assert(n2 <= 31);
    assert(n3 <= 31);
    assert(n4 <= 31);
    assert(n5 <= 31);
    assert(n6 <= 31);
    assert(n7 <= 31);
    assert(n8 <= 31);

    /* 8 outputs */
    *pDest++ = BASE32_TABLE[n1];
    *pDest++ = BASE32_TABLE[n2];
    *pDest++ = BASE32_TABLE[n3];
    *pDest++ = BASE32_TABLE[n4];
    *pDest++ = BASE32_TABLE[n5];
    *pDest++ = BASE32_TABLE[n6];
    *pDest++ = BASE32_TABLE[n7];
    *pDest++ = BASE32_TABLE[n8];

    return BASE32_OUTPUT;
  }
  else
    return 0; /*ERROR - null pointer*/
}

size_t Base32Encode::EncodeBlock(wchar_t* dest, const void* src)
{
  if (dest && src)
  {
    unsigned char* pSrc = (unsigned char*)src;
    wchar_t* pDest = dest;
    unsigned char n1, n2, n3, n4, n5, n6, n7, n8;

    /* Encode inputs */
    n8 = (pSrc[4] & 0x1f);
    n7 = ((pSrc[4] & 0xe0) >> 5);
    n7 |= ((pSrc[3] & 0x03) << 3);
    n6 = ((pSrc[3] & 0x7c) >> 2);
    n5 = ((pSrc[3] & 0x80) >> 7);
    n5 |= ((pSrc[2] & 0x0f) << 1);
    n4 = ((pSrc[2] & 0xf0) >> 4);
    n4 |= ((pSrc[1] & 0x01) << 4);
    n3 = ((pSrc[1] & 0x3e) >> 1);
    n2 = ((pSrc[1] & 0xc0) >> 6);
    n2 |= ((pSrc[0] & 0x07) << 2);
    n1 = ((pSrc[0] & 0xf8) >> 3);

    /* Validate */
    assert(n1 <= 31);
    assert(n2 <= 31);
    assert(n3 <= 31);
    assert(n4 <= 31);
    assert(n5 <= 31);
    assert(n6 <= 31);
    assert(n7 <= 31);
    assert(n8 <= 31);

    /* 8 outputs */
    *pDest++ = BASE32_TABLE[n1];
    *pDest++ = BASE32_TABLE[n2];
    *pDest++ = BASE32_TABLE[n3];
    *pDest++ = BASE32_TABLE[n4];
    *pDest++ = BASE32_TABLE[n5];
    *pDest++ = BASE32_TABLE[n6];
    *pDest++ = BASE32_TABLE[n7];
    *pDest++ = BASE32_TABLE[n8];

    return BASE32_OUTPUT;
  }
  else
    return 0; /*ERROR - null pointer*/
}
