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

#pragma once

#include "base/defs.h"
#include "util/mmDate.h"

#include "table/AccountTable.h"
#include "data/AccountData.h"

#include "_ModelBase.h"
#include "CurrencyModel.h"
#include "TrxModel.h"
#include "SchedModel.h"

#include "uicontrols/navigatortypes.h"  // remove later

class AccountModel : public TableFactory<AccountTable, AccountData>
{
public:
    static const RefTypeN s_ref_type;

public:
    AccountModel();
    ~AccountModel();

public:
    static AccountModel& instance(wxSQLite3Database* db);
    static AccountModel& instance();

    // TODO: move to AccountData
    static NavigatorTypes::TYPE_ID type_id(const Data& account_d);

    static AccountCol::STATUS STATUS(OP op, AccountStatus status);

public:
    // override
    bool purge_id(int64 account_id) override;

    // lookup for given Data
    auto get_data_currency_p(const Data& account_d) -> const CurrencyData*;
    auto get_data_balance(const Data& account_d) -> double;
    auto get_data_investment_balance(const Data& account_d) -> std::pair<double, double>;

    // lookup for given id
    auto get_id_name(int64 account_id) -> const wxString;
    auto get_id_currency_p(int64 account_id) -> const CurrencyData*;
    auto find_id_trx_aBySN(int64 account_id) -> const TrxModel::DataA;
    auto find_id_sched_a(int64 account_id) -> const SchedModel::DataA;

    // lookup for given field
    auto get_name_data_n(const wxString& name) -> const Data*;
    auto find_name_data_a(const wxString& name) -> const DataA;
    auto find_pattern_data_a(const wxString& pattern, bool only_open = false) -> const DataA;
    auto get_num_data_n(const wxString& num) -> const Data*;

    // lookup for all Data
    auto find_all_name_a(bool only_open = false) -> const wxArrayString;
    auto find_all_name_id_m(bool only_open = false) -> const std::map<wxString, int64>;
    auto find_all_type_a(bool only_open = false) -> const wxArrayString;
    int  find_money_type_c();

    // wrapper for value format
    auto value_number(const Data& account_d, double value, int precision = 2) -> const wxString;
    auto value_number_currency(const Data& account_d, double value) -> const wxString;

    // modify Data (see FIXME comments in .cpp)
    void dangerous_reset_type(wxString old_type);
    void dangerous_reset_unknown_types();
};

//----------------------------------------------------------------------------

// TODO: move to AccountData
inline NavigatorTypes::TYPE_ID AccountModel::type_id(const Data& account)
{
    return static_cast<NavigatorTypes::TYPE_ID>(NavigatorTypes::instance().getTypeIdFromDBName(account.m_type_));
}

