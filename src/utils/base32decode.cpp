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

#include "base32decode.h"
#include <assert.h>

/****************************** Base32 Decoding ******************************/

/*
 * output 5 bytes for every 8 input:
 *
 *               outputs: 1        2        3        4        5
 * inputs: 1 = ---11111 = 11111---
 *         2 = ---222XX = -----222 XX------
 *         3 = ---33333 =          --33333-
 *         4 = ---4XXXX =          -------4 XXXX----
 *         5 = ---5555X =                   ----5555 X-------
 *         6 = ---66666 =                            -66666--
 *         7 = ---77XXX =                            ------77 XXX-----
 *         8 = ---88888 =                                     ---88888
 */

static const size_t BASE32_INPUT = 8;
static const size_t BASE32_OUTPUT = 5;
static const size_t BASE32_MAX_PADDING = 6;
static const unsigned char BASE32_MAX_VALUE = 31;
static const unsigned char BASE32_TABLE[ 0x80 ] = {
  /*00-07*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  /*08-0f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  /*10-17*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  /*18-1f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  /*20-27*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  /*28-2f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  /*30-37*/ 0xFF, 0xFF, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /*6 = '2'-'7'*/
  /*38-3f*/ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0xFF, 0xFF, /*1 = '='*/
  /*40-47*/ 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /*7 = 'A'-'G'*/
  /*48-4f*/ 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, /*8 = 'H'-'O'*/
  /*50-57*/ 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, /*8 = 'P'-'W'*/
  /*58-5f*/ 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /*3 = 'X'-'Z'*/
  /*60-67*/ 0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, /*7 = 'a'-'g' (same as 'A'-'G')*/
  /*68-6f*/ 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, /*8 = 'h'-'o' (same as 'H'-'O')*/
  /*70-77*/ 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, /*8 = 'p'-'w' (same as 'P'-'W')*/
  /*78-7f*/ 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  /*3 = 'x'-'z' (same as 'X'-'Z')*/
};

int Base32Decode::Validate(const char* src, size_t srcChars)
{
  /*
     * returns 0 if the source is a valid baseXX encoding
     */

  if (!src)
    return -1; /*ERROR - NULL pointer*/

  if (srcChars % BASE32_INPUT != 0)
    return -1; /*ERROR - extra characters*/

  /* check the bytes */
  for (; srcChars >= 1; --srcChars, ++src)
  {
    unsigned char ch = *src;
    if ((ch >= 0x80) || (BASE32_TABLE[ch] > BASE32_MAX_VALUE))
      break;
  }

  /* check any padding */
  for (; 1 <= srcChars && srcChars <= BASE32_MAX_PADDING; --srcChars, ++src)
  {
    unsigned char ch = *src;
    if ((ch >= 0x80) || (BASE32_TABLE[ch] != BASE32_MAX_VALUE + 1))
      break;
  }

  /* if srcChars isn't zero then the encoded string isn't valid */
  if (srcChars != 0)
    return -2; /*ERROR - invalid baseXX character*/

  /* OK */
  return 0;
}

int Base32Decode::Validate(const wchar_t* src, size_t srcChars)
{
  /*
    * returns 0 if the source is a valid baseXX encoding
    */

  if (!src)
    return -1; /*ERROR - NULL pointer*/

  if (srcChars % BASE32_INPUT != 0)
    return -1; /*ERROR - extra characters*/

  /* check the bytes */
  for (; srcChars >= 1; --srcChars, ++src)
  {
    wchar_t ch = *src;
    if ((ch >= 0x80) || (BASE32_TABLE[ch] > BASE32_MAX_VALUE))
      break;
  }

  /* check any padding */
  for (; 1 <= srcChars && srcChars <= BASE32_MAX_PADDING; --srcChars, ++src)
  {
    wchar_t ch = *src;
    if ((ch >= 0x80) || (BASE32_TABLE[ch] != BASE32_MAX_VALUE + 1))
      break;
  }

  /* if srcChars isn't zero then the encoded string isn't valid */
  if (srcChars != 0)
    return -2; /*ERROR - invalid baseXX character*/

  /* OK */
  return 0;
}

size_t Base32Decode::GetLength(size_t srcChars)
{
  if (srcChars % BASE32_INPUT != 0)
    return 0; /*ERROR - extra characters*/

  /* OK */
  return (((srcChars + BASE32_INPUT - 1) / BASE32_INPUT) * BASE32_OUTPUT);
}

