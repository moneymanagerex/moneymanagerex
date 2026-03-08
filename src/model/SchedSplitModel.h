/*******************************************************
 Copyright (C) 2013,2014 James Higley

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

#include "table/SchedSplitTable.h"
#include "data/_DataEnum.h"
#include "data/SchedSplitData.h"

#include "_ModelBase.h"

class SchedSplitModel : public TableFactory<SchedSplitTable, SchedSplitData>
{
public:
    static const RefTypeN s_ref_type;

public:
    SchedSplitModel();
    ~SchedSplitModel();

public:
    static SchedSplitModel& instance(wxSQLite3Database* db);
    static SchedSplitModel& instance();

public:
    bool purge_id(int64 qp_id) override;

    auto get_data_amount(const DataA& qp_a) -> double;
    auto find_all_mSchedId() -> std::map<int64, DataA>;
    int  update(int64 dst_sched_id, DataA& src_qp_a);
};

