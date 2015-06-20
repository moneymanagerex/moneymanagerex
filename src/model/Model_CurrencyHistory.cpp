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

#include "Model_CurrencyHistory.h"

Model_CurrencyHistory::Model_CurrencyHistory()
: Model<DB_Table_CURRENCYHISTORY_V1>()
{
};

Model_CurrencyHistory::~Model_CurrencyHistory()
{
};

/**
* Initialize the global Model_CurrencyHistory table.
* Reset the Model_CurrencyHistory table or create the table if it does not exist.
*/
Model_CurrencyHistory& Model_CurrencyHistory::instance(wxSQLite3Database* db)
{
    Model_CurrencyHistory& ins = Singleton<Model_CurrencyHistory>::instance();
    ins.db_ = db;
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_CurrencyHistory table */
Model_CurrencyHistory& Model_CurrencyHistory::instance()
{
    return Singleton<Model_CurrencyHistory>::instance();
}

Model_CurrencyHistory::Data* Model_CurrencyHistory::get(const int& currencyID, const wxDate& date)
{
    Data* hist = this->get_one(CURRENCYID(currencyID), DB_Table_CURRENCYHISTORY_V1::CURRDATE(date.FormatISODate()));
    if (hist) return hist;

    Data_Set items = this->find(CURRENCYID(currencyID), DB_Table_CURRENCYHISTORY_V1::CURRDATE(date.FormatISODate()));
    if (!items.empty()) hist = this->get(items[0].id(), this->db_);
    return hist;
}

wxDate Model_CurrencyHistory::CURRDATE(const Data& hist)
{
    return Model::to_date(hist.CURRDATE);
}

DB_Table_CURRENCYHISTORY_V1::CURRDATE Model_CurrencyHistory::CURRDATE(const wxDate& date, OP op)
{
    return DB_Table_CURRENCYHISTORY_V1::CURRDATE(date.FormatISODate(), op);
}

/**
Adds or updates an element in stock history
*/
int Model_CurrencyHistory::addUpdate(const int& currencyID, const wxDate& date, double price, UPDTYPE type)
{
    Data *currHist = this->get(currencyID, date);
    if (!currHist) currHist = this->create();

    currHist->CURRENCYID = currencyID;
    currHist->CURRDATE = date.FormatISODate();
    currHist->CURRVALUE = price;
    currHist->CURRUPDTYPE = type;
    return save(currHist);
}
