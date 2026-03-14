/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2016 Stefano Giorgio
 Copyright (C) 2025 Klaus Wich

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

#include "table/TrxLinkTable.h"
#include "data/TrxLinkData.h"

#include "_ModelBase.h"
#include "TrxModel.h"
#include "StockModel.h"
#include "AssetModel.h"
#include "AttachmentModel.h"

class TrxLinkModel : public TableFactory<TrxLinkTable, TrxLinkData>
{
public:
    // TODO: move to *Data
    enum CHECKING_TYPE {
        AS_INCOME_EXPENSE = 32701,
        AS_TRANSFER /* Transfers ignore accounting */
    };

public:
    TrxLinkModel();
    ~TrxLinkModel();

public:
    static TrxLinkModel& instance(wxSQLite3Database* db);
    static TrxLinkModel& instance();

    // TODO: move to *Data
    static CHECKING_TYPE type_checking(const int64 tt);

public:
    void purge_ref(RefTypeN ref_type, int64 ref_id);

    auto get_trx_data_n(int64 trx_id) -> const Data*;
    auto find_ref_data_a(RefTypeN ref_type, int64 ref_id) -> DataA;
    auto find_symbol_data_a(const wxString stock_symbol) -> DataA;
    auto find_stock_id_c(const int64 stock_id) -> size_t;

    void update_asset_value(AssetData* asset_n);

    void save_record(
        int64 trx_id,
        RefTypeN ref_type,
        int64 ref_id,
        const CHECKING_TYPE checking_type
    );
    void save_asset_record(
        int64 trx_id,
        int64 asset_id,
        const CHECKING_TYPE checking_type = AS_INCOME_EXPENSE
    );
    void save_stock_record(
        int64 trx_id,
        int64 stock_id,
        const CHECKING_TYPE checking_type = AS_INCOME_EXPENSE
    );
};
