/*******************************************************
 Copyright (C) 2016 Gabriele-V

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
#include "data/FieldValueData.h"

class FieldValueModel : public TableFactory<FieldValueTable, FieldValueData>
{
public:
    FieldValueModel();
    ~FieldValueModel();

public:
    static FieldValueModel& instance(wxSQLite3Database* db);
    static FieldValueModel& instance();

    static FieldValueCol::REFID REFTYPEID(RefTypeN ref_type, int64 ref_id);

public:
    bool purge_ref(RefTypeN ref_type, int64 ref_id);
    auto get_key_data_n(int64 field_id, RefTypeN ref_type, int64 ref_id) -> const Data*;
    auto find_refType_mRefId(RefTypeN ref_type) -> std::map<int64, DataA>;
};

