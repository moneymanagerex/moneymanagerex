/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2014, 2020 - 2022 Nikolay Akimov
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

#include <unordered_set>

#include "AccountModel.h"
#include "PreferencesModel.h"
#include "StockModel.h"
#include "TransactionLinkModel.h"
#include "TransactionShareModel.h"

ChoicesName AccountModel::STATUS_CHOICES = ChoicesName({
    { STATUS_ID_OPEN,   _n("Open") },
    { STATUS_ID_CLOSED, _n("Closed") }
});

const wxString AccountModel::STATUS_NAME_OPEN   = status_name(STATUS_ID_OPEN);
const wxString AccountModel::STATUS_NAME_CLOSED = status_name(STATUS_ID_CLOSED);

AccountModel::AccountModel()
: Model<AccountTable>()
{
}

AccountModel::~AccountModel()
{
}

/**
* Initialize the global AccountModel table.
* Reset the AccountModel table or create the table if it does not exist.
*/
AccountModel& AccountModel::instance(wxSQLite3Database* db)
{
    AccountModel& ins = Singleton<AccountModel>::instance();
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();
    ins.preload();
    return ins;
}

/** Return the static instance of AccountModel table */
AccountModel& AccountModel::instance()
{
    return Singleton<AccountModel>::instance();
}

wxArrayString AccountModel::all_checking_account_names(bool skip_closed)
{
    wxArrayString accounts;
    for (const auto &account : this->get_all(Col::COL_ID_ACCOUNTNAME))
    {
        if (skip_closed && status_id(account) == STATUS_ID_CLOSED)
            continue;
        if (type_id(account) == NavigatorTypes::TYPE_ID_SHARES)
            continue;
        if (account.ACCOUNTNAME.empty())
            continue;
        accounts.Add(account.ACCOUNTNAME);
    }
    return accounts;
}

const std::map<wxString, int64> AccountModel::all_accounts(bool skip_closed)
{
    std::map<wxString, int64> accounts;
    for (const auto& account : this->get_all(Col::COL_ID_ACCOUNTNAME))
    {
        if (skip_closed && status_id(account) == STATUS_ID_CLOSED)
            continue;
        if (type_id(account) == NavigatorTypes::TYPE_ID_SHARES)
            continue;
        if (account.ACCOUNTNAME.empty())
            continue;
        accounts[account.ACCOUNTNAME] = account.ACCOUNTID;
    }
    return accounts;
}

/** Get the Data record instance in memory. */
AccountModel::Data* AccountModel::get_key(const wxString& name)
{
    Data* account = this->search_cache(ACCOUNTNAME(name));
    if (account)
        return account;

    Data_Set items = this->find(ACCOUNTNAME(name));
    if (!items.empty())
        account = this->get_id(items[0].ACCOUNTID);
    return account;
}

/** Get the Data record instance in memory. */
AccountModel::Data* AccountModel::cache_num(const wxString& num)
{
    Data* account = this->search_cache(ACCOUNTNUM(num));
    if (account)
        return account;

    Data_Set items = this->find(ACCOUNTNUM(num));
    if (!items.empty())
        account = this->get_id(items[0].ACCOUNTID);
    return account;
}

wxString AccountModel::cache_id_name(int64 account_id)
{
    Data* account = instance().get_id(account_id);
    if (account)
        return account->ACCOUNTNAME;
    else
        return _t("Account Error");
}

/** Remove the Data record instance from memory and the database. */
bool AccountModel::remove(const int64 id)
{
    this->Savepoint();
    for (const auto& r: TransactionModel::instance().find_or(TransactionModel::ACCOUNTID(id), TransactionModel::TOACCOUNTID(id)))
    {
        if (TransactionModel::foreignTransaction(r))
        {
            TransactionShareModel::RemoveShareEntry(r.TRANSID);
            TransactionLinkModel::Data tr = TransactionLinkModel::TranslinkRecord(r.TRANSID);
            TransactionLinkModel::instance().remove(tr.TRANSLINKID);
        }
        TransactionModel::instance().remove(r.TRANSID);
    }
    for (const auto& r: ScheduledModel::instance().find_or(ScheduledModel::ACCOUNTID(id), ScheduledModel::TOACCOUNTID(id)))
        ScheduledModel::instance().remove(r.BDID);

    for (const auto& r : StockModel::instance().find(StockModel::HELDAT(id)))
    {
        TransactionLinkModel::RemoveTransLinkRecords<StockModel>(r.STOCKID);
        StockModel::instance().remove(r.STOCKID);
    }
    this->ReleaseSavepoint();

    return Model<AccountTable>::remove(id);
}

CurrencyModel::Data* AccountModel::currency(const Data* r)
{
    CurrencyModel::Data * currency = CurrencyModel::instance().get_id(r->CURRENCYID);
    if (currency)
        return currency;
    else
    {
        wxASSERT(false);
        return CurrencyModel::GetBaseCurrency();
    }
}

CurrencyModel::Data* AccountModel::currency(const Data& r)
{
    return currency(&r);
}

const TransactionModel::Data_Set AccountModel::transactionsByDateTimeId(const Data*r)
{
    auto trans = TransactionModel::instance().find_or(
        TransactionModel::ACCOUNTID(r->ACCOUNTID),
        TransactionModel::TOACCOUNTID(r->ACCOUNTID)
    );
    std::sort(trans.begin(), trans.end());
    if (PreferencesModel::instance().UseTransDateTime())
        std::stable_sort(trans.begin(), trans.end(), TransactionRow::SorterByTRANSDATE());
    else
        std::stable_sort(trans.begin(), trans.end(), TransactionModel::SorterByTRANSDATE_DATE());
    return trans;
}

