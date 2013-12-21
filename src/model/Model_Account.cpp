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

#include "Model_Account.h"
#include "Model_Stock.h"

Model_Account::Model_Account()
: Model<DB_Table_ACCOUNTLIST_V1>()
{
}

Model_Account::~Model_Account()
{
}

/**
* Initialize the global Model_Account table.
* Reset the Model_Account table or create the table if it does not exist.
*/
Model_Account& Model_Account::instance(wxSQLite3Database* db)
{
    Model_Account& ins = Singleton<Model_Account>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Account table */
Model_Account& Model_Account::instance()
{
    return Singleton<Model_Account>::instance();
}

wxArrayString Model_Account::all_account_names()
{
    wxArrayString accounts;
    for (const auto &account : this->all(COL_ACCOUNTNAME))
    {
        accounts.Add(account.ACCOUNTNAME);
    }
    return accounts;
}

wxArrayString Model_Account::all_checking_account_names()
{
    wxArrayString accounts;
    for (const auto &account : this->all(COL_ACCOUNTNAME))
    {
        if (type(account) == INVESTMENT) continue;
        accounts.Add(account.ACCOUNTNAME);
    }
    return accounts;
}

wxArrayString Model_Account::all_status()
{
    wxArrayString status;
    status.Add(wxTRANSLATE("Open"));
    status.Add(wxTRANSLATE("Closed"));

    return status;
}

wxArrayString Model_Account::all_type()
{
    wxArrayString type;
    type.Add(wxTRANSLATE("Checking"));
    type.Add(wxTRANSLATE("Term"));
    type.Add(wxTRANSLATE("Investment"));

    return type;
}

/** Get the Data record instance in memory. */
Model_Account::Data* Model_Account::get(const wxString& name)
{
    Data* account = 0;
    Data_Set items = this->find(ACCOUNTNAME(name));
    if (!items.empty()) account = this->get(items[0].ACCOUNTID, this->db_);
    return account;
}

/** Remove the Data record instance from memory and the database. */
bool Model_Account::remove(int id)
{
    this->Begin();
    for (const auto& r: Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(id), Model_Checking::TOACCOUNTID(id)))
        Model_Checking::instance().remove(r.TRANSID);
    for (const auto& r: Model_Billsdeposits::instance().find_or(Model_Billsdeposits::ACCOUNTID(id), Model_Billsdeposits::TOACCOUNTID(id)))
        Model_Billsdeposits::instance().remove(r.BDID);
    this->Commit();

    return this->remove(id, db_);
}

Model_Currency::Data* Model_Account::currency(const Data* r)
{
    Model_Currency::Data * currency = Model_Currency::instance().get(r->CURRENCYID);
    if (currency)
        return currency;
    else
    {
        wxASSERT(false);
        return Model_Currency::GetBaseCurrency();
    }
}
    
Model_Currency::Data* Model_Account::currency(const Data& r)
{
    return currency(&r);
}

Model_Checking::Data_Set Model_Account::transaction(const Data*r )
{
    Model_Checking::Data_Set trans = Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(r->ACCOUNTID), Model_Checking::TOACCOUNTID(r->ACCOUNTID));
    std::sort(trans.begin(), trans.end());
    std::stable_sort(trans.begin(), trans.end(), SorterByTRANSDATE());

    return trans;
}

Model_Checking::Data_Set Model_Account::transaction(const Data& r)
{
    return transaction(&r);
}

Model_Billsdeposits::Data_Set Model_Account::billsdeposits(const Data* r)
{
	return Model_Billsdeposits::instance().find_or(Model_Billsdeposits::ACCOUNTID(r->ACCOUNTID), Model_Billsdeposits::TOACCOUNTID(r->ACCOUNTID));
}

Model_Billsdeposits::Data_Set Model_Account::billsdeposits(const Data& r)
{
    return billsdeposits(&r);
}

wxDate Model_Account::last_date(const Data* r)
{
    Model_Checking::Data_Set trans = transaction(r);
    if (!trans.empty()) return Model_Checking::TRANSDATE(trans.back());
        
    return wxDateTime::Today();
}

wxDate Model_Account::last_date(const Data& r)
{
    return last_date(&r);
}

double Model_Account::balance(const Data* r)
{
    double sum = r->INITIALBAL;
    for (const auto& tran: transaction(r))
    {
        sum += Model_Checking::balance(tran, r->ACCOUNTID); 
    }
    return sum;
}

double Model_Account::balance(const Data& r)
{
    return balance(&r);
}

std::pair<double, double> Model_Account::investment_balance(const Data* r)
{
    std::pair<double /*origianl input value*/, double /**/> sum;
    for (const auto& stock: Model_Stock::instance().find(Model_Stock::HELDAT(r->ACCOUNTID)))
    {
        sum.first += stock.VALUE;
        sum.second += Model_Stock::value(stock);
    }
    return sum;
}

std::pair<double, double> Model_Account::investment_balance(const Data& r)
{
    return investment_balance(&r);
}

wxString Model_Account::toCurrency(double value, const Data* r)
{
    return Model_Currency::toCurrency(value, currency(r));
}    

wxString Model_Account::toString(double value, const Data* r)
{
    return Model_Currency::toString(value, currency(r));
}

wxString Model_Account::toString(double value, const Data& r)
{
    return toString(value, &r);
}

Model_Account::STATUS Model_Account::status(const Data* account)
{
    if (account->STATUS.CmpNoCase(all_status()[OPEN]) == 0)
        return OPEN;
    return CLOSED;
}

Model_Account::STATUS Model_Account::status(const Data& account)
{
    return status(&account);
}

Model_Account::TYPE Model_Account::type(const Data* account)
{
    if (account->ACCOUNTTYPE.CmpNoCase(all_type()[CHECKING]) == 0)
        return CHECKING;
    else if (account->ACCOUNTTYPE.CmpNoCase(all_type()[TERM]) == 0)
        return TERM;
    else
        return INVESTMENT;
}

Model_Account::TYPE Model_Account::type(const Data& account)
{
    return type(&account);
}

bool Model_Account::FAVORITEACCT(const Data* r)
{
    return r->FAVORITEACCT.CmpNoCase("TRUE") == 0;
}

bool Model_Account::FAVORITEACCT(const Data& r)
{
    return FAVORITEACCT(&r);
}

bool Model_Account::is_used(const Model_Currency::Data* c)
{
    Data_Set accounts = Model_Account::instance().find(CURRENCYID(c->CURRENCYID));
    return !accounts.empty();
}

bool Model_Account::is_used(const Model_Currency::Data& c)
{
    return is_used(&c);
}

int Model_Account::checking_account_num()
{
    return Model_Account::instance().find(ACCOUNTTYPE(all_type()[CHECKING])).size();
}

bool Model_Account::hasActiveTermAccount()
{
    return !Model_Account::instance().find(ACCOUNTTYPE(all_type()[TERM]), DB_Table_ACCOUNTLIST_V1::STATUS(all_status()[OPEN])).empty(); 
}
