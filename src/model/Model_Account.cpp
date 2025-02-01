/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2014, 2020 - 2022 Nikolay Akimov

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

#include "option.h"
#include "Model_Account.h"
#include "Model_Stock.h"
#include "Model_Translink.h"
#include "Model_Shareinfo.h"

const std::vector<std::pair<Model_Account::TYPE_ID, wxString> > Model_Account::TYPE_CHOICES =
{
    { Model_Account::TYPE_ID_CASH,        wxString(wxTRANSLATE("Cash")) },
    { Model_Account::TYPE_ID_CHECKING,    wxString(wxTRANSLATE("Checking")) },
    { Model_Account::TYPE_ID_CREDIT_CARD, wxString(wxTRANSLATE("Credit Card")) },
    { Model_Account::TYPE_ID_LOAN,        wxString(wxTRANSLATE("Loan")) },
    { Model_Account::TYPE_ID_TERM,        wxString(wxTRANSLATE("Term")) },
    { Model_Account::TYPE_ID_INVESTMENT,  wxString(wxTRANSLATE("Investment")) },
    { Model_Account::TYPE_ID_ASSET,       wxString(wxTRANSLATE("Asset")) },
    { Model_Account::TYPE_ID_SHARES,      wxString(wxTRANSLATE("Shares")) },
};

const std::vector<std::pair<Model_Account::STATUS_ID, wxString> > Model_Account::STATUS_CHOICES =
{
    { Model_Account::STATUS_ID_OPEN,   wxString(wxTRANSLATE("Open")) },
    { Model_Account::STATUS_ID_CLOSED, wxString(wxTRANSLATE("Closed")) }
};

wxArrayString Model_Account::TYPE_STR = type_str_all();
const wxString Model_Account::TYPE_STR_CASH        = TYPE_STR[TYPE_ID_CASH];
const wxString Model_Account::TYPE_STR_CHECKING    = TYPE_STR[TYPE_ID_CHECKING];
const wxString Model_Account::TYPE_STR_CREDIT_CARD = TYPE_STR[TYPE_ID_CREDIT_CARD];
const wxString Model_Account::TYPE_STR_LOAN        = TYPE_STR[TYPE_ID_LOAN];
const wxString Model_Account::TYPE_STR_TERM        = TYPE_STR[TYPE_ID_TERM];
const wxString Model_Account::TYPE_STR_INVESTMENT  = TYPE_STR[TYPE_ID_INVESTMENT];
const wxString Model_Account::TYPE_STR_ASSET       = TYPE_STR[TYPE_ID_ASSET];
const wxString Model_Account::TYPE_STR_SHARES      = TYPE_STR[TYPE_ID_SHARES];

wxArrayString Model_Account::STATUS_STR = status_str_all();
const wxString Model_Account::STATUS_STR_OPEN   = STATUS_STR[STATUS_ID_OPEN];
const wxString Model_Account::STATUS_STR_CLOSED = STATUS_STR[STATUS_ID_CLOSED];

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
    ins.preload();

    return ins;
}

/** Return the static instance of Model_Account table */
Model_Account& Model_Account::instance()
{
    return Singleton<Model_Account>::instance();
}

wxArrayString Model_Account::all_checking_account_names(bool skip_closed)
{
    wxArrayString accounts;
    for (const auto &account : this->all(COL_ACCOUNTNAME))
    {
        if (skip_closed && status_id(account) == STATUS_ID_CLOSED)
            continue;
        if (type_id(account) == TYPE_ID_INVESTMENT)
            continue;
        if (account.ACCOUNTNAME.empty())
            continue;
        accounts.Add(account.ACCOUNTNAME);
    }
    return accounts;
}

const std::map<wxString, int64> Model_Account::all_accounts(bool skip_closed)
{
    std::map<wxString, int64> accounts;
    for (const auto& account : this->all(COL_ACCOUNTNAME))
    {
        if (skip_closed && status_id(account) == STATUS_ID_CLOSED)
            continue;
        if (type_id(account) == TYPE_ID_INVESTMENT)
            continue;
        if (account.ACCOUNTNAME.empty())
            continue;
        accounts[account.ACCOUNTNAME] = account.ACCOUNTID;
    }
    return accounts;
}

