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

void TrxShareModel::purge_trxId(const int64 trx_id)
{
    int64 ts_id = get_trxId_id(trx_id);
    if (ts_id > 0) {
        purge_id(ts_id);
    }
}

int64 TrxShareModel::get_trxId_id(const int64 trx_id)
{
    DataA ts_a = find(TrxShareCol::CHECKINGACCOUNTID(trx_id));
    return !ts_a.empty() ? ts_a.at(0).m_id : -1;
}

TrxShareData* TrxShareModel::unsafe_get_trxId_data_n(const int64 trx_id)
{
    return unsafe_get_id_data_n(get_trxId_id(trx_id));
}

const TrxShareData* TrxShareModel::get_trxId_data_n(const int64 trx_id)
{
    return get_id_data_n(get_trxId_id(trx_id));
}

// Create a Share record if it does not exist.
// Save the share record linked to the transaction.
void TrxShareModel::update_trxID(
    int64 trx_id,
    double share_number,
    double share_price,
    double share_commission,
    const wxString& share_lot,
    const std::vector<Split>& commission_splits
) {
    const Data* old_ts_n = get_trxId_data_n(trx_id);
    Data new_ts_d;
    if (old_ts_n) {
        new_ts_d = *old_ts_n;
    }
    else {
        new_ts_d = Data();
        new_ts_d.m_trx_id = trx_id;
    }

    new_ts_d.m_number     = share_number;
    new_ts_d.m_price      = share_price;
    new_ts_d.m_commission = share_commission;
    new_ts_d.m_lot        = share_lot;
    save_data_n(new_ts_d);

    TrxSplitModel::instance().update(commission_splits, new_ts_d.m_id);
    if (!old_ts_n || !new_ts_d.equals(old_ts_n))
        TrxModel::instance().save_timestamp(trx_id);
}
