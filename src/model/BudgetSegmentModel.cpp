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

#include "BudgetSegmentModel.h"
#include "BudgetModel.h"

// Initialize the global BudgetSegmentModel table.
BudgetSegmentModel& BudgetSegmentModel::instance(wxSQLite3Database* db)
{
    BudgetSegmentModel& ins = Singleton<BudgetSegmentModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of BudgetSegmentModel table
BudgetSegmentModel& BudgetSegmentModel::instance()
{
    return Singleton<BudgetSegmentModel>::instance();
}

// -- override

bool BudgetSegmentModel::purge_id(int64 segment_id)
{
    bool ok = true;
    db_savepoint();

    // Budget entries must survive the loss of their segment: clearing the link
    // returns them to "applies to the whole period" rather than orphaning them.
    detach_entries(segment_id);
    ok = unsafe_remove_id(segment_id);

    db_release_savepoint();
    return ok;
}

// -- methods

BudgetSegmentModel::DataA BudgetSegmentModel::find_period_a(int64 bp_id)
{
    DataA a = find_data_a(
        BudgetSegmentCol::WHERE_BUDGETYEARID(OP_EQ, bp_id)
    );

    DataA active;
    for (const auto& seg : a) {
        if (seg.m_active)
            active.push_back(seg);
    }
    std::sort(active.begin(), active.end(), Data::SorterBySORTORDER());

    return active;
}

bool BudgetSegmentModel::create_default_halves(int64 bp_id)
{
    if (!find_period_a(bp_id).empty())
        return false;

    db_savepoint();

    Data first;
    first.m_period_id = bp_id;
    first.m_name      = _t("First half");
    first.m_start_day = 1;
    first.m_end_day    = 15;
    first.m_sort_order = 0;
    first.m_active     = true;
    add_data_n(first);

    Data second;
    second.m_period_id  = bp_id;
    second.m_name       = _t("Second half");
    second.m_start_day  = 16;
    // 31 is interpreted as "end of month", so February stays correct
    second.m_end_day    = 31;
    second.m_sort_order = 1;
    second.m_active     = true;
    add_data_n(second);

    db_release_savepoint();
    return true;
}

void BudgetSegmentModel::detach_entries(int64 segment_id)
{
    BudgetModel& bm = BudgetModel::instance();

    BudgetModel::DataA entries = bm.find_data_a(
        BudgetCol::WHERE_SEGMENTID(OP_EQ, segment_id)
    );

    for (auto& entry : entries) {
        entry.m_segment_id = -1;
        bm.save_data_n(entry);
    }
}
