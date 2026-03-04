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

#include "TrxModel.h"
#include "TrxShareModel.h"

TrxShareModel::TrxShareModel() :
    TableFactory<TrxShareTable, TrxShareData>()
{
}

TrxShareModel::~TrxShareModel()
{
}

// Initialize the global TrxShareModel table.
// Reset the TrxShareModel table or create the table if it does not exist.
TrxShareModel& TrxShareModel::instance(wxSQLite3Database* db)
{
    TrxShareModel& ins = Singleton<TrxShareModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of TrxShareModel table
TrxShareModel& TrxShareModel::instance()
{
    return Singleton<TrxShareModel>::instance();
}

int64 TrxShareModel::find_trx_share_id(const int64 trx_id)
{
    DataA ts_a = find(TrxShareCol::CHECKINGACCOUNTID(trx_id));
    return !ts_a.empty() ? ts_a.at(0).m_id : -1;
}

TrxShareData* TrxShareModel::unsafe_get_trx_share_n(const int64 trx_id)
{
    int64 ts_id = find_trx_share_id(trx_id);
    return ts_id > 0 ? unsafe_get_id_data_n(ts_id) : nullptr;
}

void TrxShareModel::ShareEntry(
    int64 trx_id,
    double share_number,
    double share_price,
    double share_commission,
    const std::vector<Split>& commission_splits,
    const wxString& share_lot
) {
    int64 ts_id = TrxShareModel().instance().find_trx_share_id(trx_id);
    Data new_ts_d;
    Data old_ts_d;
    bool save_timestamp = false;

    if (ts_id > 0) {
        new_ts_d = *(TrxShareModel::instance().get_id_data_n(ts_id));
        old_ts_d = new_ts_d;
    }
    else {
        new_ts_d = Data();
        new_ts_d.m_trx_id = trx_id;
        save_timestamp = true;
    }

    new_ts_d.m_number     = share_number;
    new_ts_d.m_price      = share_price;
    new_ts_d.m_commission = share_commission;
    new_ts_d.m_lot        = share_lot;
    TrxShareModel::instance().save_data_n(new_ts_d);
    TrxSplitModel::instance().update(commission_splits, new_ts_d.id());
    if (save_timestamp || !new_ts_d.equals(&old_ts_d))
        TrxModel::instance().save_timestamp(trx_id);
}

void TrxShareModel::remove_trx_share(const int64 trx_id)
{
    int64 ts_id = find_trx_share_id(trx_id);
    if (ts_id > 0) {
        purge_id(ts_id);
    }
}
