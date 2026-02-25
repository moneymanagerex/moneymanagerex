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
#include "util/_choices.h"
#include "util/mmDateDay.h"

#include "table/AccountTable.h"

#include "_ModelBase.h"
#include "CurrencyModel.h"
#include "TransactionModel.h"
#include "ScheduledModel.h"

#include "uicontrols/navigatortypes.h"  // remove later

class AccountModel : public Model<AccountTable>
{
public:
    using Model<AccountTable>::remove;
    using Model<AccountTable>::get_id;

    enum STATUS_ID
    {
        STATUS_ID_OPEN = 0,
        STATUS_ID_CLOSED,
        STATUS_ID_size
    };
    static const wxString STATUS_NAME_OPEN;
    static const wxString STATUS_NAME_CLOSED;

public:
    static ChoicesName STATUS_CHOICES;

public:
    AccountModel();
    ~AccountModel();

public:
    /**
    Initialize the global AccountModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for AccountModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static AccountModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for AccountModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static AccountModel& instance();

public:
    /** Return the Data record for the given account name */
    Data* get_key(const wxString& name);

    /** Return the Data record for the given account num */
    Data* cache_num(const wxString& num);

    static wxString cache_id_name(int64 account_id);

    /** Remove the Data record from memory and the database. */
    bool remove(const int64 id);

public:
    wxArrayString all_checking_account_names(bool skip_closed = false);
    const std::map<wxString, int64> all_accounts(bool skip_closed = false);

    static CurrencyModel::Data* currency(const Data* r);
    static CurrencyModel::Data* currency(const Data& r);

    static const TransactionModel::Data_Set transactionsByDateTimeId(const Data* r);
    static const TransactionModel::Data_Set transactionsByDateTimeId(const Data& r);

    static const ScheduledModel::Data_Set billsdeposits(const Data* r);
    static const ScheduledModel::Data_Set billsdeposits(const Data& r);

    static double balance(const Data* r);
    static double balance(const Data& r);

    static std::pair<double, double> investment_balance(const Data* r);
    static std::pair<double, double> investment_balance(const Data& r);
    static wxString toCurrency(double value, const Data* r);

    static wxString toString(double value, const Data* r, int precision = 2);
    static wxString toString(double value, const Data& r, int precision = 2);

    static NavigatorTypes::TYPE_ID type_id(const Data* account);
    static NavigatorTypes::TYPE_ID type_id(const Data& account);

    static const wxString status_name(int id);
    static int status_id(const wxString& name, int default_id = STATUS_ID_CLOSED);
    static STATUS_ID status_id(const Data* account);
    static STATUS_ID status_id(const Data& account);
    static AccountTable::STATUS STATUS(OP op, STATUS_ID status);

    static bool FAVORITEACCT(const Data* r);
    static bool FAVORITEACCT(const Data& r);

    static bool is_used(const CurrencyModel::Data* c);
    static bool is_used(const CurrencyModel::Data& c);

    static int money_accounts_num();

    static bool Exist(const wxString& account_name);

    static bool BoolOf(int64 value);
    static bool is_positive(int value);

    const Data_Set FilterAccounts(const wxString& account_pattern, bool skip_closed = false);


public:
    static const wxString refTypeName;
    void resetAccountType(wxString oldtype);
    void resetUnknownAccountTypes();
    wxArrayString getUsedAccountTypes(bool skip_closed = true);
};
//----------------------------------------------------------------------------

inline NavigatorTypes::TYPE_ID AccountModel::type_id(const Data* account)
{
    return static_cast<NavigatorTypes::TYPE_ID>(NavigatorTypes::instance().getTypeIdFromDBName(account->ACCOUNTTYPE));
}

inline NavigatorTypes::TYPE_ID AccountModel::type_id(const Data& account)
{
    return type_id(&account);
}

inline const wxString AccountModel::status_name(int id)
{
    return STATUS_CHOICES.getName(id);
}

inline int AccountModel::status_id(const wxString& name, int default_id)
{
    return STATUS_CHOICES.findName(name, default_id);
}

inline AccountModel::STATUS_ID AccountModel::status_id(const Data* account)
{
    return static_cast<STATUS_ID>(status_id(account->STATUS));
}

inline AccountModel::STATUS_ID AccountModel::status_id(const Data& account)
{
    return status_id(&account);
}

inline bool AccountModel::is_positive(int value)
{
    return value > 0 ? true : false;
}

