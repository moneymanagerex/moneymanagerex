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

#include "Model_Stock.h"

Model_Stock::Model_Stock()
: Model<DB_Table_STOCK_V1>()
{
}

Model_Stock::~Model_Stock()
{
}

/**
* Initialize the global Model_Stock table.
* Reset the Model_Stock table or create the table if it does not exist.
*/
Model_Stock& Model_Stock::instance(wxSQLite3Database* db)
{
    Model_Stock& ins = Singleton<Model_Stock>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Stock table */
Model_Stock& Model_Stock::instance()
{
    return Singleton<Model_Stock>::instance();
}

wxDate Model_Stock::PURCHASEDATE(const Data* stock)
{
    return Model::to_date(stock->PURCHASEDATE);
}

wxDate Model_Stock::PURCHASEDATE(const Data& stock)
{
    return Model::to_date(stock.PURCHASEDATE);
}

double Model_Stock::value(const Data* r)
{
    return r->NUMSHARES * r->PURCHASEPRICE - r->COMMISSION;
}

double Model_Stock::value(const Data& r)
{
    return value(&r);
}

double Model_Stock::NUMSHARES(const Data* stock)
{
    double numShares = stock->NUMSHARES;
    return numShares;
}

double Model_Stock::NUMSHARES(const Data& stock)
{
    return NUMSHARES(&stock);
}
