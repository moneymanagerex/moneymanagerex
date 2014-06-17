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

#ifndef MODEL_STOCKHISTORY_H
#define MODEL_STOCKHISTORY_H

#include "Model.h"
#include "db/DB_Table_Stockhistory_V1.h"

class Model_StockHistory : public Model<DB_Table_STOCKHISTORY_V1>
{
public:
    using Model<DB_Table_STOCKHISTORY_V1>::get;
    enum UPDTYPE { ONLINE = 1, MANUAL };

public:
    Model_StockHistory();
    ~Model_StockHistory();

public:
    /**
    Initialize the global Model_StockHistory table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Stock table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_StockHistory& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_StockHistory table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_StockHistory& instance();

public:
    Data* get(int stock_id, const wxDate& date);
    static wxDate DATE(const Data& hist);

    /**
    Lists all stock history items for a given stock
    * Return the data set
    */
    Data_Set search(int stockId, bool asc = false, int limit = 0, const wxDate& startDate = wxDefaultDateTime, const wxDate& endDate = wxDefaultDateTime);

    /**
    Adds or updates an element in stock history
    */
    int addUpdate(int stockId, const wxDate& date, double price, UPDTYPE type);
};

#endif // 
