/*******************************************************
Copyright (C) 2014 Stefano Giorgio

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

//----------------------------------------------------------------------------
#include "db_init_model.h"
#include "cpu_timer.h"

#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "mmOption.h"

#include "model/Model_Currency.h"

#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Budget.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Stock.h"

//----------------------------------------------------------------------------

DB_Init_Model::DB_Init_Model()
{
//    ShowMessage("DB_Init_Model - Initialise Instance Only.");
}

DB_Init_Model::~DB_Init_Model()
{
//    ShowMessage("DB_Init_Model - Shutdown.");
}

void DB_Init_Model::Init_Model_Tables(wxSQLite3Database* test_db)
{
    CpuTimer start("DB_Init_Model");

    // For the purpose of testing, we will create the
    // settings table in the main database.
    Model_Setting::instance(test_db);
    mmIniOptions::instance().loadOptions();

    Model_Infotable::instance(test_db);
    mmOptions::instance().LoadInfotableOptions();

    Model_Currency::instance(test_db);
    Model_Account::instance(test_db);

    Model_Subcategory::instance(test_db);
    Model_Category::instance(test_db);
    Model_Payee::instance(test_db);

    Model_Checking::instance(test_db);
    Model_Splittransaction::instance(test_db);

    Model_Billsdeposits::instance(test_db);
    Model_Budgetsplittransaction::instance(test_db);

    Model_Budgetyear::instance(test_db);
    Model_Budget::instance(test_db);

    Model_Asset::instance(test_db);
    Model_Stock::instance(test_db);
}

void DB_Init_Model::Init_Model_Assets(wxSQLite3Database* test_db)
{
    Model_Setting::instance(test_db);
    test_db->Begin();
    {
        Model_Setting::instance().Set("ASSETS_COL0_WIDTH", 111);
        Model_Setting::instance().Set("ASSETS_COL1_WIDTH", 94);
        Model_Setting::instance().Set("ASSETS_COL2_WIDTH", 83);
        Model_Setting::instance().Set("ASSETS_COL3_WIDTH", 109);
        Model_Setting::instance().Set("ASSETS_COL4_WIDTH", 118);
    }
    test_db->Commit();
    mmIniOptions::instance().loadOptions();

    Model_Infotable::instance(test_db);
    mmOptions::instance().LoadInfotableOptions();

    Model_Asset::instance(test_db);
}

void DB_Init_Model::Init_Model_Stocks(wxSQLite3Database* test_db)
{
    // Initialise the required tables
    Model_Infotable::instance(test_db);
    Model_Currency::instance(test_db);
    Model_Account::instance(test_db);
    Model_Stock::instance(test_db);
}

void DB_Init_Model::Init_BaseCurrency(const wxString& base_currency_symbol, const wxString& user_name)
{
    Model_Currency currency = Model_Currency::instance();
    Model_Currency::Data currency_record = currency.GetCurrencyRecord(base_currency_symbol);
    if (base_currency_symbol == "AUD")
    {
        // ensure that group separator is a comma in database.
        currency_record.GROUP_SEPARATOR = ",";
        currency.save(&currency_record);
    }
    currency.SetBaseCurrency(&currency_record);
    // Set database User Name
    Model_Infotable::instance().Set("USERNAME", user_name);
    mmOptions::instance().userNameString_ = user_name;
}

int DB_Init_Model::Add_Account(const wxString& name, Model_Account::TYPE account_type, wxString currency_symbol)
{
    int currencyID = Model_Infotable::instance().GetBaseCurrencyId();
    if (currencyID == -1)
    {
        ShowMessage("Base Currency has not been set.\n\n Use InitDatabase(...)");
    }

    Model_Account::Data* account = Model_Account::instance().create();
    account->FAVORITEACCT = "TRUE";
    account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
    account->ACCOUNTTYPE = Model_Account::all_type()[account_type];
    account->ACCOUNTNAME = name;
    account->INITIALBAL = 0;
    account->CURRENCYID = currencyID;

    return Model_Account::instance().save(account);
}

int DB_Init_Model::Add_Payee(const wxString& name, const wxString& category, const wxString& subcategory)
{
    int cat_id = -1;
    if (!category.IsEmpty())
    {
        // Get category id using the category name
        cat_id = Model_Category::instance().get(category)->id();
    }

    int subcat_id = -1;
    if (!subcategory.IsEmpty())
    {
        // Get subcategory id using the subcategory name
        subcat_id = Model_Subcategory::instance().get(subcategory, cat_id)->id();
    }

    Model_Payee::Data* payee_entry = Model_Payee::instance().create();
    payee_entry->PAYEENAME = name;
    payee_entry->CATEGID = cat_id;
    payee_entry->SUBCATEGID = subcat_id;

    return Model_Payee::instance().save(payee_entry);
}

void DB_Init_Model::Set_AccountName(const wxString& account_name)
{
    Model_Account::Data* account = Model_Account::instance().get(account_name);
    m_account_name = account->ACCOUNTNAME;
    m_account_id = account->id();
}

int DB_Init_Model::Category_id(const wxString& category)
{
    int cat_id = -1;
    if (!category.IsEmpty())
    {
        // Get category id using the category name
        cat_id = Model_Category::instance().get(category)->id();
    }

    return cat_id;
}

int DB_Init_Model::Subcategory_id(const wxString& subcategory, int category_id)
{
    int subcat_id = -1;
    if (!subcategory.IsEmpty())
    {
        // Get subcategory id using the subcategory name
        subcat_id = Model_Subcategory::instance().get(subcategory, category_id)->id();
    }

    return subcat_id;
}

// This commands sets m_processing_transaction
int DB_Init_Model::Add_Trans_Deposit(const wxDateTime& date, const wxString& payee, double value
    , const wxString& category, const wxString& subcategory)
{
    Model_Checking::Data* tran_entry = Model_Checking::instance().create();
    tran_entry->ACCOUNTID = m_account_id;
    tran_entry->TOACCOUNTID = -1;

    Model_Payee::Data* entry = Model_Payee::instance().get(payee);
    tran_entry->PAYEEID = entry->id();

    // Set to Deposit
    tran_entry->TRANSCODE = Model_Checking::instance().all_type()[Model_Checking::DEPOSIT];
    tran_entry->TRANSAMOUNT = 1000;
    tran_entry->STATUS = Model_Checking::all_status()[Model_Checking::RECONCILED];

    tran_entry->CATEGID = Category_id(category);
    tran_entry->SUBCATEGID = Subcategory_id(subcategory, tran_entry->CATEGID);
    tran_entry->TRANSDATE = date.FormatISODate();
    tran_entry->FOLLOWUPID = 0;
    tran_entry->TOTRANSAMOUNT = value;
    return Model_Checking::instance().save(tran_entry);
}

int DB_Init_Model::Add_Trans_Withdrawal(const wxDateTime& date, const wxString& payee
    , double value, const wxString& category, const wxString& subcategory)
{
    Model_Checking::Data* tran_entry = Model_Checking::instance().create();
    tran_entry->ACCOUNTID = m_account_id;
    tran_entry->TOACCOUNTID = -1;

    Model_Payee::Data* entry = Model_Payee::instance().get(payee);
    tran_entry->PAYEEID = entry->id();

    // Set to withdrawal
    tran_entry->TRANSCODE = Model_Checking::instance().all_type()[Model_Checking::WITHDRAWAL];
    tran_entry->TRANSAMOUNT = 1000;
    tran_entry->STATUS = Model_Checking::all_status()[Model_Checking::RECONCILED];

    tran_entry->CATEGID = Category_id(category);
    tran_entry->SUBCATEGID = Subcategory_id(subcategory, tran_entry->CATEGID);
    tran_entry->TRANSDATE = date.FormatISODate();
    tran_entry->FOLLOWUPID = 0;
    tran_entry->TOTRANSAMOUNT = value;
    return Model_Checking::instance().save(tran_entry);
}

// This commands sets m_processing_transfer_transaction
int DB_Init_Model::Add_Trans_Transfer(const wxDateTime& date, const wxString& to_account, double value
    , const wxString& category, const wxString& subcategory, bool advanced, double adv_value)
{
    Model_Checking::Data* tran_entry = Model_Checking::instance().create();
    tran_entry->ACCOUNTID = m_account_id;
    tran_entry->TOACCOUNTID = Model_Account::instance().get(to_account)->id();
    tran_entry->PAYEEID = -1;

    // Set to Transfer
    tran_entry->TRANSCODE = Model_Checking::instance().all_type()[Model_Checking::TRANSFER];
    tran_entry->TRANSAMOUNT = 1000;
    tran_entry->STATUS = Model_Checking::all_status()[Model_Checking::RECONCILED];

    tran_entry->CATEGID = Category_id(category);
    tran_entry->SUBCATEGID = Subcategory_id(subcategory, tran_entry->CATEGID);
    tran_entry->TRANSDATE = date.FormatISODate();
    tran_entry->FOLLOWUPID = 0;
    tran_entry->TOTRANSAMOUNT = value;
    if (advanced) tran_entry->TOTRANSAMOUNT = adv_value;
    return Model_Checking::instance().save(tran_entry);
}

// If category not supplied, assume that it is a split.
void DB_Init_Model::Add_Trans_Split(int trans_id, double value, const wxString& category, const wxString& subcategory)
{
    Model_Checking::Data* trans_entry = Model_Checking::instance().get(trans_id);
    int cat_id = -1;
    int subcat_id = -1;

    if (trans_entry)
    {
        if ((trans_entry->CATEGID > 0) || (trans_entry->SUBCATEGID > 0))
        {
            ShowMessage("Transaction Category reset for Split transaction");
            trans_entry->CATEGID = -1;
            trans_entry->SUBCATEGID = -1;
            Model_Checking::instance().save(trans_entry);
        }

        if (!category.IsEmpty())
        {
            // Get category id for the category name
            cat_id = Model_Category::instance().get(category)->id();
            if (cat_id < 0) ShowMessage("Split transaction category not set.");
        }

        if (!subcategory.IsEmpty())
        {
            // Get subcategory id for the subcategory name
            subcat_id = Model_Subcategory::instance().get(subcategory, cat_id)->id();
        }

        Model_Splittransaction::Data* trans_split_entry = Model_Splittransaction::instance().create();
        trans_split_entry->TRANSID = trans_id;
        trans_split_entry->SPLITTRANSAMOUNT = value;
        trans_split_entry->CATEGID = cat_id;
        trans_split_entry->SUBCATEGID = subcat_id;
        Model_Splittransaction::instance().save(trans_split_entry);
    }
    else ShowMessage("Transaction not found for the Split Transaction");
}

void DB_Init_Model::Add_Bill_Split(int trans_id, double value, const wxString& category, const wxString& subcategory)
{
    Model_Checking::Data* trans_entry = Model_Checking::instance().get(trans_id);
    int cat_id = -1;
    int subcat_id = -1;

    if (trans_entry)
    {
        if ((trans_entry->CATEGID > 0) || (trans_entry->SUBCATEGID > 0))
        {
            ShowMessage("Transaction Category reset for Split transaction");
            trans_entry->CATEGID = -1;
            trans_entry->SUBCATEGID = -1;
            Model_Checking::instance().save(trans_entry);
        }

        if (!category.IsEmpty())
        {
            // Get category id for the category name
            cat_id = Model_Category::instance().get(category)->id();
            if (cat_id < 0) ShowMessage("Split transaction category not set.");
        }

        if (!subcategory.IsEmpty())
        {
            // Get subcategory id for the subcategory name
            subcat_id = Model_Subcategory::instance().get(subcategory, cat_id)->id();
        }

        Model_Budgetsplittransaction::Data* bill_split_entry = Model_Budgetsplittransaction::instance().create();
        bill_split_entry->TRANSID = trans_id;
        bill_split_entry->SPLITTRANSAMOUNT = value;
        bill_split_entry->CATEGID = cat_id;
        bill_split_entry->SUBCATEGID = subcat_id;
        Model_Budgetsplittransaction::instance().save(bill_split_entry);
    }
    else ShowMessage("Transaction not found for the Split Transaction");
}




//void DB_Init_Model::SetAdvancedRepeat(BILL_ADVANCED repeat_type, int period)
//{
//
//}

void DB_Init_Model::EndBILL()
{

}

int DB_Init_Model::Add_Asset(const wxString& name, const wxDate& date, double value, Model_Asset::TYPE asset_type,
    Model_Asset::RATE value_change, double value_change_rate, const wxString& notes)
{
    Model_Asset::Data* entry = Model_Asset::instance().create();
    entry->ASSETNAME = name;
    entry->STARTDATE = date.FormatISODate();
    entry->VALUE = value;
    entry->VALUECHANGE = Model_Asset::all_rate()[value_change];
    entry->VALUECHANGERATE = value_change_rate;
    entry->ASSETTYPE = Model_Asset::all_type()[asset_type];
    entry->NOTES = notes;
    return Model_Asset::instance().save(entry);
}

void DB_Init_Model::ShowMessage(wxString msg)
{
    wxMessageBox(msg, "MMEX_Table Data Initialisation", wxOK | wxICON_WARNING, wxTheApp->GetTopWindow());
}
