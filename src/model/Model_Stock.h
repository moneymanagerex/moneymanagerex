/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#ifndef MODEL_STOCK_H
#define MODEL_STOCK_H

#include "Model.h"
#include "db/DB_Table_Stock_V1.h"
#include "Model_Account.h"

class Model_Stock : public Model<DB_Table_STOCK_V1>
{
public:
    using Model<DB_Table_STOCK_V1>::remove;
    Model_Stock();
    ~Model_Stock();

public:
    /**
    Initialize the global Model_Stock table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Stock table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Stock& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Stock table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Stock& instance();

public:
    static wxString get_stock_name(int stock_id);

    static wxDate PURCHASEDATE(const Data* stock);
    static wxDate PURCHASEDATE(const Data& stock);

    /** Original value of Stocks */
    static double InvestmentValue(const Data* r);
    /** Original value of Stocks */
    static double InvestmentValue(const Data& r);

    static double CurrentValue(const Data* r);
    static double CurrentValue(const Data& r);

public:
    /**
    * Remove the Data record from memory and the database.
    * Delete also all stock history
    */
    bool remove(int id);

    /**
    Returns the last price date of a given stock
    */
    wxString lastPriceDate(const Self::Data* entity);

    /**
    Returns the total stock balance at a given date
    */
    double getDailyBalanceAt(const Model_Account::Data *account, const wxDate& date);
};

#endif // 
