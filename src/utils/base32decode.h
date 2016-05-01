/*
 * This file is part of QRK - Qt Registrier Kasse
 *
 * Copyright (C) 2015-2016 Christian Kvasny <chris@ckvsoft.at>
 *
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
 *
 * Button Design, and Idea for the Layout are lean out from LillePOS, Copyright 2010, Martin Koller, kollix@aon.at
 *
*/

#ifndef BASE32DECODE_H
#define BASE32DECODE_H

#include <stdlib.h>

class Base32Decode
{
  public:
    static size_t GetLength(size_t srcChars);
    static size_t Decode(void* dest, const char* src, size_t srcChars);
    static size_t Decode(void* dest, const wchar_t* src, size_t srcChars);
    int Validate(const char* src, size_t srcChars);
    int Validate(const wchar_t* src, size_t srcChars);
    size_t DecodeBlock(void* dest, const char* src);
    size_t DecodeBlock(void* dest, const wchar_t* src);
};

#endif // BASE32DECODE_H
