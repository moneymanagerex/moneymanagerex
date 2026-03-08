/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2016 Stefano Giorgio

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

#include "table/TrxShareTable.h"
#include "data/TrxShareData.h"

#include "_ModelBase.h"
#include "TrxSplitModel.h"

class TrxShareModel : public TableFactory<TrxShareTable, TrxShareData>
{
public:
    TrxShareModel();
    ~TrxShareModel();

public:
    static TrxShareModel& instance(wxSQLite3Database* db);
    static TrxShareModel& instance();

public:
    void purge_trxId(const int64 trx_id);

    auto get_trxId_id(const int64 trx_id) -> int64;
    auto unsafe_get_trxId_data_n(const int64 trx_id) -> Data*;
    auto get_trxId_data_n(const int64 trx_id) -> const Data*;

    void update_trxID(
        int64 trx_id,
        double share_number,
        double share_price,
        double share_commission,
        const wxString& share_lot,
        const std::vector<Split>& commission_splits
    );
};
