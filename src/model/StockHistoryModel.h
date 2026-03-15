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

#pragma once

#include "base/defs.h"

#include "table/StockHistoryTable.h"
#include "data/StockHistoryData.h"

#include "_ModelBase.h"

class StockHistoryModel : public TableFactory<StockHistoryTable, StockHistoryData>
{
public:
    StockHistoryModel();
    ~StockHistoryModel();

public:
    static StockHistoryModel& instance(wxSQLite3Database* db);
    static StockHistoryModel& instance();

    static StockHistoryCol::DATE DATE(OP op, const mmDate& date);

public:
    auto get_key_data_n(const wxString& symbol, const mmDate& date) -> const Data*;

    auto save_record(
        const wxString& symbol, const mmDate& date, double price, UpdateType update_type
    ) -> int64;
};

