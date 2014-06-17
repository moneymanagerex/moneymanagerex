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

Model_StockHistory::Model_StockHistory()
: Model<DB_Table_STOCKHISTORY_V1>()
{
};

Model_StockHistory::~Model_StockHistory()
{
};

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

/**
Lists all stock history items for a given stock
* Return the data set
*/
Model_StockHistory::Data_Set Model_StockHistory::search(int stockId, bool asc/* = false*/, int limit/* = 0*/, const wxDate& startDate/* = wxDefaultDateTime*/, const wxDate& endDate/* = wxDefaultDateTime*/)
{
    Data_Set result;

    if (startDate.IsValid() && !endDate.IsValid())
        result = find(DB_Table_STOCKHISTORY_V1::STOCKID(stockId), DB_Table_STOCKHISTORY_V1::DATE(startDate.FormatISODate()));
    else
        result = find(DB_Table_STOCKHISTORY_V1::STOCKID(stockId));

    if (!result.empty())
    {
        std::stable_sort(result.begin(), result.end(), SorterByDATE());
        if (!asc)
            std::reverse(result.begin(), result.end());

        // limit results: by date...
        if (startDate.IsValid() && endDate.IsValid())
        {
            int ind = 0, start = -2, end = -2;
            wxString strStartDate = startDate.FormatISODate(), strEndDate = endDate.FormatISODate();
            for (const auto &d : result)
            {
                if (start == -2 && d.DATE >= strStartDate)
                    start = ind;
                if (end == -2 && d.DATE > strEndDate)
                    end = ind;
                ind++;
            }
            // remove tail elements first
            if (end >= 0)
                result.erase(result.begin() + end, result.end());
            // then head elements
            if (start >= 0)
                result.erase(result.begin(), result.begin() + start);
        }
        // ... then by number of records
        if (limit > 0 && (int)result.size() > limit)
            result.resize(limit);

        result.shrink_to_fit();
    }

    return result;
}

Model_StockHistory::Data* Model_StockHistory::get(int stock_id, const wxDate& date)
{
    Data* hist = this->get_one(STOCKID(stock_id), DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate()));
    if (hist) return hist;

    Data_Set items = this->find(STOCKID(stock_id), DB_Table_STOCKHISTORY_V1::DATE(date.FormatISODate()));
    if (!items.empty()) hist = this->get(items[0].id(), this->db_);
    return hist;
}

wxDate Model_StockHistory::DATE(const Data& hist)
{
    return Model::to_date(hist.DATE);
}

/**
Adds or updates an element in stock history
*/
int Model_StockHistory::addUpdate(int stockId, const wxDate& date, double price, UPDTYPE type)
{
    Data *stockHist = this->get(stockId, date);
    if (!stockHist) stockHist = this->create();

    stockHist->STOCKID = stockId;
    stockHist->DATE = date.FormatISODate();
    stockHist->VALUE = price;
    stockHist->UPDTYPE = type;
    return save(stockHist);
}
