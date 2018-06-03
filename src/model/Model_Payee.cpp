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

#include "Model_Payee.h"
#include "Model_Checking.h" // detect whether the payee is used or not
#include "Model_Billsdeposits.h"

Model_Payee::Model_Payee()
: Model<DB_Table_PAYEE>()
{
}

Model_Payee::~Model_Payee()
{
}

/**
* Initialize the global Model_Payee table.
* Reset the Model_Payee table or create the table if it does not exist.
*/
Model_Payee& Model_Payee::instance(wxSQLite3Database* db)
{
    Model_Payee& ins = Singleton<Model_Payee>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();

    return ins;
}

/** Return the static instance of Model_Payee table */
Model_Payee& Model_Payee::instance()
{
    return Singleton<Model_Payee>::instance();
}

const Model_Payee::Data_Set Model_Payee::FilterPayees(const wxString& payee_pattern)
{
    Data_Set payees;
    for (auto &payee : this->all(Model_Payee::COL_PAYEENAME))
    {
        if (payee.PAYEENAME.Lower().Matches(payee_pattern.Lower().Append("*")))
            payees.push_back(payee);
    }
    return payees;
}

Model_Payee::Data* Model_Payee::get(const wxString& name)
{
    Data* payee = this->get_one(PAYEENAME(name));
    if (payee) return payee;

    Data_Set items = this->find(PAYEENAME(name));
    if (!items.empty()) payee = this->get(items[0].PAYEEID, this->db_);
    return payee;
}

wxString Model_Payee::get_payee_name(int payee_id)
{
    Data* payee = instance().get(payee_id);
    if (payee)
        return payee->PAYEENAME;
    else
        return _("Payee Error");
}

bool Model_Payee::remove(int id)
{
    if (is_used(id)) return false;
    return this->remove(id, db_);
}

const wxArrayString Model_Payee::all_payee_names()
{
    wxArrayString payees;
    for (const auto &payee: this->all(COL_PAYEENAME))
    {
        payees.Add(payee.PAYEENAME);
    }
    return payees;
}

const wxArrayString Model_Payee::used_payee_names()
{
    wxArrayString payees;
    for (const auto &payee: this->all(COL_PAYEENAME))
    {
        if (is_used(payee.PAYEEID))
        {
            payees.Add(payee.PAYEENAME);
        }
    }
    return payees;
}

bool Model_Payee::is_used(int id)
{
    const auto &trans = Model_Checking::instance().find(Model_Checking::PAYEEID(id));
    if (!trans.empty()) return true;
    const auto &bills = Model_Billsdeposits::instance().find(Model_Billsdeposits::PAYEEID(id));
    if (!bills.empty()) return true;

    return false;
}

bool Model_Payee::is_used(const Data* record)
{
    return is_used(record->PAYEEID);
}

bool Model_Payee::is_used(const Data& record)
{
    return is_used(&record);
}
