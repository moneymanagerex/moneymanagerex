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
#include "PrefModel.h"

CurrencyHistoryModel::CurrencyHistoryModel() :
    TableFactory<CurrencyHistoryTable, CurrencyHistoryData>()
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
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

/** Return the static instance of CurrencyHistoryModel table */
CurrencyHistoryModel& CurrencyHistoryModel::instance()
{
    return Singleton<CurrencyHistoryModel>::instance();
}

const CurrencyHistoryData* CurrencyHistoryModel::get_key(const int64& currencyID, const wxDate& date)
{
    const Data* ch_n = search_cache_n(
        CurrencyHistoryCol::CURRENCYID(currencyID),
        CurrencyHistoryCol::CURRDATE(date.FormatISODate())
    );
    if (ch_n)
        return ch_n;

    DataA items = this->find(
        CurrencyHistoryCol::CURRENCYID(currencyID),
        CurrencyHistoryCol::CURRDATE(date.FormatISODate())
    );
    if (!items.empty())
        ch_n = get_id_data_n(items[0].id());
    return ch_n;
}

wxDate CurrencyHistoryModel::CURRDATE(const Data& ch_d)
{
    return ch_d.m_date.getDateTime();
}

CurrencyHistoryCol::CURRDATE CurrencyHistoryModel::CURRDATE(OP op, const wxDate& date)
{
    return CurrencyHistoryCol::CURRDATE(op, date.FormatISODate());
}

// Adds or updates an element in stock history
int64 CurrencyHistoryModel::addUpdate(
    const int64 currencyID,
    const wxDate& date,
    double price,
    UPDTYPE type
) {
    const Data *ch_n = get_key(currencyID, date);
    Data ch_d = ch_n ? *ch_n : Data();
    ch_d.m_currency_id     = currencyID;
    ch_d.m_date            = mmDate(date);
    ch_d.m_base_conv_rate  = price;
    ch_d.m_update_type     = UpdateType(type);
    save_data_n(ch_d);
    return ch_d.id();
}

/** Return the rate for a specific currency in a specific day*/
double CurrencyHistoryModel::getDayRate(int64 currencyID, const wxString& iso_date)
{
    if (!PrefModel::instance().getUseCurrencyHistory()) {
        const CurrencyData* c = CurrencyModel::instance().get_id_data_n(currencyID);
        return c ? c->m_base_conv_rate : 1.0;
    }
    wxDate Date;
    if (Date.ParseDate(iso_date))
        return CurrencyHistoryModel::getDayRate(currencyID, Date);
    else {
        wxASSERT(false);
        return 1;
    }
}

double CurrencyHistoryModel::getDayRate(int64 currency_id_n, const wxDate& date)
{
    if (currency_id_n == CurrencyModel::GetBaseCurrency()->m_id || currency_id_n == -1)
        return 1;

    const CurrencyData* currency_n = CurrencyModel::instance().get_id_data_n(currency_id_n);
    if (!PrefModel::instance().getUseCurrencyHistory())
        return currency_n->m_base_conv_rate;

    CurrencyHistoryModel::DataA ch_a = CurrencyHistoryModel::instance().find(
        CurrencyHistoryCol::CURRENCYID(OP_EQ, currency_id_n),
        CurrencyHistoryModel::CURRDATE(OP_EQ, date)
    );
    if (!ch_a.empty()) {
        // Rate found for specified day
        return ch_a.back().m_base_conv_rate;
    }
    if (CurrencyHistoryModel::instance().find(
        CurrencyHistoryCol::CURRENCYID(currency_id_n)
    ).empty()) {
        return currency_n->m_base_conv_rate;
    }

    // Rate not found for specified day, look at previous and next
    // FIXME: sort by date
    CurrencyHistoryModel::DataA prev_ch_a = CurrencyHistoryModel::instance().find(
        CurrencyHistoryCol::CURRENCYID(currency_id_n),
        CurrencyHistoryModel::CURRDATE(OP_LE, date)
    );
    CurrencyHistoryModel::DataA next_ch_a = CurrencyHistoryModel::instance().find(
        CurrencyHistoryCol::CURRENCYID(currency_id_n),
        CurrencyHistoryModel::CURRDATE(OP_GE, date)
    );

    if (!prev_ch_a.empty() && !next_ch_a.empty()) {
        const wxTimeSpan prev_span = date.Subtract(prev_ch_a.back().m_date.getDateTime());
        const wxTimeSpan next_span = next_ch_a[0].m_date.getDateTime().Subtract(date);
        return prev_span <= next_span
            ? prev_ch_a.back().m_base_conv_rate
            : next_ch_a[0].m_base_conv_rate;
    }
    else if (!prev_ch_a.empty()) {
        return prev_ch_a.back().m_base_conv_rate;
    }
    else if (!next_ch_a.empty()) {
        return next_ch_a[0].m_base_conv_rate;
    }

    return currency_n->m_base_conv_rate;
}

/** Return the last rate for specified currency */
double CurrencyHistoryModel::getLastRate(const int64& currencyID)
{
    if (!PrefModel::instance().getUseCurrencyHistory())
        return CurrencyModel::instance().get_id_data_n(currencyID)->m_base_conv_rate;

    CurrencyHistoryModel::DataA histData = CurrencyHistoryModel::instance().find(
        CurrencyHistoryCol::CURRENCYID(currencyID)
    );
    std::stable_sort(histData.begin(), histData.end(), CurrencyHistoryData::SorterByCURRDATE());

    if (!histData.empty())
        return histData.back().m_base_conv_rate;
    else
    {
        const CurrencyData* currency_n = CurrencyModel::instance().get_id_data_n(currencyID);
        return currency_n->m_base_conv_rate;
    }
}

void CurrencyHistoryModel::ResetCurrencyHistory()
{
    CurrencyHistoryModel::instance().db_savepoint();
    for (const auto& r : CurrencyHistoryModel::instance().find_all()) {
        CurrencyHistoryModel::instance().purge_id(r.id());
    }
    CurrencyHistoryModel::instance().db_release_savepoint();
}