size_t Base32Decode::Decode(void* dest, const char* src, size_t srcChars)
{
  if (dest && src && (srcChars % BASE32_INPUT == 0))
  {
    const char* pSrc = src;
    unsigned char* pDest = (unsigned char*)dest;
    size_t dwSrcSize = srcChars;
    size_t dwDestSize = 0;
    unsigned char in1, in2, in3, in4, in5, in6, in7, in8;

    while (dwSrcSize >= 1)
    {
      /* 8 inputs */
      in1 = *pSrc++;
      in2 = *pSrc++;
      in3 = *pSrc++;
      in4 = *pSrc++;
      in5 = *pSrc++;
      in6 = *pSrc++;
      in7 = *pSrc++;
      in8 = *pSrc++;
      dwSrcSize -= BASE32_INPUT;

      /* Validate ASCII */
      if (in1 >= 0x80 || in2 >= 0x80 || in3 >= 0x80 || in4 >= 0x80
          || in5 >= 0x80 || in6 >= 0x80 || in7 >= 0x80 || in8 >= 0x80)
        return 0; /*ERROR - invalid base32 character*/

      /* Convert ASCII to base32 */
      in1 = BASE32_TABLE[in1];
      in2 = BASE32_TABLE[in2];
      in3 = BASE32_TABLE[in3];
      in4 = BASE32_TABLE[in4];
      in5 = BASE32_TABLE[in5];
      in6 = BASE32_TABLE[in6];
      in7 = BASE32_TABLE[in7];
      in8 = BASE32_TABLE[in8];

      /* Validate base32 */
      if (in1 > BASE32_MAX_VALUE || in2 > BASE32_MAX_VALUE)
        return 0; /*ERROR - invalid base32 character*/
      /*the following can be padding*/
      if (in3 > BASE32_MAX_VALUE + 1 || in4 > BASE32_MAX_VALUE + 1
          || in5 > BASE32_MAX_VALUE + 1 || in6 > BASE32_MAX_VALUE + 1
          || in7 > BASE32_MAX_VALUE + 1 || in8 > BASE32_MAX_VALUE + 1)
        return 0; /*ERROR - invalid base32 character*/

      /* 5 outputs */
      *pDest++ = ((in1 & 0x1f) << 3) | ((in2 & 0x1c) >> 2);
      *pDest++ = ((in2 & 0x03) << 6) | ((in3 & 0x1f) << 1) | ((in4 & 0x10) >> 4);
      *pDest++ = ((in4 & 0x0f) << 4) | ((in5 & 0x1e) >> 1);
      *pDest++ = ((in5 & 0x01) << 7) | ((in6 & 0x1f) << 2) | ((in7 & 0x18) >> 3);
      *pDest++ = ((in7 & 0x07) << 5) | (in8 & 0x1f);
      dwDestSize += BASE32_OUTPUT;

      /* Padding */
      if (in8 == BASE32_MAX_VALUE + 1)
      {
        --dwDestSize;
        assert((in7 == BASE32_MAX_VALUE + 1 && in6 == BASE32_MAX_VALUE + 1)
               || (in7 != BASE32_MAX_VALUE + 1));
        if (in6 == BASE32_MAX_VALUE + 1)
        {
          --dwDestSize;
          if (in5 == BASE32_MAX_VALUE + 1)
          {
            --dwDestSize;
            assert((in4 == BASE32_MAX_VALUE + 1 && in3 == BASE32_MAX_VALUE + 1)
                   || (in4 != BASE32_MAX_VALUE + 1));
            if (in3 == BASE32_MAX_VALUE + 1)
            {
              --dwDestSize;
            }
          }
        }
      }
    }

    return dwDestSize;
  }
  else
    return 0; /*ERROR - null pointer, or srcChars isn't a multiple of 8*/
}

