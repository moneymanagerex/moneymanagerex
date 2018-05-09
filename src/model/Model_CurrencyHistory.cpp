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

Model_CurrencyHistory::Model_CurrencyHistory()
: Model<DB_Table_CURRENCYHISTORY>()
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
    Data* hist = this->get_one(CURRENCYID(currencyID), DB_Table_CURRENCYHISTORY::CURRDATE(date.FormatISODate()));
    if (hist) return hist;

    Data_Set items = this->find(CURRENCYID(currencyID), DB_Table_CURRENCYHISTORY::CURRDATE(date.FormatISODate()));
    if (!items.empty()) hist = this->get(items[0].id(), this->db_);
    return hist;
}

wxDate Model_CurrencyHistory::CURRDATE(const Data& hist)
{
    return Model::to_date(hist.CURRDATE);
}

DB_Table_CURRENCYHISTORY::CURRDATE Model_CurrencyHistory::CURRDATE(const wxDate& date, OP op)
{
    return DB_Table_CURRENCYHISTORY::CURRDATE(date.FormatISODate(), op);
}

/**
Adds or updates an element in currency history
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

/** Return the rate for a specific currency in a specific day*/
double Model_CurrencyHistory::getDayRate(const int& currencyID, const wxString& DateISO)
{
    wxDate Date;
    if (Date.ParseDate(DateISO))
        return Model_CurrencyHistory::getDayRate(currencyID, Date);
    else
        return 1;
}

double Model_CurrencyHistory::getDayRate(const int& currencyID, const wxDate& Date)
{
    if (currencyID == Model_Currency::GetBaseCurrency()->CURRENCYID || currencyID == -1)
        return 1;
    
    Model_CurrencyHistory::Data_Set Data = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID),Model_CurrencyHistory::CURRDATE(Date));

    if (!Data.empty())
    {
        //Rate found for specified day
        return Data.back().CURRVALUE;
    }
    else if (Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID)).size() > 0)
    {
        //Rate not found for specified day
        //Custom query requested to speed-up performances, no way to obtain it with our ORM
        wxDateTime dFuture, dPast, dNearest;
        wxString DateISO = Date.FormatISODate();

        const wxString sqlPast = wxString::Format("SELECT MAX(currdate) FROM CURRENCYHISTORY WHERE currencyid = '%i' AND currdate <= '%s';", currencyID, DateISO);
        wxSQLite3ResultSet rsPast = Model_CurrencyHistory::instance().db_->ExecuteQuery(sqlPast);
        while (rsPast.NextRow())
        {
            dPast.ParseDate(rsPast.GetAsString(0));
        }

        const wxString sqlFuture = wxString::Format("SELECT MIN(currdate) FROM CURRENCYHISTORY WHERE currencyid = '%i' AND currdate >= '%s';", currencyID, DateISO);
        wxSQLite3ResultSet rsFuture = Model_CurrencyHistory::instance().db_->ExecuteQuery(sqlFuture);
        while (rsFuture.NextRow())
        {
            dFuture.ParseDate(rsFuture.GetAsString(0));
        }

        if (dPast.IsValid() && dFuture.IsValid())
        {
            const wxTimeSpan spanPast = Date.Subtract(dPast);
            const wxTimeSpan spanFuture = dFuture.Subtract(Date);

            dNearest = spanPast <= spanFuture ? dPast : dFuture;
        }
        else if (dPast.IsValid())
        {
            dNearest = dPast;
        }
        else if (dFuture.IsValid())
        {
            dNearest = dFuture;
        }
        else
        {
            //TODO: Show warning alert but only one time?
            return 1;
        }
        
        return Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID), Model_CurrencyHistory::CURRDATE(dNearest))[0].CURRVALUE;
    }
    else
    {
        return 1;
    }
}

/** Return the last currency rate */
double Model_CurrencyHistory::getLastRate(const int& currencyID)
{
    Model_CurrencyHistory::Data_Set histData = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID));
    std::stable_sort(histData.begin(), histData.end(), SorterByCURRDATE());

    if (!histData.empty())
        return histData.back().CURRVALUE;
    else
        return 1;
}

/** Return the last currency rate not after the date */
double Model_CurrencyHistory::getLastRate(const int& currencyID, const wxString& dateISO)
{
    Model_CurrencyHistory::Data_Set histData = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currencyID));
    if (histData.empty())
        return 1;

    std::stable_sort(histData.begin(), histData.end(), SorterByCURRDATE());
    Model_CurrencyHistory::Data date;
    date.CURRDATE = dateISO;
    auto lowerb = std::lower_bound(histData.begin(), histData.end(), date, SorterByCURRDATE());

    if (lowerb == histData.end())
        return histData.back().CURRVALUE;
    else if (lowerb->CURRDATE == dateISO)
        return lowerb->CURRVALUE;
    else if (lowerb==histData.begin())
        return 1;
    else
        return (lowerb-1)->CURRVALUE;
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
