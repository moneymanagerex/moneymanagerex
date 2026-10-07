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

#include <algorithm>
#include "base/_defs.h"

#include "PlanAssumptionModel.h"
#include "PlanItemModel.h"

// -- static

TableClauseV<wxString> PlanAssumptionModel::WHERE_KIND(OP op, PlanAssumptionKind kind)
{
    return PlanAssumptionCol::WHERE_KIND(op, kind.key());
}

// -- constructor

// Initialize the global PlanAssumptionModel table.
PlanAssumptionModel& PlanAssumptionModel::instance(wxSQLite3Database* db)
{
    PlanAssumptionModel& ins = Singleton<PlanAssumptionModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of PlanAssumptionModel table
PlanAssumptionModel& PlanAssumptionModel::instance()
{
    return Singleton<PlanAssumptionModel>::instance();
}

// -- override

bool PlanAssumptionModel::purge_id(int64 assumption_id)
{
    bool ok = true;
    db_savepoint();

    // Items keep the values they last resolved to; removing the assumption
    // must not silently rewrite them to zero.
    detach_dependents(assumption_id);
    ok = unsafe_remove_id(assumption_id);

    db_release_savepoint();
    return ok;
}

// -- methods

PlanAssumptionModel::DataA PlanAssumptionModel::find_active_a()
{
    DataA out;
    for (const auto& a : find_data_a()) {
        if (a.m_active)
            out.push_back(a);
    }
    std::sort(out.begin(), out.end(), Data::SorterByASSUMPTIONNAME());

    return out;
}

const PlanAssumptionData* PlanAssumptionModel::get_name_data_n(const wxString& name)
{
    if (name.IsEmpty())
        return nullptr;

    for (const auto& a : find_data_a()) {
        if (a.m_active && a.m_name.IsSameAs(name, false))
            return get_idN_data_n(a.m_id);
    }
    return nullptr;
}

const PlanAssumptionData* PlanAssumptionModel::get_scope_data_n(
    PlanAssumptionKind kind,
    const wxString& scope_key
) {
    if (scope_key.IsEmpty())
        return nullptr;

    for (const auto& a : find_data_a()) {
        if (!a.m_active)
            continue;
        if (a.m_kind.id() != kind.id())
            continue;
        if (a.m_scope_key.IsSameAs(scope_key, false))
            return get_idN_data_n(a.m_id);
    }
    return nullptr;
}

PlanAssumptionModel::DataA PlanAssumptionModel::find_group_a(int64 group_id)
{
    DataA out;
    if (group_id <= 0)
        return out;

    for (const auto& a : find_data_a()) {
        if (a.m_active && a.m_group_id == group_id)
            out.push_back(a);
    }
    std::sort(out.begin(), out.end(), Data::SorterByASSUMPTIONNAME());

    return out;
}

const wxString PlanAssumptionModel::make_unique_name(const wxString& base)
{
    // Names are UNIQUE in the schema, so a duplicate needs a name of its own
    // rather than failing the insert.
    if (!get_name_data_n(base))
        return base;

    for (int n = 2; n < 1000; ++n) {
        const wxString candidate = wxString::Format("%s (%d)", base, n);
        if (!get_name_data_n(candidate))
            return candidate;
    }
    return wxString::Format("%s (%lld)", base, static_cast<long long>(wxDateTime::Now().GetTicks()));
}

bool PlanAssumptionModel::applies_to(
    const Data& a,
    PlanAssumptionKind kind,
    const wxString& scope_key
) {
    if (!a.m_active)
        return false;

    // An assumption answers exactly one question: a share price is not a tax
    // rate, so the kinds must agree before anything else is considered.
    if (a.m_kind.id() != kind.id())
        return false;

    // An unscoped assumption is deliberately generic - "my tax rate" - and
    // applies anywhere. A scoped one names what it is about (a ticker, say)
    // and must only be offered to rows about that same thing.
    if (a.m_scope_key.IsEmpty())
        return true;

    return a.m_scope_key.IsSameAs(scope_key, false);
}

bool PlanAssumptionModel::id_applies_to(
    int64 assumption_id,
    PlanAssumptionKind kind,
    const wxString& scope_key
) {
    if (assumption_id <= 0)
        return false;

    const Data* a_n = get_idN_data_n(assumption_id);
    return a_n && applies_to(*a_n, kind, scope_key);
}

PlanAssumptionModel::DataA PlanAssumptionModel::find_applicable_a(
    PlanAssumptionKind kind,
    const wxString& scope_key
) {
    DataA out;
    for (const auto& a : find_data_a()) {
        if (applies_to(a, kind, scope_key))
            out.push_back(a);
    }

    // Scoped assumptions first: when a row names a ticker, the assumption for
    // that ticker is the one being looked for, not the generic fallback.
    std::sort(out.begin(), out.end(),
        [](const Data& x, const Data& y) {
            const bool xs = !x.m_scope_key.IsEmpty();
            const bool ys = !y.m_scope_key.IsEmpty();
            if (xs != ys)
                return xs;
            return x.m_name.CmpNoCase(y.m_name) < 0;
        });

    return out;
}

double PlanAssumptionModel::get_value(int64 assumption_id, double fallback)
{
    if (assumption_id <= 0)
        return fallback;

    const Data* a_n = get_idN_data_n(assumption_id);
    if (!a_n || !a_n->m_active)
        return fallback;

    return a_n->m_value;
}

double PlanAssumptionModel::get_rate(int64 assumption_id, double fallback)
{
    if (assumption_id <= 0)
        return fallback;

    const Data* a_n = get_idN_data_n(assumption_id);
    if (!a_n || !a_n->m_active)
        return fallback;

    return a_n->as_rate();
}

std::size_t PlanAssumptionModel::count_dependents(int64 assumption_id)
{
    if (assumption_id <= 0)
        return 0;

    std::size_t n = 0;
    for (const auto& item : PlanItemModel::instance().find_data_a()) {
        if (!item.m_active)
            continue;
        if (item.m_price_assumption_id == assumption_id ||
            item.m_tax_assumption_id == assumption_id)
            ++n;
    }
    return n;
}

void PlanAssumptionModel::detach_dependents(int64 assumption_id)
{
    if (assumption_id <= 0)
        return;

    PlanItemModel& pim = PlanItemModel::instance();

    for (auto& item : pim.find_data_a()) {
        bool changed = false;

        if (item.m_price_assumption_id == assumption_id) {
            // Freeze the value that was in force so the plan does not move.
            if (item.m_unit_price <= 0.0)
                item.m_unit_price = get_value(assumption_id, item.m_unit_price);
            item.m_price_assumption_id = -1;
            changed = true;
        }
        if (item.m_tax_assumption_id == assumption_id) {
            if (item.m_tax_rate <= 0.0)
                item.m_tax_rate = get_rate(assumption_id, item.m_tax_rate);
            item.m_tax_assumption_id = -1;
            changed = true;
        }

        if (changed)
            pim.save_data_n(item);
    }
}
