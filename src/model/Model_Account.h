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

    enum STATUS { OPEN = 0, CLOSED };
    enum TYPE { CHECKING = 0, TERM, INVESTMENT };

    static const std::vector<std::pair<STATUS, wxString> > STATUS_CHOICES;
    static const std::vector<std::pair<TYPE, wxString> > TYPE_CHOICES;

public:
    Model_Account();
    ~Model_Account();

public:
    /**
    * Initialize the global Model_Account table.
    * Reset the Model_Account table or create the table if it does not exist.
    */
    static Model_Account& instance(wxSQLite3Database* db);

    /** Return the static instance of Model_Account table */
    static Model_Account& instance();

public:
    /** Get the Data record instance in memory. */
    Data* get(const wxString& name);
    
    /** Remove the Data record instance from memory and the database. */
    bool remove(int id);

public:
    wxArrayString all_account_names();
    wxArrayString all_checking_account_names();

    static wxArrayString all_status();
    static wxArrayString all_type();

    static Model_Currency::Data* currency(const Data* r);
    static Model_Currency::Data* currency(const Data& r);

    static Model_Checking::Data_Set transaction(const Data* r);
    static Model_Checking::Data_Set transaction(const Data& r);

    static Model_Billsdeposits::Data_Set billsdeposits(const Data* r);
    static Model_Billsdeposits::Data_Set billsdeposits(const Data& r);

    static wxDate last_date(const Data* r);
    static wxDate last_date(const Data& r);

    static double balance(const Data* r);
    static double balance(const Data& r);

    static std::pair<double, double> investment_balance(const Data* r);
    static std::pair<double, double> investment_balance(const Data& r);
    static wxString toCurrency(double value, const Data* r);

    static wxString toString(double value, const Data* r);
    static wxString toString(double value, const Data& r);

    static STATUS status(const Data* account);
    static STATUS status(const Data& account);

    static TYPE type(const Data* account);
    static TYPE type(const Data& account);

    static bool FAVORITEACCT(const Data* r);
    static bool FAVORITEACCT(const Data& r);

    static bool is_used(const Model_Currency::Data* c);
    static bool is_used(const Model_Currency::Data& c);

    static int checking_account_num();
    static bool hasActiveTermAccount();
};

#endif // 
