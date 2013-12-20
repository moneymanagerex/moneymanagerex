/*******************************************************
 Copyright (C) 2013 Guan Lisheng (guanlisheng@gmail.com)

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

#include "Model_Checking.h"
#include "Model_Account.h"
#include "Model_Payee.h"
#include "Model_Category.h"

const std::vector<std::pair<Model_Checking::TYPE, wxString> > Model_Checking::TYPE_CHOICES = 
{
    std::make_pair(Model_Checking::WITHDRAWAL, wxTRANSLATE("Withdrawal"))
    , std::make_pair(Model_Checking::DEPOSIT, wxTRANSLATE("Deposit"))
    , std::make_pair(Model_Checking::TRANSFER, wxTRANSLATE("Transfer"))
};

Model_Checking::Model_Checking(): Model<DB_Table_CHECKINGACCOUNT_V1>()
{
}

Model_Checking::~Model_Checking() 
{
}

wxArrayString Model_Checking::all_type()
{
    wxArrayString types;
    // keep the sequence with TYPE
    for (const auto& r : TYPE_CHOICES) types.Add(r.second);

    return types;
}

wxArrayString Model_Checking::all_status()
{
    wxArrayString status;
    // keep the sequence with STATUS
    status.Add(wxTRANSLATE("None"));
    status.Add(wxTRANSLATE("Reconciled"));
    status.Add(wxTRANSLATE("Void"));
    status.Add(wxTRANSLATE("Follow up"));
    status.Add(wxTRANSLATE("Duplicate"));

    return status;
}

/** Return the static instance of Model_Checking table */
Model_Checking& Model_Checking::instance()
{
    return Singleton<Model_Checking>::instance();
}

/**
* Initialize the global Model_Checking table.
* Reset the Model_Checking table or create the table if it does not exist.
*/
Model_Checking& Model_Checking::instance(wxSQLite3Database* db)
{
    Model_Checking& ins = Singleton<Model_Checking>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

Model_Checking::Full_Data::Full_Data(): Data(0), BALANCE(0)
{
}

Model_Checking::Full_Data::Full_Data(const Data& r): Data(r), BALANCE(0)
 , m_splits(Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r.TRANSID)))
{
    const Model_Account::Data* from_account = Model_Account::instance().get(r.ACCOUNTID);
    const Model_Account::Data* to_account = Model_Account::instance().get(r.TOACCOUNTID);
    if (from_account) this->ACCOUNTNAME = from_account->ACCOUNTNAME;
    if (to_account) this->TOACCOUNTNAME = to_account->ACCOUNTNAME;

    const Model_Payee::Data* payee = Model_Payee::instance().get(r.PAYEEID);
    if (payee) this->PAYEENAME = payee->PAYEENAME;
    
    if (Model_Checking::splittransaction(r).empty())
        this->CATEGNAME = Model_Category::instance().full_name(r.CATEGID, r.SUBCATEGID);
    else
        this->CATEGNAME = "...";
}

Model_Checking::Full_Data::~Full_Data()
{
}

wxString Model_Checking::Full_Data::real_payee_name(int account_id) const
{
    if (Model_Checking::TRANSFER == Model_Checking::type(this))
    {
        return this->ACCOUNTID == account_id ? this->TOACCOUNTNAME : this->ACCOUNTNAME;
    }
    else
    {
        return this->PAYEENAME;
    }
}

bool Model_Checking::Full_Data::has_split() const
{
    return !this->m_splits.empty();
}
