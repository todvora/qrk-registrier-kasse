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

#ifndef BASE32ENCODE_H
#define BASE32ENCODE_H

#include <stdlib.h>

class Base32Encode
{
  public:
    static size_t GetLength(size_t srcBytes);
    static size_t Encode(char* dest, const void* src, size_t srcBytes);
    static size_t Encode(wchar_t* dest, const void* src, size_t srcBytes);
    size_t EncodeBlock(char* dest, const void* src);
    size_t EncodeBlock(wchar_t* dest, const void* src);
};

#endif // BASE32ENCODE_H
