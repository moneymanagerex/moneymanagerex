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
#include "PrefModel.h"
#include "StockModel.h"
#include "TrxLinkModel.h"
#include "TrxShareModel.h"

const RefTypeN AccountModel::s_ref_type = RefTypeN(RefTypeN::e_account);

AccountModel::AccountModel() :
    TableFactory<AccountTable, AccountData>()
{
}

AccountModel::~AccountModel()
{
}

// Initialize the global AccountModel table.
// Reset the AccountModel table or create the table if it does not exist.
AccountModel& AccountModel::instance(wxSQLite3Database* db)
{
    AccountModel& ins = Singleton<AccountModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();
    ins.preload_cache();

    return ins;
}

// Return the static instance of AccountModel table
AccountModel& AccountModel::instance()
{
    return Singleton<AccountModel>::instance();
}

AccountCol::STATUS AccountModel::STATUS(OP op, AccountStatus status)
{
    return AccountCol::STATUS(op, status.name());
}

// Remove the Data record of a given id, including its auxiliary records
bool AccountModel::purge_id(int64 account_id)
{
    // FIXME: check if account_id is used in InfoTable

    db_savepoint();

    for (const auto& trx_d : TrxModel::instance().find_or(
        TrxCol::ACCOUNTID(account_id),
        TrxCol::TOACCOUNTID(account_id)
    )) {
        if (TrxModel::is_foreign(trx_d)) {
            TrxShareModel::instance().purge_trxId(trx_d.m_id);
            const TrxLinkData* tl_n = TrxLinkModel::instance().get_trx_data_n(trx_d.m_id);
            if (tl_n) {
                TrxLinkModel::instance().purge_id(tl_n->m_id);
            }
        }
        TrxModel::instance().purge_id(trx_d.m_id);
    }

    for (const auto& sched_d : SchedModel::instance().find_or(
        SchedCol::ACCOUNTID(account_id),
        SchedCol::TOACCOUNTID(account_id)
    ))
        SchedModel::instance().purge_id(sched_d.m_id);

    for (const auto& stock_d : StockModel::instance().find(StockCol::HELDAT(account_id))) {
        TrxLinkModel::instance().purge_ref(StockModel::s_ref_type, stock_d.m_id);
        StockModel::instance().purge_id(stock_d.m_id);
    }

    // FIXME: remove AttachmentData owned by account_id

    db_release_savepoint();

    return unsafe_remove_id(account_id);
}

const CurrencyData* AccountModel::get_data_currency_p(const Data& account_d)
{
    const CurrencyData* currency_n = CurrencyModel::instance().get_id_data_n(
        account_d.m_currency_id
    );
    if (currency_n)
        return currency_n;
    else {
        wxASSERT(false);
        return CurrencyModel::instance().get_base_data_n();
    }
}

double AccountModel::get_data_balance(const Data& account_d)
{
    double sum = account_d.m_open_balance;
    // TODO: skip Void and deleted transactions; order by SN is not important
    for (const auto& trx_d: find_id_trx_aBySN(account_d.m_id)) {
        sum += trx_d.account_flow(account_d.m_id);
    }
    return sum;
}

double AccountModel::get_data_balance_to_date(const Data& account_d, mmDate date)
{
    double sum = account_d.m_open_balance;
    // TODO: skip Void and deleted transactions
    // TODO: query transactions up to date; order by SN is not important
    for (const auto& trx_d: find_id_trx_aBySN(account_d.m_id)) {
        if (trx_d.m_date() <= date) {
            sum += trx_d.account_flow(account_d.m_id);
        }
    }
    return sum;
}

std::pair<double, double> AccountModel::get_data_investment_balance(const Data& account_d)
{
    std::pair<double /*market value*/, double /*invest value*/> sum;

    for (const auto& stock_d : StockModel::instance().find(
        StockCol::HELDAT(account_d.m_id)
    )) {
        sum.first  += stock_d.current_value();
        sum.second += stock_d.m_purchase_value;
    }

    for (const auto& asset_d : AssetModel::instance().find_or(
        AssetCol::ASSETNAME(account_d.m_name),
        AssetCol::ASSETTYPE(account_d.m_name)
    )) {
        auto asset_bal = AssetModel::instance().get_data_value(asset_d);
        sum.first  += asset_bal.second;
        sum.second += asset_bal.first;
    }

    return sum;
}

// Get the Data name of a given id
const wxString AccountModel::get_id_name(int64 account_id)
{
    const Data* account_n = get_id_data_n(account_id);
    if (account_n)
        return account_n->m_name;
    else
        return _t("Account Error");
}

// Get the currency of a given account id, or the base currency
const CurrencyData* AccountModel::get_id_currency_p(int64 account_id)
{
    const Data* account_n = get_id_data_n(account_id);
    if (account_n)
        return AccountModel::get_data_currency_p(*account_n);
    else {
        wxASSERT(false);
        return CurrencyModel::instance().get_base_data_n();
    }
}

// Return the list of transactions of a given account id,
// sorted by Serial Number (i.e., by Date/Id or by DateTime/Id)
const TrxModel::DataA AccountModel::find_id_trx_aBySN(int64 account_id)
{
    auto trx_a = TrxModel::instance().find_or(
        TrxCol::ACCOUNTID(account_id),
        TrxCol::TOACCOUNTID(account_id)
    );
    std::sort(trx_a.begin(), trx_a.end());
    if (PrefModel::instance().UseTransDateTime())
        std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByDateTime());
    else
        std::stable_sort(trx_a.begin(), trx_a.end(), TrxData::SorterByDate());
    return trx_a;
}

