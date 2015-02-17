/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2015 Stefano Giorgio

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

#ifndef MODEL_TRANSFERTRANS_H
#define MODEL_TRANSFERTRANS_H

#include "Model.h"
#include "db/DB_Table_Transfertrans_V1.h"
#include "Model_Currency.h"
#include "Model_Checking.h"

class Model_TransferTrans : public Model<DB_Table_TRANSFERTRANS_V1>
{
public:
    enum TABLE_TYPE { ASSETS = 0, STOCKS };
    enum CHECKING_TYPE { AS_INCOME_EXPENSE = 32701, AS_TRANSFER }; /* Transfers ignore accounting */

public:
    static const std::vector<std::pair<TABLE_TYPE, wxString> > TABLE_TYPE_CHOICES;

public:
    Model_TransferTrans();
    ~Model_TransferTrans();

public:
    /**
    Initialize the global Model_TransferTrans table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_TransferTrans table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_TransferTrans& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_TransferTrans table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_TransferTrans& instance();

public:
    static wxArrayString all_table_type();

    static TABLE_TYPE type(const Data* r);
    static TABLE_TYPE type(const Data& r);

public:
    /** Returns the currency Data record pointer*/
    static Model_Currency::Data* Currency(const Data* /* r */);

    /* Create a transaction record for Assets */
    static Model_TransferTrans::Data* SetAssetTransferTransaction(const int& asset_id
        , const int& checking_id
        , const CHECKING_TYPE& transfer = AS_INCOME_EXPENSE
        , const wxString& currency_symbol = Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL);

    /* Create a transaction record for Stocks */
    static Model_TransferTrans::Data* SetShareTransferTransaction(const int& share_id, const int& checking_id
        , const double& share_price = 0, const double& share_number = 0, const double& share_commission = 0
        , const CHECKING_TYPE& transfer = AS_INCOME_EXPENSE
        , const wxString& currency_symbol = Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL);

    /* Return the transfer list for for the correct table type.
    Model_TransferTrans::ASSETS or Model_TransferTrans::STOCKS */
    static Model_TransferTrans::Data_Set TransferList(Model_TransferTrans::TABLE_TYPE table_type
        , const int& entry_id);

    /* Return the list of records maintained by the transfer list.
    Model_TransferTrans::ASSETS or Model_TransferTrans::STOCKS */
    static Model_Checking::Data_Set TransferRecordList(Model_TransferTrans::TABLE_TYPE table_type
        , const int& entry_id);

    /* Return the transfer record for the checking account */
    static Model_TransferTrans::Data TransferEntry(const int& checking_account_id);

    /*
    Remove all transfer transactions associated with the asset_id,
    including all associated checking account entries */
    static void RemoveTransferTransactions(Model_TransferTrans::TABLE_TYPE table_type
        , const int& entry_id);
 
    /* Remove the checking account entry and its associated transfer transaction. */
    static void RemoveTransferEntry(const int& checking_account_id);

private:
    static Model_TransferTrans::Data* SetTransferTransaction(const wxString& trans_type
        , const int& table_record_id, const int& checking_id, const CHECKING_TYPE& transfer
        , const wxString& currency_symbol
        , const double& share_price = 0, const double& share_number = 0, const double& share_commission = 0);
};

#endif // 
