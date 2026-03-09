/*******************************************************
 Copyright (C) 2014 Gabriele-V
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
#include "table/AttachmentTable.h"
#include "data/AttachmentData.h"

#include "_ModelBase.h"

class AttachmentModel : public TableFactory<AttachmentTable, AttachmentData>
{
public:
    AttachmentModel();
    ~AttachmentModel();

public:
    static AttachmentModel& instance(wxSQLite3Database* db);
    static AttachmentModel& instance();

public:
    int  find_ref_c(RefTypeN ref_type, const int64 ref_id);
    auto find_ref_data_a(RefTypeN ref_type, const int64 ref_id) -> const DataA;
    int  find_ref_last_num(RefTypeN ref_type, const int64 ref_id);
    auto find_refType_mRefId(RefTypeN ref_type) -> std::map<int64, DataA>;
    auto find_all_desc_a() -> wxArrayString;
};