const TransactionModel::Data_Set AccountModel::transactionsByDateTimeId(const Data& r)
{
    return transactionsByDateTimeId(&r);
}

const ScheduledModel::Data_Set AccountModel::billsdeposits(const Data* r)
{
    return ScheduledModel::instance().find_or(ScheduledModel::ACCOUNTID(r->ACCOUNTID), ScheduledModel::TOACCOUNTID(r->ACCOUNTID));
}

const ScheduledModel::Data_Set AccountModel::billsdeposits(const Data& r)
{
    return billsdeposits(&r);
}

double AccountModel::balance(const Data* r)
{
    double sum = r->INITIALBAL;
    for (const auto& tran: transactionsByDateTimeId(r))
    {
        sum += TransactionModel::account_flow(tran, r->ACCOUNTID);
    }

    return sum;
}

double AccountModel::balance(const Data& r)
{
    return balance(&r);
}

std::pair<double, double> AccountModel::investment_balance(const Data* r)
{
    std::pair<double /*market value*/, double /*invest value*/> sum;
    for (const auto& stock: StockModel::instance().find(StockModel::HELDAT(r->ACCOUNTID)))
    {
        sum.first += StockModel::CurrentValue(stock);
        sum.second += StockModel::InvestmentValue(stock);
    }

    for (const auto& asset: AssetModel::instance().find_or(AssetModel::ASSETNAME(r->ACCOUNTNAME), AssetTable::ASSETTYPE(r->ACCOUNTNAME)))
    {
        auto asset_bal = AssetModel::value(asset);
        sum.first += asset_bal.second;
        sum.second += asset_bal.first;
    }
    return sum;
}

std::pair<double, double> AccountModel::investment_balance(const Data& r)
{
    return investment_balance(&r);
}

wxString AccountModel::toCurrency(double value, const Data* r)
{
    return CurrencyModel::toCurrency(value, currency(r));
}

wxString AccountModel::toString(double value, const Data* r, int precision)
{
    return CurrencyModel::toString(value, currency(r), precision);
}

wxString AccountModel::toString(double value, const Data& r, int precision)
{
    return toString(value, &r, precision);
}

AccountTable::STATUS AccountModel::STATUS(OP op, STATUS_ID status)
{
    return AccountTable::STATUS(op, status_name(status));
}

bool AccountModel::FAVORITEACCT(const Data* r)
{
    return r->FAVORITEACCT.CmpNoCase("TRUE") == 0;
}
bool AccountModel::FAVORITEACCT(const Data& r)
{
    return FAVORITEACCT(&r);
}

bool AccountModel::is_used(const CurrencyModel::Data* c)
{
    if (!c) return false;
    const auto &accounts = AccountModel::instance().find(
        CURRENCYID(c->CURRENCYID),
        STATUS(OP_NE, STATUS_ID_CLOSED)
    );
    return !accounts.empty();
}

bool AccountModel::is_used(const CurrencyModel::Data& c)
{
    return is_used(&c);
}

int AccountModel::money_accounts_num()
{
    return
        AccountModel::instance().find(ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_CASH))).size()
        + AccountModel::instance().find(ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_CHECKING))).size()
        + AccountModel::instance().find(ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_CREDIT_CARD))).size()
        + AccountModel::instance().find(ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_LOAN))).size()
        + AccountModel::instance().find(ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_TERM))).size()
        + AccountModel::instance().find(ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_ASSET))).size()
        + AccountModel::instance().find(ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_SHARES))).size();
}

bool AccountModel::Exist(const wxString& account_name)
{
    Data_Set list = instance().find(ACCOUNTNAME(account_name));

    return !list.empty();
}

bool AccountModel::BoolOf(int64 value)
{
    return value > 0 ? true : false;
}

const AccountModel::Data_Set AccountModel::FilterAccounts(const wxString& account_pattern, bool skip_closed)
{
    Data_Set accounts;
    for (auto &account : this->get_all(AccountCol::COL_ID_ACCOUNTNAME))
    {
        if (skip_closed && status_id(account) == STATUS_ID_CLOSED)
            continue;
        if (type_id(account) == NavigatorTypes::TYPE_ID_INVESTMENT)
            continue;
        if (account.ACCOUNTNAME.Lower().Matches(account_pattern.Lower().Append("*")))
            accounts.push_back(account);
    }
    return accounts;
}

void AccountModel::resetAccountType(wxString oldtype)
{
    for (auto item : AccountModel::instance().find(ACCOUNTTYPE(oldtype))) {
        AccountModel::Data* adata = this->get_key(item.ACCOUNTNAME);
        adata->ACCOUNTTYPE = "Checking";
        this->save(adata);
    }
}

void AccountModel::resetUnknownAccountTypes()
{
    for (const auto &account : this->get_all(Col::COL_ID_ACCOUNTNAME)) {
        if (NavigatorTypes::instance().getTypeIdFromDBName(account.ACCOUNTTYPE, -1) == -1) {
            AccountModel::Data* adata = this->get_key(account.ACCOUNTNAME);
            adata->ACCOUNTTYPE = "Checking";
            this->save(adata);
        }
    }
}

wxArrayString AccountModel::getUsedAccountTypes(bool skip_closed)
{
    wxArrayString usedTypes;
    for (auto &account : this->get_all(AccountCol::COL_ID_ACCOUNTTYPE))
    {
        if (skip_closed && status_id(account) == STATUS_ID_CLOSED)
            continue;
        if (type_id(account) == NavigatorTypes::TYPE_ID_INVESTMENT)
            continue;
        if (usedTypes.Index(account.ACCOUNTTYPE) == wxNOT_FOUND) {
            usedTypes.Add(account.ACCOUNTTYPE);
        }
    }
    return usedTypes;
}
