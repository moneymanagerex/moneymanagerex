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

const std::vector<std::pair<Model_Checking::STATUS_ENUM, wxString> > Model_Checking::STATUS_ENUM_CHOICES =
{
    std::make_pair(Model_Checking::NONE, wxTRANSLATE("None"))
    , std::make_pair(Model_Checking::RECONCILED, wxTRANSLATE("Reconciled"))
    , std::make_pair(Model_Checking::VOID_, wxTRANSLATE("Void"))
    , std::make_pair(Model_Checking::FOLLOWUP, wxTRANSLATE("Follow up"))
    , std::make_pair(Model_Checking::DUPLICATE_, wxTRANSLATE("Duplicate"))
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
    for (const auto& r : TYPE_CHOICES) types.Add(r.second);

    return types;
}

wxArrayString Model_Checking::all_status()
{
    wxArrayString status;
    for (const auto& r : STATUS_ENUM_CHOICES) status.Add(r.second);

    return status;
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

/** Return the static instance of Model_Checking table */
Model_Checking& Model_Checking::instance()
{
    return Singleton<Model_Checking>::instance();
}

bool Model_Checking::remove(int id)
{
    //TODO: remove all split at once
    //Model_Splittransaction::instance().remove(Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id)));
    for (const auto& r : Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id)))
        Model_Splittransaction::instance().remove(r.SPLITTRANSID);
    return this->remove(id, db_);
}

Model_Splittransaction::Data_Set Model_Checking::splittransaction(const Data* r)
{
    return Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r->TRANSID));
}

Model_Splittransaction::Data_Set Model_Checking::splittransaction(const Data& r)
{
    return Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r.TRANSID));
}

DB_Table_CHECKINGACCOUNT_V1::TRANSDATE Model_Checking::TRANSDATE(const wxDate& date, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::TRANSDATE(date.FormatISODate(), op);
}

DB_Table_CHECKINGACCOUNT_V1::STATUS Model_Checking::STATUS(STATUS_ENUM status, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::STATUS(toShortStatus(all_status()[status]), op);
}

DB_Table_CHECKINGACCOUNT_V1::TRANSCODE Model_Checking::TRANSCODE(TYPE type, OP op)
{
    return DB_Table_CHECKINGACCOUNT_V1::TRANSCODE(all_type()[type], op);
}

wxDate Model_Checking::TRANSDATE(const Data* r)
{
    return Model::to_date(r->TRANSDATE);
}

wxDate Model_Checking::TRANSDATE(const Data& r)
{
    return Model::to_date(r.TRANSDATE);
}

Model_Checking::TYPE Model_Checking::type(const Data* r)
{
    for (const auto& t : TYPE_CHOICES) if (r->TRANSCODE.CmpNoCase(t.second) == 0) return t.first;

    return WITHDRAWAL;
}

Model_Checking::TYPE Model_Checking::type(const Data& r)
{
    return type(&r);
}

Model_Checking::STATUS_ENUM Model_Checking::status(const Data* r)
{
    if (r->STATUS.CmpNoCase(all_status()[NONE]) == 0)
        return NONE;
    else if (r->STATUS.CmpNoCase(all_status()[RECONCILED]) == 0 || r->STATUS.CmpNoCase("R") == 0)
        return RECONCILED;
    else if (r->STATUS.CmpNoCase(all_status()[VOID_]) == 0 || r->STATUS.CmpNoCase("V") == 0)
        return VOID_;
    else if (r->STATUS.CmpNoCase(all_status()[FOLLOWUP]) == 0 || r->STATUS.CmpNoCase("F") == 0)
        return FOLLOWUP;
    else if (r->STATUS.CmpNoCase(all_status()[DUPLICATE_]) == 0 || r->STATUS.CmpNoCase("D") == 0)
        return DUPLICATE_;
    else
        return NONE;
}

Model_Checking::STATUS_ENUM Model_Checking::status(const Data& r)
{
    return status(&r);
}

double Model_Checking::amount(const Data* r, int account_id)
{
    double sum = 0;
    switch (type(r))
    {
    case WITHDRAWAL:
        sum -= r->TRANSAMOUNT;
        break;
    case DEPOSIT:
        sum += r->TRANSAMOUNT;
        break;
    case TRANSFER:
        if (account_id == r->ACCOUNTID)
            sum -= r->TRANSAMOUNT;
        else
            sum += r->TOTRANSAMOUNT;
        break;
    default:
        break;
    }
    return sum;
}

double Model_Checking::amount(const Data&r, int account_id)
{
    return amount(&r, account_id);
}

double Model_Checking::balance(const Data* r, int account_id)
{
    if (Model_Checking::status(r) == Model_Checking::VOID_) return 0;
    return amount(r, account_id);
}

double Model_Checking::balance(const Data& r, int account_id)
{
    return balance(&r, account_id);
}

double Model_Checking::withdrawal(const Data* r, int account_id)
{
    double bal = balance(r, account_id);
    return bal <= 0 ? -bal : 0;
}

double Model_Checking::withdrawal(const Data& r, int account_id)
{
    return withdrawal(&r, account_id);
}

double Model_Checking::deposit(const Data* r, int account_id)
{
    double bal = balance(r, account_id);
    return bal > 0 ? bal : 0;
}

double Model_Checking::deposit(const Data& r, int account_id)
{
    return deposit(&r, account_id);
}

double Model_Checking::reconciled(const Data* r, int account_id)
{
    double bal = balance(r, account_id);
    return Model_Checking::status(r) == Model_Checking::RECONCILED ? bal : 0;
}

double Model_Checking::reconciled(const Data& r, int account_id)
{
    return reconciled(&r, account_id);
}

wxString Model_Checking::toShortStatus(const wxString& fullStatus)
{
    wxString s = fullStatus.Left(1);
    s.Replace("N", "");
    return s;
}

Model_Checking::Full_Data::Full_Data() : Data(0), BALANCE(0)
{
}

Model_Checking::Full_Data::Full_Data(const Data& r): Data(r), BALANCE(0)
 , m_splits(Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(r.TRANSID)))
{
    const Model_Account::Data* from_account = Model_Account::instance().get(r.ACCOUNTID);
    if (from_account) this->ACCOUNTNAME = from_account->ACCOUNTNAME;

    if (Model_Checking::TRANSFER == Model_Checking::type(this))
    {
        const Model_Account::Data* to_account = Model_Account::instance().get(r.TOACCOUNTID);
        if (to_account) this->TOACCOUNTNAME = to_account->ACCOUNTNAME;
    }
    else
    {
        const Model_Payee::Data* payee = Model_Payee::instance().get(r.PAYEEID);
        if (payee) this->PAYEENAME = payee->PAYEENAME;
    }
    
    /*if (Model_Checking::splittransaction(r).empty())
        this->CATEGNAME = Model_Category::instance().full_name(r.CATEGID, r.SUBCATEGID);
    else
        this->CATEGNAME = "...";*/
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
