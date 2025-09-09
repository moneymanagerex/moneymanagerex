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
#include "Model.h"
#include "db/DB_Table_Translink_V1.h"
#include "Model_Checking.h"
#include "Model_Stock.h"
#include "Model_Asset.h"
#include "Model_Attachment.h"

class Model_Translink : public Model<DB_Table_TRANSLINK_V1>
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
    static CHECKING_TYPE type_checking(const int64 tt);

public:
    /* Create the translink record as Asset */
    static Model_Translink::Data* SetAssetTranslink(const int64 asset_id
        , const int64 checking_id
        , const CHECKING_TYPE checking_type = AS_INCOME_EXPENSE);

    /* Create a translink record as Stock */
    static Model_Translink::Data* SetStockTranslink(const int64 stock_id
        , const int64 checking_id
        , const CHECKING_TYPE checking_type = AS_INCOME_EXPENSE);

    /*
    Return a list of translink records for the associated foreign table type.
    Equivalent SQL statements:
    select * from TRANSLINK_V1 where LINKTYPE = "Asset" AND LINKRECORDID = link_id;
    select * from TRANSLINK_V1 where LINKTYPE = "Stock" AND LINKRECORDID = link_id;
    */
    template <typename T>
    static Model_Translink::Data_Set TranslinkList(const int64 link_id);

    /*
    Return the link record for the symbol
    Equivalent SQL statements:
    SELECT * FROM TRANSLINK_V1 WHERE LINKRECORDID IN (SELECT STOCKID FROM STOCK_V1 WHERE SYMBOL = ?)
    */
    static Model_Translink::Data_Set TranslinkListBySymbol(const wxString symbol);

    static bool HasShares(const int64 stock_id);

    /*
    Return the link record for the checking account
    Equivalent SQL statements:
    select * from TRANSLINK_V1 where CHECKINGACCOUNTID = checking_id;
    */
    static Model_Translink::Data TranslinkRecord(const int64 checking_id);

    /* Remove all records associated with the Translink list */
    template <typename T>
    static void RemoveTransLinkRecords(const int64 entry_id);

    /* Remove the checking account entry and its associated transfer transaction. */
    static void RemoveTranslinkEntry(const int64 checking_account_id);

    static void UpdateAssetValue(Model_Asset::Data* asset_entry);

    /* Return true with the account id of the first share entry in the stock translink list */
    static bool ShareAccountId(int64& stock_entry_id);

private:

    static Model_Translink::Data* SetTranslink(const int64 checking_id, const CHECKING_TYPE checking_type
        , const wxString& link_type, const int64 link_record_id);
};
