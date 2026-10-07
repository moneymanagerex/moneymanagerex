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

#include "base/_defs.h"
#include "base/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/PlanAssumptionGroupData.h"

class PlanAssumptionGroupModel : public TableFactory<PlanAssumptionGroupTable, PlanAssumptionGroupData>
{
// -- constructor

public:
    PlanAssumptionGroupModel() :
        TableFactory<PlanAssumptionGroupTable, PlanAssumptionGroupData>() {}
    ~PlanAssumptionGroupModel() {}

    static PlanAssumptionGroupModel& instance(wxSQLite3Database* db);
    static PlanAssumptionGroupModel& instance();

// -- override

public:
    // override TableFactory
    virtual bool purge_id(int64 id) override;

// -- methods

public:
    auto find_active_a() -> DataA;

    // Groups that can answer a given question, most specific first. A group
    // scoped to a ticker only offers itself to rows about that ticker; an
    // unscoped group is generic and always applies.
    auto find_applicable_a(PlanAssumptionKind kind, const wxString& scope_key) -> DataA;

    auto get_name_data_n(const wxString& name) -> const Data*;

    // The member the group currently points at. Falls back to the only member,
    // so a group with a single value works without an explicit choice.
    auto get_active_assumption_id(int64 group_id) -> int64;

    // The value of that member, or the fallback when the group is empty or its
    // active member has gone away.
    double get_active_value(int64 group_id, double fallback = 0.0);

    // As above, normalised to a 0..1 fraction for rate-like kinds.
    double get_active_rate(int64 group_id, double fallback = 0.0);

    // Point the group at one of its own members. Rejects a member that is not
    // in the group, so the active pointer can never dangle.
    bool set_active_assumption(int64 group_id, int64 assumption_id);

    // True when the group may hold this kind/scope combination.
    bool accepts(int64 group_id, PlanAssumptionKind kind, const wxString& scope_key);

    // How many plan items follow this group, so the blast radius of switching
    // the active member is visible before it is switched.
    int count_dependents(int64 group_id);

    // Detach members and dependents before the group is removed, so nothing is
    // left pointing at a group that no longer exists.
    void detach_members(int64 group_id);
};
