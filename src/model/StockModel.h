/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#pragma once

#include "base/defs.h"

#include "table/StockTable.h"
#include "data/StockData.h"

#include "_ModelBase.h"
#include "AccountModel.h"

class StockModel : public TableFactory<StockTable, StockData>
{
public:
    static const RefTypeN s_ref_type;

public:
    StockModel();
    ~StockModel();

public:
    static StockModel& instance(wxSQLite3Database* db);
    static StockModel& instance();

public:
    // override
    bool purge_id(int64 id) override;

    auto get_id_name(int64 stock_id) -> const wxString;
    auto find_last_hist_date(const Data& stock_d) -> const mmDate;

    auto calculate_account_balance(const AccountData& account_d, const mmDate& date) -> double;
    auto calculate_realized_gain(const Data& stock_d, bool base_curr = false) -> double;
    auto calculate_unrealiazed_gain(const Data& stock_d, bool base_curr = false) -> double;

    void update_symbol_current_price(const wxString& symbol, double price = -1);
    void update_data_position(Data* stock_n);
};

