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

#ifndef MODEL_STOCK_H
#define MODEL_STOCK_H

#include "Model.h"
#include "db/DB_Table_Stock_V1.h"
#include "Model_Currency.h" // detect base currency

class Model_Stock : public Model_Mix<DB_Table_STOCK_V1>
{
public:
    Model_Stock(): Model_Mix<DB_Table_STOCK_V1>() {};
    ~Model_Stock() {};

public:
    /** Return the static instance of Model_Stock table */
    static Model_Stock& instance()
    {
        return Singleton<Model_Stock>::instance();
    }

    /**
    * Initialize the global Model_Stock table.
    * Reset the Model_Stock table or create the table if it does not exist.
    */
    static Model_Stock& instance(wxSQLite3Database* db)
    {
        Model_Stock& ins = Singleton<Model_Stock>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.ensure(db);

        return ins;
    }
public:
    static wxDate PURCHASEDATE(const Data* stock) { return Model::to_date(stock->PURCHASEDATE); }
    static wxDate PURCHASEDATE(const Data& stock) { return Model::to_date(stock.PURCHASEDATE); }
public:
    static double value(const Data* r)
    {
        return r->NUMSHARES * r->PURCHASEPRICE - r->COMMISSION;
    }
    static double value(const Data& r) { return value(&r); }
    static wxString NUMSHARES(const Data* stock)
    {
        wxString numSharesStr;
        double numShares = stock->NUMSHARES;
        if ((numShares - static_cast<long>(numShares)) != 0.0)
            numSharesStr = wxString::Format("%.4f", numShares);
        else
            numSharesStr << static_cast<long>(numShares);
        return numSharesStr;
    }
    static wxString NUMSHARES(const Data& stock) { return NUMSHARES(&stock); }
};

#endif // 
