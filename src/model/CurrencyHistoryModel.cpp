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

#include "CurrencyModel.h"
#include "CurrencyHistoryModel.h"
#include "PreferencesModel.h"

CurrencyHistoryModel::CurrencyHistoryModel()
    : Model<DB_Table_CURRENCYHISTORY_V1>()
{
}

CurrencyHistoryModel::~CurrencyHistoryModel()
{
}

/**
* Initialize the global CurrencyHistoryModel table.
* Reset the CurrencyHistoryModel table or create the table if it does not exist.
*/
CurrencyHistoryModel& CurrencyHistoryModel::instance(wxSQLite3Database* db)
{
    CurrencyHistoryModel& ins = Singleton<CurrencyHistoryModel>::instance();
    ins.db_ = db;
    ins.ensure(db);

    return ins;
}

/** Return the static instance of CurrencyHistoryModel table */
CurrencyHistoryModel& CurrencyHistoryModel::instance()
{
    return Singleton<CurrencyHistoryModel>::instance();
}

CurrencyHistoryModel::Data* CurrencyHistoryModel::get(const int64& currencyID, const wxDate& date)
{
    Data* hist = this->get_one(CURRENCYID(currencyID), DB_Table_CURRENCYHISTORY_V1::CURRDATE(date.FormatISODate()));
    if (hist) return hist;

    Data_Set items = this->find(CURRENCYID(currencyID), DB_Table_CURRENCYHISTORY_V1::CURRDATE(date.FormatISODate()));
    if (!items.empty()) hist = this->get(items[0].id(), this->db_);
    return hist;
}

wxDate CurrencyHistoryModel::CURRDATE(const Data& hist)
{
    return parseDateTime(hist.CURRDATE);
}

DB_Table_CURRENCYHISTORY_V1::CURRDATE CurrencyHistoryModel::CURRDATE(const wxDate& date, OP op)
{
    return DB_Table_CURRENCYHISTORY_V1::CURRDATE(date.FormatISODate(), op);
}

/**
Adds or updates an element in stock history
*/
int64 CurrencyHistoryModel::addUpdate(const int64 currencyID, const wxDate& date, double price, UPDTYPE type)
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
double CurrencyHistoryModel::getDayRate(int64 currencyID, const wxString& DateISO)
{
    if (!PreferencesModel::instance().getUseCurrencyHistory()) {
        auto c = CurrencyModel::instance().get(currencyID);
        return c ? c->BASECONVRATE : 1.0;
    }
    wxDate Date;
    if (Date.ParseDate(DateISO))
        return CurrencyHistoryModel::getDayRate(currencyID, Date);
    else
    {
        wxASSERT(false);
        return 1;
    }
}

double CurrencyHistoryModel::getDayRate(int64 currencyID, const wxDate& Date)
{
    if (currencyID == CurrencyModel::GetBaseCurrency()->CURRENCYID || currencyID == -1)
        return 1;

    if (!PreferencesModel::instance().getUseCurrencyHistory())
        return CurrencyModel::instance().get(currencyID)->BASECONVRATE;

    CurrencyHistoryModel::Data_Set Data = CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(currencyID), CurrencyHistoryModel::CURRDATE(Date));
    if (!Data.empty())
    {
        //Rate found for specified day
        return Data.back().CURRVALUE;
    }
    else if (CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(currencyID)).size() > 0)
    {
        //Rate not found for specified day, look at previous and next
        CurrencyHistoryModel::Data_Set DataPrevious = CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(currencyID), CurrencyHistoryModel::CURRDATE(Date, LESS_OR_EQUAL));
        CurrencyHistoryModel::Data_Set DataNext = CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(currencyID), CurrencyHistoryModel::CURRDATE(Date, GREATER_OR_EQUAL));

        if (!DataPrevious.empty() && !DataNext.empty())
        {
            const wxTimeSpan spanPast = Date.Subtract(parseDateTime(DataPrevious.back().CURRDATE));
            const wxTimeSpan spanFuture = parseDateTime(DataNext[0].CURRDATE).Subtract(Date);

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

    return CurrencyModel::instance().get(currencyID)->BASECONVRATE;
}

/** Return the last rate for specified currency */
double CurrencyHistoryModel::getLastRate(const int64& currencyID)
{
    if (!PreferencesModel::instance().getUseCurrencyHistory())
        return CurrencyModel::instance().get(currencyID)->BASECONVRATE;

    CurrencyHistoryModel::Data_Set histData = CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(currencyID));
    std::stable_sort(histData.begin(), histData.end(), SorterByCURRDATE());

    if (!histData.empty())
        return histData.back().CURRVALUE;
    else
    {
        CurrencyModel::Data* Currency = CurrencyModel::instance().get(currencyID);
        return Currency->BASECONVRATE;
    }
}

void CurrencyHistoryModel::ResetCurrencyHistory()
{
    CurrencyHistoryModel::instance().Savepoint();
    for (const auto& r : CurrencyHistoryModel::instance().all())
    {
        CurrencyHistoryModel::instance().remove(r.id());
    }
    CurrencyHistoryModel::instance().ReleaseSavepoint();
}
