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

StockHistoryModel::StockHistoryModel() :
    TableFactory<StockHistoryTable, StockHistoryData>()
{
}

StockHistoryModel::~StockHistoryModel()
{
}

// Initialize the global StockHistoryModel table.
// Reset the StockHistoryModel table or create the table if it does not exist.
StockHistoryModel& StockHistoryModel::instance(wxSQLite3Database* db)
{
    StockHistoryModel& ins = Singleton<StockHistoryModel>::instance();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of StockHistoryModel table
StockHistoryModel& StockHistoryModel::instance()
{
    return Singleton<StockHistoryModel>::instance();
}

StockHistoryCol::DATE StockHistoryModel::DATE(OP op, const mmDate& date)
{
    return StockHistoryCol::DATE(op, date.isoDate());
}

const StockHistoryData* StockHistoryModel::get_key_data_n(
    const wxString& symbol,
    const mmDate& date
) {
    const Data* sh_n = search_cache_n(
        StockHistoryCol::SYMBOL(symbol),
        StockHistoryModel::DATE(OP_EQ, date)
    );
    if (sh_n)
        return sh_n;

    const DataA sh_a = this->find(
        StockHistoryCol::SYMBOL(symbol),
        StockHistoryModel::DATE(OP_EQ, date)
    );
    if (!sh_a.empty())
        sh_n = get_id_data_n(sh_a[0].m_id);
    return sh_n;
}

// Add or update an element in stock history
int64 StockHistoryModel::save_record(
    const wxString& symbol,
    const mmDate& date,
    double price,
    UpdateType update_type
) {
    const Data* sh_n = get_key_data_n(symbol, date);
    Data sh_d = sh_n ? *sh_n : Data();
    sh_d.m_symbol      = symbol;
    sh_d.m_date        = date;
    sh_d.m_price       = price;
    sh_d.m_update_type = update_type;

    if (find(
        StockHistoryCol::SYMBOL(symbol),
        StockHistoryModel::DATE(OP_GT, date)
    ).size() == 0) {
        StockModel::instance().update_symbol_current_price(symbol, price);
    }

    save_data_n(sh_d);
    return sh_d.m_id;
}
