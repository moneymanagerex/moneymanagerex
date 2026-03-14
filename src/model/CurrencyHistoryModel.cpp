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

// Initialize the global CurrencyHistoryModel table.
// Reset the CurrencyHistoryModel table or create the table if it does not exist.
CurrencyHistoryModel& CurrencyHistoryModel::instance(wxSQLite3Database* db)
{
    CurrencyHistoryModel& ins = Singleton<CurrencyHistoryModel>::instance();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of CurrencyHistoryModel table
CurrencyHistoryModel& CurrencyHistoryModel::instance()
{
    return Singleton<CurrencyHistoryModel>::instance();
}

CurrencyHistoryCol::CURRDATE CurrencyHistoryModel::CURRDATE(OP op, const mmDate& date)
{
    return CurrencyHistoryCol::CURRDATE(op, date.isoDate());
}

const CurrencyHistoryData* CurrencyHistoryModel::get_key_data_n(
    int64 currency_id,
    const mmDate& date
) {
    const Data* uh_n = search_cache_n(
        CurrencyHistoryCol::CURRENCYID(currency_id),
        CurrencyHistoryModel::CURRDATE(OP_EQ, date)
    );
    if (uh_n)
        return uh_n;

    const DataA uh_a = find(
        CurrencyHistoryCol::CURRENCYID(currency_id),
        CurrencyHistoryModel::CURRDATE(OP_EQ, date)
    );
    if (!uh_a.empty())
        uh_n = get_id_data_n(uh_a[0].m_id);
    return uh_n;
}

double CurrencyHistoryModel::get_id_date_rate(int64 currency_id_n, const mmDate& date)
{
    if (currency_id_n == CurrencyModel::instance().get_base_data_n()->m_id ||
        currency_id_n == -1
    )
        return 1.0;

    const CurrencyData* currency_n = CurrencyModel::instance().get_id_data_n(currency_id_n);
    if (!PrefModel::instance().getUseCurrencyHistory())
        return currency_n->m_base_conv_rate;

    const DataA uh_a = CurrencyHistoryModel::instance().find(
        CurrencyHistoryCol::CURRENCYID(OP_EQ, currency_id_n),
        CurrencyHistoryModel::CURRDATE(OP_EQ, date)
    );
    if (!uh_a.empty()) {
        // Rate found for specified day
        return uh_a.back().m_base_conv_rate;
    }
    if (find(
        CurrencyHistoryCol::CURRENCYID(currency_id_n)
    ).empty()) {
        return currency_n->m_base_conv_rate;
    }

    // Rate not found for specified day, look at previous and next
    // FIXME: sort by date
    const CurrencyHistoryModel::DataA prev_uh_a = find(
        CurrencyHistoryCol::CURRENCYID(currency_id_n),
        CurrencyHistoryModel::CURRDATE(OP_LE, date)
    );
    CurrencyHistoryModel::DataA next_uh_a = find(
        CurrencyHistoryCol::CURRENCYID(currency_id_n),
        CurrencyHistoryModel::CURRDATE(OP_GE, date)
    );

    if (!prev_uh_a.empty() && !next_uh_a.empty()) {
        const wxTimeSpan prev_span = date.getDateTime().
            Subtract(prev_uh_a.back().m_date.getDateTime());
        const wxTimeSpan next_span = next_uh_a[0].m_date.getDateTime().
            Subtract(date.getDateTime());
        return prev_span <= next_span
            ? prev_uh_a.back().m_base_conv_rate
            : next_uh_a[0].m_base_conv_rate;
    }
    else if (!prev_uh_a.empty()) {
        return prev_uh_a.back().m_base_conv_rate;
    }
    else if (!next_uh_a.empty()) {
        return next_uh_a[0].m_base_conv_rate;
    }

    return currency_n->m_base_conv_rate;
}

// Return the last rate for specified currency
double CurrencyHistoryModel::get_id_last_rate(int64 currency_id)
{
    if (!PrefModel::instance().getUseCurrencyHistory())
        return CurrencyModel::instance().get_id_data_n(currency_id)->m_base_conv_rate;

    DataA uh_a = find(
        CurrencyHistoryCol::CURRENCYID(currency_id)
    );
    std::stable_sort(uh_a.begin(), uh_a.end(),
        CurrencyHistoryData::SorterByCURRDATE()
    );

    if (!uh_a.empty())
        return uh_a.back().m_base_conv_rate;
    else {
        const CurrencyData* currency_n = CurrencyModel::instance().get_id_data_n(currency_id);
        return currency_n->m_base_conv_rate;
    }
}

// Adds or updates an element in currency history
int64 CurrencyHistoryModel::save_record(
    int64 currency_id,
    const mmDate& date,
    double price,
    UpdateType update_type
) {
    const Data *uh_n = get_key_data_n(currency_id, date);
    Data uh_d = uh_n ? *uh_n : Data();
    uh_d.m_currency_id    = currency_id;
    uh_d.m_date           = date;
    uh_d.m_base_conv_rate = price;
    uh_d.m_update_type    = update_type;
    save_data_n(uh_d);
    return uh_d.m_id;
}

// Clears the currency History table
void CurrencyHistoryModel::purge_all()
{
    db_savepoint();
    for (const auto& uh_d : find_all()) {
        purge_id(uh_d.m_id);
    }
    db_release_savepoint();
}
