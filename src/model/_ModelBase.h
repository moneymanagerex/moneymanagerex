/*******************************************************
Copyright (C) 2013 - 2018 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2018 Stefano Giorgio (stef145g)

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

#include "base/defs.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <wx/datetime.h>
#include <wx/log.h>

#include "util/mmSingleton.h"
#include "table/_TableFactory.h"

typedef wxDateTime wxDate;
typedef std::vector<int64> wxArrayInt64;

#if (wxMAJOR_VERSION == 3 && wxMINOR_VERSION >= 1)
// wx 3.1 has implemented such hash
#else
namespace std
{
    template<>
    struct hash<wxString>
    {
        size_t operator()(const wxString& k) const
        {
            return std::hash<std::wstring>()(k.ToStdWstring());
        }
    };
}
#endif

class ModelBase
{
public:
    ModelBase() {};
    ~ModelBase() {};
};
