/*******************************************************
 Copyright (C) 2013 Guan Lisheng

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

class Model_Stock : public Model, public DB_Table_STOCK_V1
{
    using DB_Table_STOCK_V1::all;
    using DB_Table_STOCK_V1::get;
    using DB_Table_STOCK_V1::save;
    using DB_Table_STOCK_V1::remove;
public:
    Model_Stock(): Model(), DB_Table_STOCK_V1() {};
    ~Model_Stock() {};

public:
    static Model_Stock& instance()
    {
        return Singleton<Model_Stock>::instance();
    }
    static Model_Stock& instance(wxSQLite3Database* db)
    {
        Model_Stock& ins = Singleton<Model_Stock>::instance();
        ins.db_ = db;
        ins.all();
        return ins;
    }
public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
    template<typename... Args>
    Data_Set find(const Args&... args)
    {
        return find_by(this, db_, true, args...);
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    bool remove(int id)
    {
        return this->remove(id, db_);
    }

public:
    static wxDate PURCHASEDATE(const Data* stock) { return Model::to_date(stock->PURCHASEDATE); }
    static wxDate PURCHASEDATE(const Data& stock) { return Model::to_date(stock.PURCHASEDATE); }
public:
    static double value(const Data* r)
    {
        return r->NUMSHARES * r->PURCHASEPRICE + r->COMMISSION;
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