const SchedModel::DataA AccountModel::find_id_sched_a(int64 account_id)
{
    return SchedModel::instance().find_or(
        SchedCol::ACCOUNTID(account_id),
        SchedCol::TOACCOUNTID(account_id)
    );
}

// Get the Data record with given name
const AccountData* AccountModel::get_name_data_n(const wxString& name)
{
    const Data* account_n = search_cache_n(AccountCol::ACCOUNTNAME(name));
    if (account_n)
        return account_n;

    DataA account_a = find(AccountCol::ACCOUNTNAME(name));
    if (!account_a.empty())
        account_n = get_id_data_n(account_a[0].m_id);
    return account_n;
}

const AccountModel::DataA AccountModel::find_name_data_a(const wxString& name)
{
    return find(AccountCol::ACCOUNTNAME(name));
}

const AccountModel::DataA AccountModel::find_pattern_data_a(
    const wxString& name_pattern,
    bool only_open
) {
    DataA account_a;
    for (auto &account_d : find_all(
        AccountCol::COL_ID_ACCOUNTNAME
    )) {
        if (only_open && !account_d.is_open())
            continue;
        if (type_id(account_d) == NavigatorTypes::TYPE_ID_INVESTMENT)
            continue;
        if (account_d.m_name.Lower().Matches(name_pattern.Lower().Append("*")))
            account_a.push_back(account_d);
    }
    return account_a;
}

// Get a Data record with given num
const AccountData* AccountModel::get_num_data_n(const wxString& num)
{
    const Data* account_n = search_cache_n(AccountCol::ACCOUNTNUM(num));
    if (account_n)
        return account_n;

    DataA account_a = find(AccountCol::ACCOUNTNUM(num));
    if (!account_a.empty())
        account_n = get_id_data_n(account_a[0].m_id);
    return account_n;
}

const wxArrayString AccountModel::find_all_name_a(bool only_open)
{
    wxArrayString name_a;
    for (const auto& account_d : find_all(Col::COL_ID_ACCOUNTNAME)) {
        if (only_open && !account_d.is_open())
            continue;
        if (type_id(account_d) == NavigatorTypes::TYPE_ID_SHARES)
            continue;
        if (account_d.m_name.empty())
            continue;
        name_a.Add(account_d.m_name);
    }
    return name_a;
}

const std::map<wxString, int64> AccountModel::find_all_name_id_m(bool only_open)
{
    std::map<wxString, int64> name_id_m;
    for (const auto& account_d : find_all(Col::COL_ID_ACCOUNTNAME)) {
        if (only_open && !account_d.is_open())
            continue;
        if (type_id(account_d) == NavigatorTypes::TYPE_ID_SHARES)
            continue;
        if (account_d.m_name.empty())
            continue;
        name_id_m[account_d.m_name] = account_d.m_id;
    }
    return name_id_m;
}

const wxArrayString AccountModel::find_all_type_a(bool only_open)
{
    wxArrayString usedTypes;
    for (auto& account_d : find_all(
        AccountCol::COL_ID_ACCOUNTTYPE
    )) {
        if (only_open && !account_d.is_open())
            continue;
        if (type_id(account_d) == NavigatorTypes::TYPE_ID_INVESTMENT)
            continue;
        if (usedTypes.Index(account_d.m_type_) == wxNOT_FOUND) {
            usedTypes.Add(account_d.m_type_);
        }
    }
    return usedTypes;
}

int AccountModel::find_money_type_c()
{
    return
        find(
            AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_CASH))
        ).size() + find(
            AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_CHECKING))
        ).size() + find(
            AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_CREDIT_CARD))
        ).size() + find(
            AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_LOAN))
        ).size() + find(
            AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_TERM))
        ).size() + find(
            AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_ASSET))
        ).size() + find(
            AccountCol::ACCOUNTTYPE(NavigatorTypes::instance().type_name(NavigatorTypes::TYPE_ID_SHARES))
        ).size();
}

// FIXME: see comments in NavigatorTypes::DeleteEntry()
void AccountModel::dangerous_reset_type(wxString old_type)
{
    for (auto& account_d : find(
        AccountCol::ACCOUNTTYPE(old_type)
    )) {
        AccountData* account_n = unsafe_get_id_data_n(account_d.m_id);
        account_n->m_type_ = "Checking";
        unsafe_save_data_n(account_n);
    }
}

// FIXME: see comments in mmNavigatorDialog::setDefault()
void AccountModel::dangerous_reset_unknown_types()
{
    for (const auto& account_d : find_all(
        Col::COL_ID_ACCOUNTNAME
    )) {
        // FIXME: cannot use NavigatorTypes::instance() here.
        // *Model is lower level than the GUI; it must be independent of GUI.
        if (NavigatorTypes::instance().getTypeIdFromDBName(account_d.m_type_, -1) == -1) {
            // CHECK: use account_d directly
            AccountData acc_d = *(get_name_data_n(account_d.m_name));
            acc_d.m_type_ = "Checking";
            save_data_n(acc_d);
        }
    }
}

const wxString AccountModel::value_number(const Data& account_d, double value, int precision)
{
    return CurrencyModel::instance().toString(
        value,
        AccountModel::get_data_currency_p(account_d),
        precision
    );
}

const wxString AccountModel::value_number_currency(const Data& account_d, double value)
{
    return CurrencyModel::instance().toCurrency(
        value,
        AccountModel::get_data_currency_p(account_d)
    );
}
