/*******************************************************
 Copyright (C) 2013,2014 James Higley
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "SchedSplitModel.h"
#include "AttachmentModel.h"
#include "TagLinkModel.h"

const RefTypeN SchedSplitModel::s_ref_type = RefTypeN(RefTypeN::e_sched_split);

SchedSplitModel::SchedSplitModel() :
    TableFactory<SchedSplitTable, SchedSplitData>()
{
}

SchedSplitModel::~SchedSplitModel()
{
}

// Initialize the global SchedSplitModel table.
// Reset the SchedSplitModel table or create the table if it does not exist.
SchedSplitModel& SchedSplitModel::instance(wxSQLite3Database* db)
{
    SchedSplitModel& ins = Singleton<SchedSplitModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of SchedSplitModel table
SchedSplitModel& SchedSplitModel::instance()
{
    return Singleton<SchedSplitModel>::instance();
}

bool SchedSplitModel::purge_id(int64 qp_id)
{
    TagLinkModel::instance().purge_ref(s_ref_type, qp_id);
    return unsafe_remove_id(qp_id);
}

double SchedSplitModel::get_data_amount(const DataA& qp_a)
{
    double amount = 0.0;
    for (const Data& qp_d : qp_a)
        amount += qp_d.m_amount;
    return amount;
}

std::map<int64, SchedSplitModel::DataA> SchedSplitModel::find_all_mSchedId()
{
    std::map<int64, SchedSplitModel::DataA> schedId_qpA_m;
    for (const auto& qp_d : find_all()) {
        schedId_qpA_m[qp_d.m_sched_id].push_back(qp_d);
    }
    return schedId_qpA_m;
}

int SchedSplitModel::update(int64 dst_sched_id, DataA& src_qp_a)
{

    for (const auto& qp_d : find(
        SchedSplitCol::TRANSID(dst_sched_id)
    )) {
        instance().purge_id(qp_d.m_id);
    }

    if (!src_qp_a.empty()) {
        DataA new_qp_a;
        for (const auto& src_qp_d : src_qp_a) {
            Data new_qp_d = Data();
            new_qp_d.m_sched_id    = dst_sched_id;
            new_qp_d.m_amount      = src_qp_d.m_amount;
            new_qp_d.m_category_id = src_qp_d.m_category_id;
            new_qp_d.m_notes       = src_qp_d.m_notes;
            new_qp_a.push_back(new_qp_d);
        }
        instance().save_data_a(new_qp_a);

        // Send back the new m_id which is needed to update taglinks
        // CHECK: src_qp_a.at(i).m_sched_id is not updated
        for (int i = 0; i < static_cast<int>(src_qp_a.size()); i++)
            src_qp_a.at(i).m_id = new_qp_a.at(i).m_id;
    }
    return src_qp_a.size();
}
