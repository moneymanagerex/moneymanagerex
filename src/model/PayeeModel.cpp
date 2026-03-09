/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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

#include "PayeeModel.h"
#include "AttachmentModel.h"
#include "TrxModel.h"
#include "SchedModel.h"

const RefTypeN PayeeModel::s_ref_type = RefTypeN(RefTypeN::e_payee);

PayeeModel::PayeeModel() :
    TableFactory<PayeeTable, PayeeData>()
{
}

PayeeModel::~PayeeModel()
{
}

// Initialize the global PayeeModel table.
// Reset the PayeeModel table or create the table if it does not exist.
PayeeModel& PayeeModel::instance(wxSQLite3Database* db)
{
    PayeeModel& ins = Singleton<PayeeModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();
    ins.preload_cache();

    return ins;
}

// Return the static instance of PayeeModel table
PayeeModel& PayeeModel::instance()
{
    return Singleton<PayeeModel>::instance();
}

int PayeeModel::find_id_aux_c(int64 payee_id)
{
    return AttachmentModel::instance().find_ref_c(s_ref_type, payee_id);
}

int PayeeModel::find_id_dep_c(int64 payee_id)
{
    // FIX (2026-03-01): Do not exclude deleted transactions. Deleted transactions
    // are shown in a panel and they can be restored; they must have a valid payee id.
    int cnt_trx = TrxModel::instance().find(
        TrxCol::PAYEEID(payee_id)
    ).size();

    int cnt_sched = SchedModel::instance().find(
        SchedCol::PAYEEID(payee_id)
    ).size();

    return cnt_trx + cnt_sched;
}

bool PayeeModel::purge_id(int64 payee_id)
{
    if (PayeeModel::find_id_dep_c(payee_id) > 0)
        return false;

    // FIXME: remove AttachmentData owned by payee_id

    return unsafe_remove_id(payee_id);
}

const wxString PayeeModel::get_id_name(int64 payee_id)
{
    const Data* payee_n = get_id_data_n(payee_id);
    if (payee_n)
        return payee_n->m_name;
    else
        return _t("Payee Error");
}

const PayeeData* PayeeModel::get_name_data_n(const wxString& name)
{
    const Data* payee_n = search_cache_n(PayeeCol::PAYEENAME(name));
    if (payee_n)
        return payee_n;

    DataA payee_a = find(PayeeCol::PAYEENAME(name));
    if (!payee_a.empty())
        payee_n = get_id_data_n(payee_a[0].m_id);
    return payee_n;
}

const wxArrayString PayeeModel::find_all_name_a()
{
    wxArrayString name_a;
    for (const auto& payee_d : find_all(Col::COL_ID_PAYEENAME)) {
        name_a.Add(payee_d.m_name);
    }
    return name_a;
}

const std::map<wxString, int64> PayeeModel::find_all_name_id_m(bool only_active)
{
    std::map<wxString, int64> name_id_m;
    for (const auto& payee_d : find_all()) {
        if (only_active && !payee_d.m_active)
            continue;
        name_id_m[payee_d.m_name] = payee_d.m_id;
    }
    return name_id_m;
}

const std::set<int64> PayeeModel::find_used_id_s()
{
    std::set<int64> used_id_s;
    for (const auto& trx_d : TrxModel::instance().find_all()) {
        used_id_s.insert(trx_d.m_payee_id_n);
    }
    for (const auto& sched_d : SchedModel::instance().find_all()) {
        used_id_s.insert(sched_d.m_payee_id_n);
    }
    return used_id_s;
}

const PayeeModel::DataA PayeeModel::find_pattern_data_a(
    const wxString& pattern,
    bool only_active
) {
    const wxString pattern_lower = pattern.Lower().Append("*");
    DataA payee_a;
    for (const Data& payee_d : find_all(PayeeCol::COL_ID_PAYEENAME)) {
        if (only_active && !payee_d.m_active)
            continue;
        if (payee_d.m_name.Lower().Matches(pattern_lower)) {
            payee_a.push_back(payee_d);
        }
    }
    return payee_a;
}

