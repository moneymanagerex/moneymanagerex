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

#ifndef MODEL_ACCOUNT_H
#define MODEL_ACCOUNT_H

#include "Model.h"
#include "db/DB_Table_Accountlist_V1.h"
#include "Model_Currency.h" // detect base currency
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"

class Model_Account : public Model<DB_Table_ACCOUNTLIST_V1>
{
public:
    using Model<DB_Table_ACCOUNTLIST_V1>::remove;
    using Model<DB_Table_ACCOUNTLIST_V1>::get;

    enum TYPE_ID
    {
        TYPE_ID_CASH = 0,
        TYPE_ID_CHECKING,
        TYPE_ID_CREDIT_CARD,
        TYPE_ID_LOAN,
        TYPE_ID_TERM,
        TYPE_ID_INVESTMENT,
        TYPE_ID_ASSET,
        TYPE_ID_SHARES,
        TYPE_ID_size
    };
    enum STATUS_ID
    {
        STATUS_ID_OPEN = 0,
        STATUS_ID_CLOSED
    };
    static wxArrayString TYPE_STR;
    static const wxString TYPE_STR_CASH;
    static const wxString TYPE_STR_CHECKING;
    static const wxString TYPE_STR_CREDIT_CARD;
    static const wxString TYPE_STR_LOAN;
    static const wxString TYPE_STR_TERM;
    static const wxString TYPE_STR_INVESTMENT;
    static const wxString TYPE_STR_ASSET;
    static const wxString TYPE_STR_SHARES;
    static wxArrayString STATUS_STR;
    static const wxString STATUS_STR_OPEN;
    static const wxString STATUS_STR_CLOSED;

public:
    static const std::vector<std::pair<TYPE_ID, wxString> > TYPE_CHOICES;
    static const std::vector<std::pair<STATUS_ID, wxString> > STATUS_CHOICES;

private:
    static wxArrayString type_str_all();
    static wxArrayString status_str_all();

public:
    Model_Account();
    ~Model_Account();

public:
    /**
    Initialize the global Model_Account table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Account table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Account& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Account table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Account& instance();

public:
    /** Return the Data record for the given account name */
    Data* get(const wxString& name);
  
    /** Return the Data record for the given account num */
    Data* getByAccNum(const wxString& num);   

    static wxString get_account_name(int64 account_id);

    /** Remove the Data record from memory and the database. */
    bool remove(int64 id);

public:
    wxArrayString all_checking_account_names(bool skip_closed = false);
    const std::map<wxString, int64> all_accounts(bool skip_closed = false);

    static Model_Currency::Data* currency(const Data* r);
    static Model_Currency::Data* currency(const Data& r);

    static const Model_Checking::Data_Set transaction(const Data* r);
    static const Model_Checking::Data_Set transaction(const Data& r);

    static const Model_Billsdeposits::Data_Set billsdeposits(const Data* r);
    static const Model_Billsdeposits::Data_Set billsdeposits(const Data& r);

    static double balance(const Data* r);
    static double balance(const Data& r);

    static std::pair<double, double> investment_balance(const Data* r);
    static std::pair<double, double> investment_balance(const Data& r);
    static wxString toCurrency(double value, const Data* r);

    static wxString toString(double value, const Data* r, int precision = 2);
    static wxString toString(double value, const Data& r, int precision = 2);

    static STATUS_ID status_id(const Data* account);
    static STATUS_ID status_id(const Data& account);
    static DB_Table_ACCOUNTLIST_V1::STATUS STATUS(STATUS_ID status, OP op = EQUAL);

    static TYPE_ID type_id(const Data* account);
    static TYPE_ID type_id(const Data& account);

    static bool FAVORITEACCT(const Data* r);
    static bool FAVORITEACCT(const Data& r);

    static bool is_used(const Model_Currency::Data* c);
    static bool is_used(const Model_Currency::Data& c);

    static int money_accounts_num();

    static bool Exist(const wxString& account_name);

    static wxDateTime DateOf(const wxString& date_str);
    static bool BoolOf(int64 value);
    static bool is_positive(int value);
    static wxDateTime get_date_by_string(const wxString& date_str);

    const Data_Set FilterAccounts(const wxString& account_pattern, bool skip_closed = false);

};

inline wxDateTime Model_Account::get_date_by_string(const wxString& date_str) { return Model::to_date(date_str); }
inline bool Model_Account::is_positive(int value) { return value > 0 ? true : false; }

#endif // 