size_t Base32Decode::Decode(void* dest, const wchar_t* src, size_t srcChars)
{
  if (dest && src && (srcChars % BASE32_INPUT == 0))
  {
    const wchar_t* pSrc = src;
    unsigned char* pDest = (unsigned char*)dest;
    size_t dwSrcSize = srcChars;
    size_t dwDestSize = 0;
    wchar_t in1, in2, in3, in4, in5, in6, in7, in8;

    while (dwSrcSize >= 1)
    {
      /* 8 inputs */
      in1 = *pSrc++;
      in2 = *pSrc++;
      in3 = *pSrc++;
      in4 = *pSrc++;
      in5 = *pSrc++;
      in6 = *pSrc++;
      in7 = *pSrc++;
      in8 = *pSrc++;
      dwSrcSize -= BASE32_INPUT;

      /* Validate ASCII */
      if (in1 >= 0x80 || in2 >= 0x80 || in3 >= 0x80 || in4 >= 0x80
          || in5 >= 0x80 || in6 >= 0x80 || in7 >= 0x80 || in8 >= 0x80)
        return 0; /*ERROR - invalid base32 character*/

      /* Convert ASCII to base32 */
      in1 = BASE32_TABLE[in1];
      in2 = BASE32_TABLE[in2];
      in3 = BASE32_TABLE[in3];
      in4 = BASE32_TABLE[in4];
      in5 = BASE32_TABLE[in5];
      in6 = BASE32_TABLE[in6];
      in7 = BASE32_TABLE[in7];
      in8 = BASE32_TABLE[in8];

      /* Validate base32 */
      if (in1 > BASE32_MAX_VALUE || in2 > BASE32_MAX_VALUE)
        return 0; /*ERROR - invalid base32 character*/
      /*the following can be padding*/
      if (in3 > BASE32_MAX_VALUE + 1 || in4 > BASE32_MAX_VALUE + 1
          || in5 > BASE32_MAX_VALUE + 1 || in6 > BASE32_MAX_VALUE + 1
          || in7 > BASE32_MAX_VALUE + 1 || in8 > BASE32_MAX_VALUE + 1)
        return 0; /*ERROR - invalid base32 character*/

      /* 5 outputs */
      *pDest++ = ((in1 & 0x1f) << 3) | ((in2 & 0x1c) >> 2);
      *pDest++ = ((in2 & 0x03) << 6) | ((in3 & 0x1f) << 1) | ((in4 & 0x10) >> 4);
      *pDest++ = ((in4 & 0x0f) << 4) | ((in5 & 0x1e) >> 1);
      *pDest++ = ((in5 & 0x01) << 7) | ((in6 & 0x1f) << 2) | ((in7 & 0x18) >> 3);
      *pDest++ = ((in7 & 0x07) << 5) | (in8 & 0x1f);
      dwDestSize += BASE32_OUTPUT;

      /* Padding */
      if (in8 == BASE32_MAX_VALUE + 1)
      {
        --dwDestSize;
        assert((in7 == BASE32_MAX_VALUE + 1 && in6 == BASE32_MAX_VALUE + 1)
               || (in7 != BASE32_MAX_VALUE + 1));
        if (in6 == BASE32_MAX_VALUE + 1)
        {
          --dwDestSize;
          if (in5 == BASE32_MAX_VALUE + 1)
          {
            --dwDestSize;
            assert((in4 == BASE32_MAX_VALUE + 1 && in3 == BASE32_MAX_VALUE + 1)
                   || (in4 != BASE32_MAX_VALUE + 1));
            if (in3 == BASE32_MAX_VALUE + 1)
            {
              --dwDestSize;
            }
          }
        }
      }
    }

    return dwDestSize;
  }
  else
    return 0; /*ERROR - null pointer, or srcChars isn't a multiple of 8*/
}

