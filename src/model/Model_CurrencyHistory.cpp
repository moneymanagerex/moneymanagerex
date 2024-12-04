/*******************************************************
Copyright (C) 2015 Gabriele-V

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
#include "Model_Currency.h"
#include "option.h"

Model_CurrencyHistory::Model_CurrencyHistory()
    : Model<DB_Table_CURRENCYHISTORY_V1>()
{
}

Model_CurrencyHistory::~Model_CurrencyHistory()
{
}

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

Model_CurrencyHistory::Data* Model_CurrencyHistory::get(const int64& currencyID, const wxDate& date)
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
int64 Model_CurrencyHistory::addUpdate(const int64 currencyID, const wxDate& date, double price, UPDTYPE type)
{
    Data *currHist = this->get(currencyID, date);
    if (!currHist) currHist = this->create();

    currHist->CURRENCYID = currencyID;
    currHist->CURRDATE = date.FormatISODate();
    currHist->CURRVALUE = price;
    currHist->CURRUPDTYPE = type;
    return save(currHist);
}

/** Return the rate for a specific currency in a specific day*/
double Model_CurrencyHistory::getDayRate(int64 currencyID, const wxString& DateISO)
{
    if (!Option::instance().getCurrencyHistoryEnabled()) {
        auto c = Model_Currency::instance().get(currencyID);
        return c ? c->BASECONVRATE : 1.0;
    }
    wxDate Date;
    if (Date.ParseDate(DateISO))
        return Model_CurrencyHistory::getDayRate(currencyID, Date);
    else
    {
        wxASSERT(false);
        return 1;
    }
}

double Model_CurrencyHistory::getDayRate(int64 currencyID, const wxDate& Date)
{
    if (currencyID == Model_Currency::GetBaseCurrency()->CURRENCYID || currencyID == -1)
        return 1;

    if (!Option::instance().getCurrencyHistoryEnabled())
        return Model_Currency::instance().get(currencyID)->BASECONVRATE;

    Model_CurrencyHistory::Data_Set Data = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID), Model_CurrencyHistory::CURRDATE(Date));
    if (!Data.empty())
    {
        //Rate found for specified day
        return Data.back().CURRVALUE;
    }
    else if (Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID)).size() > 0)
    {
        //Rate not found for specified day, look at previous and next
        Model_CurrencyHistory::Data_Set DataPrevious = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID), Model_CurrencyHistory::CURRDATE(Date, LESS_OR_EQUAL));
        Model_CurrencyHistory::Data_Set DataNext = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID), Model_CurrencyHistory::CURRDATE(Date, GREATER_OR_EQUAL));

        if (!DataPrevious.empty() && !DataNext.empty())
        {
            const wxTimeSpan spanPast = Date.Subtract(Model::to_date(DataPrevious.back().CURRDATE));
            const wxTimeSpan spanFuture = Model::to_date(DataNext[0].CURRDATE).Subtract(Date);

            return spanPast <= spanFuture ? DataPrevious.back().CURRVALUE : DataNext[0].CURRVALUE;
        }
        else if (!DataPrevious.empty())
        {
            return DataPrevious.back().CURRVALUE;
        }
        else if (!DataNext.empty())
        {
            return DataNext[0].CURRVALUE;
        }
    }

    return Model_Currency::instance().get(currencyID)->BASECONVRATE;
}

/** Return the last rate for specified currency */
double Model_CurrencyHistory::getLastRate(const int64& currencyID)
{
    if (!Option::instance().getCurrencyHistoryEnabled())
        return Model_Currency::instance().get(currencyID)->BASECONVRATE;

    Model_CurrencyHistory::Data_Set histData = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID));
    std::stable_sort(histData.begin(), histData.end(), SorterByCURRDATE());

    if (!histData.empty())
        return histData.back().CURRVALUE;
    else
    {
        Model_Currency::Data* Currency = Model_Currency::instance().get(currencyID);
        return Currency->BASECONVRATE;
    }
}

void Model_CurrencyHistory::ResetCurrencyHistory()
{
    Model_CurrencyHistory::instance().Savepoint();
    for (const auto& r : Model_CurrencyHistory::instance().all())
    {
        Model_CurrencyHistory::instance().remove(r.id());
    }
    Model_CurrencyHistory::instance().ReleaseSavepoint();
}
