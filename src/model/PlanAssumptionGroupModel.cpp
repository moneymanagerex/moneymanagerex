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

#include "PlanAssumptionGroupModel.h"
#include "PlanAssumptionModel.h"
#include "PlanItemModel.h"

// -- constructor

// Initialize the global PlanAssumptionGroupModel table.
PlanAssumptionGroupModel& PlanAssumptionGroupModel::instance(wxSQLite3Database* db)
{
    PlanAssumptionGroupModel& ins = Singleton<PlanAssumptionGroupModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of PlanAssumptionGroupModel table
PlanAssumptionGroupModel& PlanAssumptionGroupModel::instance()
{
    return Singleton<PlanAssumptionGroupModel>::instance();
}

// -- override

bool PlanAssumptionGroupModel::purge_id(int64 group_id)
{
    bool ok = true;
    db_savepoint();

    detach_members(group_id);
    ok = unsafe_remove_id(group_id);

    db_release_savepoint();
    return ok;
}

// -- methods

PlanAssumptionGroupModel::DataA PlanAssumptionGroupModel::find_active_a()
{
    DataA out;
    for (const auto& g : find_data_a()) {
        if (g.m_active)
            out.push_back(g);
    }
    std::sort(out.begin(), out.end(), Data::SorterByGROUPNAME());

    return out;
}

PlanAssumptionGroupModel::DataA PlanAssumptionGroupModel::find_applicable_a(
    PlanAssumptionKind kind,
    const wxString& scope_key
) {
    DataA out;
    for (const auto& g : find_data_a()) {
        if (g.m_active && g.accepts(kind, scope_key))
            out.push_back(g);
    }

    // A group named for this exact ticker is what the user is looking for; a
    // generic one is only a fallback, so it sorts after.
    std::sort(out.begin(), out.end(),
        [](const Data& x, const Data& y) {
            if (x.is_scoped() != y.is_scoped())
                return x.is_scoped();
            return x.m_name.CmpNoCase(y.m_name) < 0;
        });

    return out;
}

const PlanAssumptionGroupData* PlanAssumptionGroupModel::get_name_data_n(const wxString& name)
{
    if (name.IsEmpty())
        return nullptr;

    for (const auto& g : find_data_a()) {
        if (g.m_active && g.m_name.IsSameAs(name, false))
            return get_idN_data_n(g.m_id);
    }
    return nullptr;
}

int64 PlanAssumptionGroupModel::get_active_assumption_id(int64 group_id)
{
    if (group_id <= 0)
        return -1;

    const Data* g_n = get_idN_data_n(group_id);
    if (!g_n || !g_n->m_active)
        return -1;

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();

    // Honour the explicit choice, but only while that member still belongs to
    // the group and is itself active.
    if (g_n->m_active_assumption_id > 0) {
        const PlanAssumptionData* a_n = pam.get_idN_data_n(g_n->m_active_assumption_id);
        if (a_n && a_n->m_active && a_n->m_group_id == group_id)
            return a_n->m_id;
    }

    // No usable choice recorded: a single-member group has an obvious answer.
    const PlanAssumptionModel::DataA members = pam.find_group_a(group_id);
    if (members.size() == 1)
        return members[0].m_id;

    return -1;
}

double PlanAssumptionGroupModel::get_active_value(int64 group_id, double fallback)
{
    const int64 id = get_active_assumption_id(group_id);
    if (id <= 0)
        return fallback;

    return PlanAssumptionModel::instance().get_value(id, fallback);
}

double PlanAssumptionGroupModel::get_active_rate(int64 group_id, double fallback)
{
    const int64 id = get_active_assumption_id(group_id);
    if (id <= 0)
        return fallback;

    return PlanAssumptionModel::instance().get_rate(id, fallback);
}

bool PlanAssumptionGroupModel::set_active_assumption(int64 group_id, int64 assumption_id)
{
    if (group_id <= 0)
        return false;

    const Data* g_n = get_idN_data_n(group_id);
    if (!g_n)
        return false;

    Data g = *g_n;

    // Clearing the choice is legitimate; setting it is not unless the member
    // really is in this group, otherwise the pointer would dangle.
    if (assumption_id > 0) {
        const PlanAssumptionData* a_n =
            PlanAssumptionModel::instance().get_idN_data_n(assumption_id);
        if (!a_n || a_n->m_group_id != group_id)
            return false;
    }

    g.m_active_assumption_id = (assumption_id > 0) ? assumption_id : -1;
    save_data_n(g);

    return true;
}

bool PlanAssumptionGroupModel::accepts(
    int64 group_id,
    PlanAssumptionKind kind,
    const wxString& scope_key
) {
    if (group_id <= 0)
        return false;

    const Data* g_n = get_idN_data_n(group_id);
    return g_n && g_n->m_active && g_n->accepts(kind, scope_key);
}

int PlanAssumptionGroupModel::count_dependents(int64 group_id)
{
    if (group_id <= 0)
        return 0;

    int n = 0;
    for (const auto& item : PlanItemModel::instance().find_data_a()) {
        if (!item.m_active)
            continue;
        if (item.m_price_assumption_group_id == group_id ||
            item.m_tax_assumption_group_id == group_id)
            ++n;
    }
    return n;
}

void PlanAssumptionGroupModel::detach_members(int64 group_id)
{
    if (group_id <= 0)
        return;

    PlanAssumptionModel& pam = PlanAssumptionModel::instance();
    PlanItemModel& pim = PlanItemModel::instance();

    // Members outlive the group: they simply become standalone assumptions
    // rather than being deleted along with it.
    for (auto& a : pam.find_group_a(group_id)) {
        a.m_group_id = -1;
        pam.save_data_n(a);
    }

    // Items lose the link, but keep the value that was in force, so removing a
    // group never silently rewrites the plan to zero.
    for (auto& item : pim.find_data_a()) {
        bool changed = false;

        if (item.m_price_assumption_group_id == group_id) {
            if (item.m_unit_price <= 0.0)
                item.m_unit_price = get_active_value(group_id, item.m_unit_price);
            item.m_price_assumption_group_id = -1;
            changed = true;
        }
        if (item.m_tax_assumption_group_id == group_id) {
            if (item.m_tax_rate <= 0.0)
                item.m_tax_rate = get_active_rate(group_id, item.m_tax_rate);
            item.m_tax_assumption_group_id = -1;
            changed = true;
        }

        if (changed)
            pim.save_data_n(item);
    }
}