size_t Base32Decode::DecodeBlock(void* dest, const char* src)
{
  if (dest && src)
  {
    const char* pSrc = src;
    unsigned char* pDest = (unsigned char*)dest;
    unsigned char in1, in2, in3, in4, in5, in6, in7, in8;

    /* 8 inputs */
    in1 = *pSrc++;
    in2 = *pSrc++;
    in3 = *pSrc++;
    in4 = *pSrc++;
    in5 = *pSrc++;
    in6 = *pSrc++;
    in7 = *pSrc++;
    in8 = *pSrc++;

    /* Validate ASCII */
    if (in1 >= 0x80 || in2 >= 0x80 || in3 >= 0x80 || in4 >= 0x80
        || in5 >= 0x80 || in6 >= 0x80 || in7 >= 0x80 || in8 >= 0x80)
      return 0; /*ERROR - invalid base32 character*/

    /* Convert ASCII to base32 */
    in1 = BASE32_TABLE[in1];
    in2 = BASE32_TABLE[in2];
    in3 = BASE32_TABLE[in3];
    in4 = BASE32_TABLE[in4];
    in5 = BASE32_TABLE[in5];
    in6 = BASE32_TABLE[in6];
    in7 = BASE32_TABLE[in7];
    in8 = BASE32_TABLE[in8];

    /* Validate base32 */
    if (in1 > BASE32_MAX_VALUE || in2 > BASE32_MAX_VALUE)
      return 0; /*ERROR - invalid base32 character*/
    /*the following can be padding*/
    if (in3 > BASE32_MAX_VALUE + 1 || in4 > BASE32_MAX_VALUE + 1
        || in5 > BASE32_MAX_VALUE + 1 || in6 > BASE32_MAX_VALUE + 1
        || in7 > BASE32_MAX_VALUE + 1 || in8 > BASE32_MAX_VALUE + 1)
      return 0; /*ERROR - invalid base32 character*/

    /* 5 outputs */
    *pDest++ = ((in1 & 0x1f) << 3) | ((in2 & 0x1c) >> 2);
    *pDest++ = ((in2 & 0x03) << 6) | ((in3 & 0x1f) << 1) | ((in4 & 0x10) >> 4);
    *pDest++ = ((in4 & 0x0f) << 4) | ((in5 & 0x1e) >> 1);
    *pDest++ = ((in5 & 0x01) << 7) | ((in6 & 0x1f) << 2) | ((in7 & 0x18) >> 3);
    *pDest++ = ((in7 & 0x07) << 5) | (in8 & 0x1f);

    return BASE32_OUTPUT;
  }
  else
    return 0; /*ERROR - null pointer*/
}

size_t Base32Decode::DecodeBlock(void* dest, const wchar_t* src)
{
  if (dest && src)
  {
    const wchar_t* pSrc = src;
    unsigned char* pDest = (unsigned char*)dest;
    wchar_t in1, in2, in3, in4, in5, in6, in7, in8;

    /* 8 inputs */
    in1 = *pSrc++;
    in2 = *pSrc++;
    in3 = *pSrc++;
    in4 = *pSrc++;
    in5 = *pSrc++;
    in6 = *pSrc++;
    in7 = *pSrc++;
    in8 = *pSrc++;

    /* Validate ASCII */
    if (in1 >= 0x80 || in2 >= 0x80 || in3 >= 0x80 || in4 >= 0x80
        || in5 >= 0x80 || in6 >= 0x80 || in7 >= 0x80 || in8 >= 0x80)
      return 0; /*ERROR - invalid base32 character*/

    /* Convert ASCII to base32 */
    in1 = BASE32_TABLE[in1];
    in2 = BASE32_TABLE[in2];
    in3 = BASE32_TABLE[in3];
    in4 = BASE32_TABLE[in4];
    in5 = BASE32_TABLE[in5];
    in6 = BASE32_TABLE[in6];
    in7 = BASE32_TABLE[in7];
    in8 = BASE32_TABLE[in8];

    /* Validate base32 */
    if (in1 > BASE32_MAX_VALUE || in2 > BASE32_MAX_VALUE)
      return 0; /*ERROR - invalid base32 character*/
    /*the following can be padding*/
    if (in3 > BASE32_MAX_VALUE + 1 || in4 > BASE32_MAX_VALUE + 1
        || in5 > BASE32_MAX_VALUE + 1 || in6 > BASE32_MAX_VALUE + 1
        || in7 > BASE32_MAX_VALUE + 1 || in8 > BASE32_MAX_VALUE + 1)
      return 0; /*ERROR - invalid base32 character*/

    /* 5 outputs */
    *pDest++ = ((in1 & 0x1f) << 3) | ((in2 & 0x1c) >> 2);
    *pDest++ = ((in2 & 0x03) << 6) | ((in3 & 0x1f) << 1) | ((in4 & 0x10) >> 4);
    *pDest++ = ((in4 & 0x0f) << 4) | ((in5 & 0x1e) >> 1);
    *pDest++ = ((in5 & 0x01) << 7) | ((in6 & 0x1f) << 2) | ((in7 & 0x18) >> 3);
    *pDest++ = ((in7 & 0x07) << 5) | (in8 & 0x1f);

    return BASE32_OUTPUT;
  }
  else
    return 0; /*ERROR - null pointer*/
}
