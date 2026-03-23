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

#include <wx/longlong.h>
#include <wx/datetime.h>

typedef wxLongLong int64;
typedef std::vector<int64> wxArrayInt64;

// make int64 hashable
template<>
struct std::hash<int64> {
    size_t operator()(const int64& x) const {
        return std::hash<long>{}(x.GetHi()) ^ std::hash<unsigned long>{}(x.GetLo());
    }
};

#if (wxMAJOR_VERSION == 3 && wxMINOR_VERSION >= 1)
// wx 3.1 has implemented such hash
#else
template<>
struct std::hash<wxString>
{
    size_t operator()(const wxString& k) const
    {
        return std::hash<std::wstring>()(k.ToStdWstring());
    }
};
#endif

