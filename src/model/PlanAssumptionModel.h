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
#include "data/PlanAssumptionData.h"

class PlanAssumptionModel : public TableFactory<PlanAssumptionTable, PlanAssumptionData>
{
// -- static

public:
    static auto WHERE_KIND(OP op, PlanAssumptionKind kind) -> TableClauseV<wxString>;

// -- constructor

public:
    PlanAssumptionModel() :
        TableFactory<PlanAssumptionTable, PlanAssumptionData>() {}
    ~PlanAssumptionModel() {}

    static PlanAssumptionModel& instance(wxSQLite3Database* db);
    static PlanAssumptionModel& instance();

// -- override

public:
    // override TableFactory
    virtual bool purge_id(int64 id) override;

// -- methods

public:
    // All active assumptions, ordered by name.
    auto find_active_a() -> DataA;

    // Look up by name (case-insensitive) or by kind + scope, e.g. the share
    // price assumption for "MSFT".
    auto get_name_data_n(const wxString& name) -> const Data*;
    auto get_scope_data_n(PlanAssumptionKind kind, const wxString& scope_key) -> const Data*;

    // Members of an assumption group, ordered by name.
    auto find_group_a(int64 group_id) -> DataA;

    // Whether an assumption answers a given question: the kind must match, and
    // a scoped assumption only applies to its own scope. An unscoped one is
    // generic and applies anywhere.
    bool applies_to(const Data& a, PlanAssumptionKind kind, const wxString& scope_key);
    bool id_applies_to(int64 assumption_id, PlanAssumptionKind kind, const wxString& scope_key);

    // Assumptions that may be offered for a kind + scope, most specific first.
    auto find_applicable_a(PlanAssumptionKind kind, const wxString& scope_key) -> DataA;

    // A name that does not collide with an existing one, for duplication.
    auto make_unique_name(const wxString& base) -> const wxString;

    // Resolved value, or `fallback` when the assumption is missing/inactive.
    double get_value(int64 assumption_id, double fallback = 0.0);
    // Resolved rate normalised to a 0..1 fraction.
    double get_rate(int64 assumption_id, double fallback = 0.0);

    // How many active plan items depend on an assumption. Used to warn before
    // deleting one, and to show the blast radius of changing it.
    std::size_t count_dependents(int64 assumption_id);

    // Detach an assumption from every item that references it, so items keep
    // their last resolved values instead of silently falling back to zero.
    void detach_dependents(int64 assumption_id);
};
