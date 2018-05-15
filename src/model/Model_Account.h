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

#ifndef MODEL_ACCOUNT_H
#define MODEL_ACCOUNT_H

#include "Model.h"
#include "Table_Accountlist.h"
#include "Model_Currency.h" // detect base currency
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"

class Model_Account : public Model<DB_Table_ACCOUNTLIST>
{
public:
    using Model<DB_Table_ACCOUNTLIST>::remove;
    using Model<DB_Table_ACCOUNTLIST>::get;

    enum STATUS_ENUM { OPEN = 0, CLOSED };
    enum TYPE { CASH = 0, CHECKING, CREDIT_CARD, LOAN, TERM, CRYPTO, INVESTMENT, ASSET, SHARES };

    static const std::vector<std::pair<STATUS_ENUM, wxString> > STATUS_CHOICES;
    static const std::vector<std::pair<TYPE, wxString> > TYPE_CHOICES;

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

    static wxString get_account_name(int account_id);

    /** Remove the Data record from memory and the database. */
    bool remove(int id);

public:
    wxArrayString all_checking_account_names(bool skip_closed = false);

    static wxArrayString all_status();
    static wxArrayString all_type();

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

    static STATUS_ENUM status(const Data* account);
    static STATUS_ENUM status(const Data& account);
    static DB_Table_ACCOUNTLIST::STATUS STATUS(STATUS_ENUM status, OP op = EQUAL);

    static TYPE type(const Data* account);
    static TYPE type(const Data& account);

    static bool FAVORITEACCT(const Data* r);
    static bool FAVORITEACCT(const Data& r);

    static bool is_used(const Model_Currency::Data* c);
    static bool is_used(const Model_Currency::Data& c);

    static int money_accounts_num();

    static bool Exist(const wxString& account_name);

    static wxDateTime DateOf(const wxString& date_str);
    static bool BoolOf(int value);
};

#endif // 
