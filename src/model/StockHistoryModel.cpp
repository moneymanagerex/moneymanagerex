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

#include "StockModel.h"
#include "StockHistoryModel.h"

StockHistoryModel::StockHistoryModel()
: Model<DB_Table_STOCKHISTORY_V1>()
{
}

StockHistoryModel::~StockHistoryModel()
{
}

/**
* Initialize the global StockHistoryModel table.
* Reset the StockHistoryModel table or create the table if it does not exist.
*/
StockHistoryModel& StockHistoryModel::instance(wxSQLite3Database* db)
{
    StockHistoryModel& ins = Singleton<StockHistoryModel>::instance();
    ins.db_ = db;
    ins.ensure(db);

    return ins;
}

/** Return the static instance of StockHistoryModel table */
StockHistoryModel& StockHistoryModel::instance()
{
    return Singleton<StockHistoryModel>::instance();
}

StockHistoryModel::Data* StockHistoryModel::get(const wxString& symbol, const wxDate& date)
{
    Data* hist = this->get_one(SYMBOL(symbol), DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate()));
    if (hist) return hist;

    Data_Set items = this->find(SYMBOL(symbol), DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate()));
    if (!items.empty()) hist = this->get(items[0].id(), this->db_);
    return hist;
}

wxDate StockHistoryModel::DATE(const Data& hist)
{
    return parseDateTime(hist.DATE);
}

DB_Table_STOCKHISTORY_V1::DATE StockHistoryModel::DATE(const wxDate& date, OP op)
{
    return DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate(), op);
}

/**
Adds or updates an element in stock history
*/
int64 StockHistoryModel::addUpdate(const wxString& symbol, const wxDate& date, double price, UPDTYPE type)
{
    Data *stockHist = this->get(symbol, date);
    if (!stockHist) stockHist = this->create();

    stockHist->SYMBOL = symbol;
    stockHist->DATE = date.FormatISODate();
    stockHist->VALUE = price;
    stockHist->UPDTYPE = type;

    if (StockHistoryModel::instance().find(StockHistoryModel::SYMBOL(symbol), StockHistoryModel::DATE(date, GREATER)).size() == 0) {
        StockModel::UpdateCurrentPrice(symbol, price);
    }

    return save(stockHist);
}