wxArrayString Model_Account::type_str_all()
{
    wxArrayString type;
    int i = 0;
    for (const auto& item : TYPE_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in Model_Account::TYPE_CHOICES");
        type.Add(item.second);
    }
    return type;
}

wxArrayString Model_Account::status_str_all()
{
    wxArrayString status;
    int i = 0;
    for (const auto& item : STATUS_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in Model_Account::STATUS_CHOICES");
        status.Add(item.second);
    }
    return status;
}

/** Get the Data record instance in memory. */
Model_Account::Data* Model_Account::get(const wxString& name)
{
    Data* account = this->get_one(ACCOUNTNAME(name));
    if (account) return account;

    Data_Set items = this->find(ACCOUNTNAME(name));
    if (!items.empty()) account = this->get(items[0].ACCOUNTID, this->db_);
    return account;
}

/** Get the Data record instance in memory. */
Model_Account::Data* Model_Account::getByAccNum(const wxString& num)
{
    Data* account = this->get_one(ACCOUNTNUM(num));
    if (account) return account;

    Data_Set items = this->find(ACCOUNTNUM(num));
    if (!items.empty()) account = this->get(items[0].ACCOUNTID, this->db_);
    return account;
}

wxString Model_Account::get_account_name(int64 account_id)
{
    Data* account = instance().get(account_id);
    if (account)
        return account->ACCOUNTNAME;
    else
        return _("Account Error");
}

/** Remove the Data record instance from memory and the database. */
bool Model_Account::remove(int64 id)
{
    this->Savepoint();
    for (const auto& r: Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(id), Model_Checking::TOACCOUNTID(id)))
    {
        if (Model_Checking::foreignTransaction(r))
        {
            Model_Shareinfo::RemoveShareEntry(r.TRANSID);
            Model_Translink::Data tr = Model_Translink::TranslinkRecord(r.TRANSID);
            Model_Translink::instance().remove(tr.TRANSLINKID);
        }
        Model_Checking::instance().remove(r.TRANSID);
    }
    for (const auto& r: Model_Billsdeposits::instance().find_or(Model_Billsdeposits::ACCOUNTID(id), Model_Billsdeposits::TOACCOUNTID(id)))
        Model_Billsdeposits::instance().remove(r.BDID);

    for (const auto& r : Model_Stock::instance().find(Model_Stock::HELDAT(id)))
    {
        Model_Translink::RemoveTransLinkRecords(Model_Attachment::REFTYPE_ID_STOCK, r.STOCKID);
        Model_Stock::instance().remove(r.STOCKID);
    }
    this->ReleaseSavepoint();

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

const Model_Checking::Data_Set Model_Account::transactionsByDateTimeId(const Data*r)
{
    auto trans = Model_Checking::instance().find_or(
        Model_Checking::ACCOUNTID(r->ACCOUNTID),
        Model_Checking::TOACCOUNTID(r->ACCOUNTID)
    );
    std::sort(trans.begin(), trans.end());
    if (Option::instance().UseTransDateTime())
        std::stable_sort(trans.begin(), trans.end(), SorterByTRANSDATE());
    else
        std::stable_sort(trans.begin(), trans.end(), Model_Checking::SorterByTRANSDATE_DATE());
    return trans;
}

const Model_Checking::Data_Set Model_Account::transactionsByDateTimeId(const Data& r)
{
    return transactionsByDateTimeId(&r);
}

const Model_Billsdeposits::Data_Set Model_Account::billsdeposits(const Data* r)
{
    return Model_Billsdeposits::instance().find_or(Model_Billsdeposits::ACCOUNTID(r->ACCOUNTID), Model_Billsdeposits::TOACCOUNTID(r->ACCOUNTID));
}

const Model_Billsdeposits::Data_Set Model_Account::billsdeposits(const Data& r)
{
    return billsdeposits(&r);
}

