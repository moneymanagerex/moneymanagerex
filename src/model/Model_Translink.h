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
#include "Model.h"
#include "Table_Translink.h"
#include "Model_Checking.h"
#include "Model_Stock.h"
#include "Model_Asset.h"
#include "Model_Attachment.h"

class Model_Translink : public Model<DB_Table_TRANSLINK>
{
public:
    enum CHECKING_TYPE { AS_INCOME_EXPENSE = 32701, AS_TRANSFER }; /* Transfers ignore accounting */

public:
    Model_Translink();
    ~Model_Translink();

public:
    /**
    Initialize the global Model_Translink table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Translink table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Translink& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Translink table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Translink& instance();

public:
    static CHECKING_TYPE type_checking(const int tt);

public:
    /* Create the translink record as Asset */
    static Model_Translink::Data* SetAssetTranslink(const int asset_id
        , const int checking_id
        , const CHECKING_TYPE checking_type = AS_INCOME_EXPENSE);

    /* Create a translink record as Stock */
    static Model_Translink::Data* SetStockTranslink(const int stock_id
        , const int checking_id
        , const CHECKING_TYPE checking_type = AS_INCOME_EXPENSE);

    /*
    Return a list of translink records for the associated foreign table type.
    Equivalent SQL statements:
    select * from TRANSLINK where LINKTYPE = "Asset" AND LINKRECORDID = link_id;
    select * from TRANSLINK where LINKTYPE = "Stock" AND LINKRECORDID = link_id;
    */
    static Model_Translink::Data_Set TranslinkList(Model_Attachment::REFTYPE link_table
        , const int link_id);

    static bool HasShares(const int stock_id);

    /*
    Return the link record for the checking account 
    Equivalent SQL statements:
    select * from TRANSLINK where CHECKINGACCOUNTID = checking_id;
    */
    static Model_Translink::Data TranslinkRecord(const int checking_id);

    /* Remove all records associated with the Translink list */
    static void RemoveTransLinkRecords(Model_Attachment::REFTYPE table_type, const int entry_id);
 
    /* Remove the checking account entry and its associated transfer transaction. */
    static void RemoveTranslinkEntry(const int checking_account_id);

    /*
    stock_entry.PURCHASEPRICE ... obsolete. This is now on a per entry basis.
    stock_entry.NUMSHARES = total amount of shares purchased.
    stock_entry.VALUE     = value of shares based on:
    ... share_entry.SHARENUMBER * share_entry.SHAREPRICE
    */
    static void UpdateStockValue(Model_Stock::Data* stock_entry);
    static void UpdateAssetValue(Model_Asset::Data* asset_entry);

    /* Return true with the account id of the first share entry in the stock translink list */
    static bool ShareAccountId(int& stock_entry_id);

private:

    static Model_Translink::Data* SetTranslink(const int checking_id, const CHECKING_TYPE checking_type
        , const wxString& link_type, const int link_record_id);
};
