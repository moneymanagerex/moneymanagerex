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
#include "TransactionModel.h" // detect whether the payee is used or not
#include "ScheduledModel.h"

PayeeModel::PayeeModel()
: Model<PayeeTable>()
{
}

PayeeModel::~PayeeModel()
{
}

/**
* Initialize the global PayeeModel table.
* Reset the PayeeModel table or create the table if it does not exist.
*/
PayeeModel& PayeeModel::instance(wxSQLite3Database* db)
{
    PayeeModel& ins = Singleton<PayeeModel>::instance();
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();
    ins.preload();

    return ins;
}

/** Return the static instance of PayeeModel table */
PayeeModel& PayeeModel::instance()
{
    return Singleton<PayeeModel>::instance();
}

const PayeeModel::Data_Set PayeeModel::FilterPayees(const wxString& payee_pattern, bool includeInActive)
{
    Data_Set payees;
    for (auto &payee : this->get_all(PayeeModel::COL_PAYEENAME))
    {
        if (payee.PAYEENAME.Lower().Matches(payee_pattern.Lower().Append("*")) &&
            (includeInActive || payee.ACTIVE == 1)) {
            payees.push_back(payee);
            }
    }
    return payees;
}

PayeeModel::Data* PayeeModel::cache_key(const wxString& name)
{
    Data* payee = this->search_cache(PAYEENAME(name));
    if (payee)
        return payee;

    Data_Set items = this->find(PAYEENAME(name));
    if (!items.empty())
        payee = this->cache_id(items[0].PAYEEID);
    return payee;
}

wxString PayeeModel::get_payee_name(int64 payee_id)
{
    Data* payee = instance().cache_id(payee_id);
    if (payee)
        return payee->PAYEENAME;
    else
        return _t("Payee Error");
}

bool PayeeModel::remove(int64 id)
{
    if (is_used(id)) return false;
    return this->remove(id);
}

const wxArrayString PayeeModel::all_payee_names()
{
    wxArrayString payees;
    for (const auto &payee: this->get_all(COL_PAYEENAME))
    {
        payees.Add(payee.PAYEENAME);
    }
    return payees;
}

const std::map<wxString, int64> PayeeModel::all_payees(bool excludeHidden)
{
    std::map<wxString, int64> payees;
    for (const auto& payee : this->get_all())
    {
        if (!excludeHidden || (payee.ACTIVE == 1))
            payees[payee.PAYEENAME] = payee.PAYEEID;
    }
    return payees;
}

const std::map<wxString, int64> PayeeModel::used_payee()
{
    std::map<int64, wxString> cache;
    for (const auto& p : get_all())
        cache[p.PAYEEID] = p.PAYEENAME;

    std::map<wxString, int64> payees;
    for (const auto &t : TransactionModel::instance().get_all())
    {
        if (cache.count(t.PAYEEID) > 0)
            payees[cache[t.PAYEEID]] = t.PAYEEID;
    }
    for (const auto& b : ScheduledModel::instance().get_all())
    {
        if (cache.count(b.PAYEEID) > 0)
            payees[cache[b.PAYEEID]] = b.PAYEEID;
    }
    return payees;
}

// -- Check if Payee should be made available for use

bool PayeeModel::is_hidden(int64 id)
{
    const auto payee = PayeeModel::instance().cache_id(id);
    if (payee && payee->ACTIVE == 0)
        return true;

    return false;
}

bool PayeeModel::is_hidden(const Data* record)
{
    return is_hidden(record->PAYEEID);
}

bool PayeeModel::is_hidden(const Data& record)
{
    return is_hidden(&record);
}

// -- Check if Payee if being used

bool PayeeModel::is_used(int64 id)
{
    const auto &trans = TransactionModel::instance().find(TransactionModel::PAYEEID(id));
    if (!trans.empty())
    {
        for (const auto& txn : trans)
            if (txn.DELETEDTIME.IsEmpty())
                return true;
    }
    const auto &bills = ScheduledModel::instance().find(ScheduledModel::PAYEEID(id));
    if (!bills.empty()) return true;

    return false;
}

bool PayeeModel::is_used(const Data* record)
{
    return is_used(record->PAYEEID);
}

bool PayeeModel::is_used(const Data& record)
{
    return is_used(&record);
}
