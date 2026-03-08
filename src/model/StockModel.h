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

    static wxString get_id_name(int64 stock_id);

    static wxDate PURCHASEDATE(const Data& stock_d);

    static double InvestmentValue(const Data& stock_d);
    static double CurrentValue(const Data& stock_d);

    static double RealGainLoss(const Data& stock_d, bool base_curr = false);
    static double UnrealGainLoss(const Data& stock_d, bool base_curr = false);

    static void UpdateCurrentPrice(const wxString& symbol, const double price = -1);

    wxString lastPriceDate(const Data& stock_d);
    double getDailyBalanceAt(const AccountData& account_d, const wxDate& date);

    /*
    stock_entry.m_purchase_price = avg price of shares purchased.
    stock_entry.m_num_shares = total amount of shares purchased.
    stock_entry.VALUE     = value of shares based on:
    ... share_entry.SHARENUMBER * share_entry.SHAREPRICE
    */
    static void UpdatePosition(Data* stock_n);
};

