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

#include "TransactionShareModel.h"
#include "TransactionModel.h"

TransactionShareModel::TransactionShareModel()
: Model<DB_Table_SHAREINFO_V1>()
{
}

TransactionShareModel::~TransactionShareModel()
{
}

/**
* Initialize the global TransactionShareModel table.
* Reset the TransactionShareModel table or create the table if it does not exist.
*/
TransactionShareModel& TransactionShareModel::instance(wxSQLite3Database* db)
{
    TransactionShareModel& ins = Singleton<TransactionShareModel>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of TransactionShareModel table */
TransactionShareModel& TransactionShareModel::instance()
{
    return Singleton<TransactionShareModel>::instance();
}

TransactionShareModel::Data_Set TransactionShareModel::ShareList(const int64 checking_id)
{
    // SQL equivalent statement:
    // select * from TransactionShareModel where CHECKINGACCOUNTID = checking_account_id;
    TransactionShareModel::Data_Set trans_list = TransactionShareModel::instance()
        .find(TransactionShareModel::CHECKINGACCOUNTID(checking_id));

    return trans_list;
}

TransactionShareModel::Data* TransactionShareModel::ShareEntry(const int64 checking_id)
{
    Data_Set list = TransactionShareModel::ShareList(checking_id);
    if (!list.empty())
    {
        return TransactionShareModel::instance().get(list.at(0).SHAREINFOID);
    }
    return nullptr;
}

void TransactionShareModel::ShareEntry(int64 checking_id
    , double share_number
    , double share_price
    , double share_commission
    , const std::vector<Split>& commission_splits
    , const wxString& share_lot)
{
    bool updateTimestamp = false;
    Data old_entry;
    Data* share_entry = nullptr;
    Data_Set share_list = ShareList(checking_id);

    if (share_list.empty())
    {
        share_entry = TransactionShareModel::instance().create();
        share_entry->CHECKINGACCOUNTID = checking_id;
        updateTimestamp = true;
    }
    else
    {
        old_entry = share_list[0];
        share_entry = &share_list[0];
    }

    share_entry->SHARENUMBER = share_number;
    share_entry->SHAREPRICE = share_price;
    share_entry->SHARECOMMISSION = share_commission;
    share_entry->SHARELOT = share_lot;
    auto id = TransactionShareModel::instance().save(share_entry);

    TransactionSplitModel::instance().update(commission_splits, id);

    if(updateTimestamp || !share_entry->equals(&old_entry))
        TransactionModel::instance().updateTimestamp(checking_id);
}

void TransactionShareModel::RemoveShareEntry(const int64 checking_id)
{
    Data_Set list = ShareList(checking_id);
    if (!list.empty())
    {
        TransactionShareModel::instance().remove(list.at(0).SHAREINFOID);
    }
}
