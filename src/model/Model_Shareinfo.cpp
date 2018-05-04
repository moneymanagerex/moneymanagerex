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

#include "Model_Shareinfo.h"

Model_Shareinfo::Model_Shareinfo()
: Model<DB_Table_SHAREINFO>()
{
}

Model_Shareinfo::~Model_Shareinfo()
{
}

/**
* Initialize the global Model_Shareinfo table.
* Reset the Model_Shareinfo table or create the table if it does not exist.
*/
Model_Shareinfo& Model_Shareinfo::instance(wxSQLite3Database* db)
{
    Model_Shareinfo& ins = Singleton<Model_Shareinfo>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Shareinfo table */
Model_Shareinfo& Model_Shareinfo::instance()
{
    return Singleton<Model_Shareinfo>::instance();
}

Model_Shareinfo::Data_Set Model_Shareinfo::ShareList(const int checking_id)
{
    // SQL equivalent statement:
    // select * from Model_Shareinfo where CHECKINGACCOUNTID = checking_account_id;
    Model_Shareinfo::Data_Set trans_list = Model_Shareinfo::instance()
        .find(Model_Shareinfo::CHECKINGACCOUNTID(checking_id));

    return trans_list;
}

Model_Shareinfo::Data* Model_Shareinfo::ShareEntry(const int checking_id)
{
    Data_Set list = Model_Shareinfo::ShareList(checking_id);
    if (!list.empty())
    {
        return Model_Shareinfo::instance().get(list.at(0).SHAREINFOID);
    }
    return NULL;
}

void Model_Shareinfo::ShareEntry(int checking_id
    , double share_number
    , double share_price
    , double share_commission
    , const wxString& share_lot)
{
    Data* share_entry = NULL;
    Data_Set share_list = ShareList(checking_id);

    if (share_list.empty())
    {
        share_entry = Model_Shareinfo::instance().create();
        share_entry->CHECKINGACCOUNTID = checking_id;
    }
    else
    {
        share_entry = &share_list[0];
    }

    share_entry->SHARENUMBER = share_number;
    share_entry->SHAREPRICE = share_price;
    share_entry->SHARECOMMISSION = share_commission;
    share_entry->SHARELOT = share_lot;
    Model_Shareinfo::instance().save(share_entry);
}

void Model_Shareinfo::RemoveShareEntry(const int checking_id)
{
    Data_Set list = ShareList(checking_id);
    if (!list.empty())
    {
        Model_Shareinfo::instance().remove(list.at(0).SHAREINFOID);
    }
}
