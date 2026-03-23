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

#pragma once

#include "base/defs.h"
#include "util/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/CurrencyHistoryData.h"

class CurrencyHistoryModel : public TableFactory<CurrencyHistoryTable, CurrencyHistoryData>
{
public:
    CurrencyHistoryModel();
    ~CurrencyHistoryModel();

public:
    static CurrencyHistoryModel& instance(wxSQLite3Database* db);
    static CurrencyHistoryModel& instance();

    static CurrencyHistoryCol::CURRDATE CURRDATE(OP op, const mmDate& date);

public:
    auto get_key_data_n(int64 currency_id, const mmDate& date) -> const Data*;
    auto get_id_date_rate(int64 currency_id, const mmDate& date = mmDate::today()) -> double;
    auto get_id_last_rate(int64 currency_id) -> double;

    auto save_record(
        int64 currency_id, const mmDate& date, double price, UpdateType update_type
    ) -> int64;

    void purge_all();
};
