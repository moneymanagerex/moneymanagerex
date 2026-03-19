/*******************************************************
 Copyright (C) 2026 George Ef (george.a.ef@gmail.com)

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#pragma once

#include "base/types.h"
#include "_DataEnum.h"

struct Repeat
{
    static const int encoding_base = 100;

    RepeatMode m_mode;
    RepeatFreq m_freq;
    int        m_num;  // positive (> 0) or -1 (infinity)
    int        m_x;    // positive if freq is e_{in,every}_x_*; -1 (null) otherwise

    Repeat();
    Repeat(RepeatMode mode, RepeatFreq freq, int num);

    static Repeat from_row(int64 row_REPEATS, int64 row_NUMOCCURRENCES);

    void to_row(int64& row_REPEATS, int64& row_NUMOCCURRENCES) const;
    auto next_datetime(const wxDateTime dateTime, bool revese = false) -> const wxDateTime;
    void next_repeat();
};