double Model_Account::balance(const Data* r)
{
    double sum = r->INITIALBAL;
    for (const auto& tran: transactionsByDateTimeId(r))
    {
        sum += Model_Checking::account_flow(tran, r->ACCOUNTID); 
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
        sum.first += Model_Stock::CurrentValue(stock);
        sum.second += Model_Stock::InvestmentValue(stock);
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

wxString Model_Account::toString(double value, const Data* r, int precision)
{
    return Model_Currency::toString(value, currency(r), precision);
}

wxString Model_Account::toString(double value, const Data& r, int precision)
{
    return toString(value, &r, precision);
}

Model_Account::STATUS_ID Model_Account::status_id(const Data* account)
{
    if (account->STATUS.CmpNoCase(status_str_all()[STATUS_ID_OPEN]) == 0)
        return STATUS_ID_OPEN;
    return STATUS_ID_CLOSED;
}

Model_Account::STATUS_ID Model_Account::status_id(const Data& account)
{
    return status_id(&account);
}

DB_Table_ACCOUNTLIST_V1::STATUS Model_Account::STATUS(STATUS_ID status, OP op)
{
    return DB_Table_ACCOUNTLIST_V1::STATUS(status_str_all()[status], op);
}

Model_Account::TYPE_ID Model_Account::type_id(const Data* account)
{
    static std::unordered_map<wxString, TYPE_ID> cache;
    const auto it = cache.find(account->ACCOUNTTYPE);
    if (it != cache.end()) return it->second;

    for (const auto& t : TYPE_CHOICES) 
    {
        if (account->ACCOUNTTYPE.CmpNoCase(t.second) == 0)
        {
            cache.insert(std::make_pair(account->ACCOUNTTYPE, t.first));
            return t.first;
        }
    }

    cache.insert(std::make_pair(account->ACCOUNTTYPE, TYPE_ID_CHECKING));
    return TYPE_ID_CHECKING;
}

Model_Account::TYPE_ID Model_Account::type_id(const Data& account)
{
    return type_id(&account);
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
    if (!c) return false;
    const auto &accounts = Model_Account::instance().find(
        CURRENCYID(c->CURRENCYID),
        STATUS(STATUS_ID_CLOSED, NOT_EQUAL));
    return !accounts.empty();
}

bool Model_Account::is_used(const Model_Currency::Data& c)
{
    return is_used(&c);
}

int Model_Account::money_accounts_num()
{
    return
        Model_Account::instance().find(ACCOUNTTYPE(TYPE_STR_CASH)).size()
        + Model_Account::instance().find(ACCOUNTTYPE(TYPE_STR_CHECKING)).size()
        + Model_Account::instance().find(ACCOUNTTYPE(TYPE_STR_CREDIT_CARD)).size()
        + Model_Account::instance().find(ACCOUNTTYPE(TYPE_STR_LOAN)).size()
        + Model_Account::instance().find(ACCOUNTTYPE(TYPE_STR_TERM)).size()
        + Model_Account::instance().find(ACCOUNTTYPE(TYPE_STR_ASSET)).size()
        + Model_Account::instance().find(ACCOUNTTYPE(TYPE_STR_SHARES)).size();
}

bool Model_Account::Exist(const wxString& account_name)
{
    Data_Set list = instance().find(ACCOUNTNAME(account_name));

    return !list.empty();
}

wxDateTime Model_Account::DateOf(const wxString& date_str)
{
    return Model::to_date(date_str);
}

bool Model_Account::BoolOf(int64 value)
{
    return value > 0 ? true : false;
}

const Model_Account::Data_Set Model_Account::FilterAccounts(const wxString& account_pattern, bool skip_closed)
{
    Data_Set accounts;
    for (auto &account : this->all(Model_Account::COL_ACCOUNTNAME))
    {
        if (skip_closed && status_id(account) == STATUS_ID_CLOSED)
            continue;
        if (type_id(account) == TYPE_ID_INVESTMENT)
            continue;
        if (account.ACCOUNTNAME.Lower().Matches(account_pattern.Lower().Append("*")))
            accounts.push_back(account);
    }
    return accounts;
}
