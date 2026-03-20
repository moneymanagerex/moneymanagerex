/*******************************************************
 Copyright (C) 2016 Guan Lisheng (guanlisheng@gmail.com)
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

#include "base/defs.h"
#include "util/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/TagLinkData.h"

class TagLinkModel : public TableFactory<TagLinkTable, TagLinkData>
{
public:
    TagLinkModel();
    ~TagLinkModel();

public:
    static TagLinkModel& instance(wxSQLite3Database* db);
    static TagLinkModel& instance();

public:
    void purge_ref(RefTypeN ref_type, int64 ref_id);
    int  update(RefTypeN ref_type, int64 ref_id, const DataA& src_gl_a);

    auto get_key_data_n(int64 tag_id, RefTypeN ref_type, int64 ref_id) -> const Data*;
    auto find_ref_mTagName(RefTypeN ref_type, int64 ref_id) -> std::map<wxString, int64>;
    auto find_refType_mRefId(RefTypeN ref_type) -> std::map<int64, DataA>;
};
