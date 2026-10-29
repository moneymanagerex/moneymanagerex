/*******************************************************
 Copyright (C) 2026

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

#include <vector>
#include "base/_defs.h"
#include "base/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/PlanGroupData.h"

class PlanGroupModel : public TableFactory<PlanGroupTable, PlanGroupData>
{
// -- constructor

public:
    PlanGroupModel() :
        TableFactory<PlanGroupTable, PlanGroupData>() {}
    ~PlanGroupModel() {}

    static PlanGroupModel& instance(wxSQLite3Database* db);
    static PlanGroupModel& instance();

// -- override

public:
    // override TableFactory
    virtual bool purge_id(int64 id) override;

// -- methods

public:
    // Active top-level groups, ordered for display.
    auto find_root_a() -> DataA;

    // Active direct children of a group.
    auto find_children_a(int64 group_id) -> DataA;

    // Group ids of a subtree, including group_id itself. Cycle-safe: a group
    // whose parent chain loops is visited at most once.
    auto find_subtree_id_a(int64 group_id) -> std::vector<int64>;

    // Display name including ancestors, e.g. "Watches | Omega".
    auto get_id_path_name(int64 group_id) -> const wxString;
};
