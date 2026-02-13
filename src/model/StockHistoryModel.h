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

#pragma once

#include "base/defs.h"
#include "_Model.h"
#include "db/DB_Table_Stockhistory_V1.h"

class StockHistoryModel : public Model<DB_Table_STOCKHISTORY_V1>
{
public:
    using Model<DB_Table_STOCKHISTORY_V1>::get;
    enum UPDTYPE { ONLINE = 1, MANUAL };

public:
    StockHistoryModel();
    ~StockHistoryModel();

public:
    /**
    Initialize the global StockHistoryModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for StockModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static StockHistoryModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for StockHistoryModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static StockHistoryModel& instance();

public:
    Data* get(const wxString& symbol, const wxDate& date);
    static wxDate DATE(const Data& hist);

    static DB_Table_STOCKHISTORY_V1::DATE DATE(const wxDate& date, OP op = EQUAL);
    /**
    Adds or updates an element in stock history
    */
    int64 addUpdate(const wxString& symbol, const wxDate& date, double price, UPDTYPE type);
};

