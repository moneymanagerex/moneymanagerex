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

#include "Model_StockHistory.h"
#include "Model_Stock.h"

Model_StockHistory::Model_StockHistory()
: Model<DB_Table_STOCKHISTORY_V1>()
{
}

Model_StockHistory::~Model_StockHistory()
{
}

/**
* Initialize the global Model_StockHistory table.
* Reset the Model_StockHistory table or create the table if it does not exist.
*/
Model_StockHistory& Model_StockHistory::instance(wxSQLite3Database* db)
{
    Model_StockHistory& ins = Singleton<Model_StockHistory>::instance();
    ins.db_ = db;
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_StockHistory table */
Model_StockHistory& Model_StockHistory::instance()
{
    return Singleton<Model_StockHistory>::instance();
}

Model_StockHistory::Data* Model_StockHistory::get(const wxString& symbol, const wxDate& date)
{
    Data* hist = this->get_one(SYMBOL(symbol), DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate()));
    if (hist) return hist;

    Data_Set items = this->find(SYMBOL(symbol), DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate()));
    if (!items.empty()) hist = this->get(items[0].id(), this->db_);
    return hist;
}

wxDate Model_StockHistory::DATE(const Data& hist)
{
    return Model::to_date(hist.DATE);
}

DB_Table_STOCKHISTORY_V1::DATE Model_StockHistory::DATE(const wxDate& date, OP op)
{
    return DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate(), op);
}

/**
Adds or updates an element in stock history
*/
int64 Model_StockHistory::addUpdate(const wxString& symbol, const wxDate& date, double price, UPDTYPE type)
{
    Data *stockHist = this->get(symbol, date);
    if (!stockHist) stockHist = this->create();

    stockHist->SYMBOL = symbol;
    stockHist->DATE = date.FormatISODate();
    stockHist->VALUE = price;
    stockHist->UPDTYPE = type;

    if (Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(symbol), Model_StockHistory::DATE(date, GREATER)).size() == 0) {
        Model_Stock::UpdateCurrentPrice(symbol, price);
    }

    return save(stockHist);
}
