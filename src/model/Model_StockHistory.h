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
#include "Table_Stockhistory.h"

class Model_StockHistory : public Model<DB_Table_STOCKHISTORY>
{
public:
    using Model<DB_Table_STOCKHISTORY>::get;
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
    Data* get(const wxString& symbol, const wxDate& date);
    static wxDate DATE(const Data& hist);

    static DB_Table_STOCKHISTORY::DATE DATE(const wxDate& date, OP op = EQUAL);
    /**
    Adds or updates an element in stock history
    */
    int addUpdate(const wxString& symbol, const wxDate& date, double price, UPDTYPE type);
};

#endif // 
