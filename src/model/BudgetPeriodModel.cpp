/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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

#include "BudgetPeriodModel.h"
#include "BudgetModel.h"

BudgetPeriodModel::BudgetPeriodModel() :
    TableFactory<BudgetPeriodTable, BudgetPeriodData>()
{
}

BudgetPeriodModel::~BudgetPeriodModel()
{
}

// Initialize the global BudgetPeriodModel table.
// Reset the BudgetPeriodModel table or create the table if it does not exist.
BudgetPeriodModel& BudgetPeriodModel::instance(wxSQLite3Database* db)
{
    BudgetPeriodModel& ins = Singleton<BudgetPeriodModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of BudgetPeriodModel table
BudgetPeriodModel& BudgetPeriodModel::instance()
{
    return Singleton<BudgetPeriodModel>::instance();
}

bool BudgetPeriodModel::purge_id(int64 bp_id)
{
    for (const BudgetData& budget_d : BudgetModel::instance().find(
        BudgetCol::BUDGETYEARID(bp_id)
    ))
        BudgetModel::instance().purge_id(budget_d.m_period_id);
    return unsafe_remove_id(bp_id);
}

const wxString BudgetPeriodModel::get_id_name_n(int64 bp_id)
{
    const Data* bp_n = get_id_data_n(bp_id);
    return bp_n ? bp_n->m_name : "";
}

int64 BudgetPeriodModel::get_name_id_n(const wxString& bp_name)
{
    // TODO: lookup bp_name in cache
    for (const auto& bp_d : find_all()) {
        if (bp_d.m_name == bp_name)
            return bp_d.m_id;
    }
    return -1;
}

int64 BudgetPeriodModel::ensure_name(const wxString& bp_name)
{
    int64 bp_id_n = get_name_id_n(bp_name);
    if (bp_id_n < 0) {
        Data new_bp_d = Data();
        new_bp_d.m_name = bp_name;
        add_data_n(new_bp_d);
        bp_id_n = new_bp_d.id();
    }
    return bp_id_n;
}
