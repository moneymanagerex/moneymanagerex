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
: m_bill_entry(0)
, m_bill_initialised(false)
, m_bill_transaction_set(false)
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
    Model_Infotable::instance().Set("DATEFORMAT", "%d-%m-%Y");
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
    Model_Setting::instance(test_db);
    test_db->Begin();
    {
        Model_Setting::instance().Set("STOCKS_COL3_WIDTH", 72);
        Model_Setting::instance().Set("STOCKS_COL0_WIDTH", 126);
        Model_Setting::instance().Set("STOCKS_COL4_WIDTH", 73);
        Model_Setting::instance().Set("STOCKS_COL5_WIDTH", 66);
        Model_Setting::instance().Set("STOCKS_COL1_WIDTH", 128);
        Model_Setting::instance().Set("STOCKS_COL6_WIDTH", 138);
    }
    test_db->Commit();
    mmIniOptions::instance().loadOptions();
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

int DB_Init_Model::Add_Bank_Account(const wxString& name, double initial_value, const wxString& notes, bool favorite, const wxString& currency_symbol)
{
    return Add_Account(name, Model_Account::TYPE::CHECKING, initial_value, notes, favorite, currency_symbol);
}

int DB_Init_Model::Add_Investment_Account(const wxString& name, double initial_value, const wxString& notes, bool favorite, const wxString& currency_symbol)
{
    return Add_Account(name, Model_Account::TYPE::INVESTMENT, initial_value, notes, favorite, currency_symbol);
}

int DB_Init_Model::Add_Term_Account(const wxString& name, double initial_value, const wxString& notes, bool favorite, const wxString& currency_symbol)
{
    return Add_Account(name, Model_Account::TYPE::TERM, initial_value, notes, favorite, currency_symbol);
}

int DB_Init_Model::Add_Account(const wxString& name, Model_Account::TYPE account_type, double initial_value, const wxString& notes, bool favorite, const wxString& currency_symbol)
{
    int currencyID = -1;
    if (!currency_symbol.IsEmpty())
    {
        currencyID = Model_Currency::instance().GetCurrencyRecord(currency_symbol).id();
    }
    else currencyID = Model_Infotable::instance().GetBaseCurrencyId();

    if (currencyID == -1)
    {
        ShowMessage("Base Currency has not been set.\n\n Use InitDatabase(...)");
    }

    Model_Account::Data* account = Model_Account::instance().create();
    if (favorite)
    {
        account->FAVORITEACCT = "TRUE";
    }
    else account->FAVORITEACCT = "FALSE";

    account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
    account->ACCOUNTTYPE = Model_Account::all_type()[account_type];
    account->ACCOUNTNAME = name;
    account->INITIALBAL = initial_value;
    account->NOTES = notes;
    account->CURRENCYID = currencyID;

    return Model_Account::instance().save(account);
}

int DB_Init_Model::Add_Payee(const wxString& name, const wxString& category, const wxString& subcategory)
{
    Model_Payee::Data* payee_entry = Model_Payee::instance().create();
    payee_entry->PAYEENAME = name;
    payee_entry->CATEGID = Category_id(category);
    payee_entry->SUBCATEGID = Subcategory_id(payee_entry->CATEGID, subcategory);

    return Model_Payee::instance().save(payee_entry);
}

int DB_Init_Model::Get_Payee_id(const wxString& name)
{
    int payee_id = -1;
    Model_Payee::Data* entry = Model_Payee::instance().get(name);
    if (entry)
    {
        payee_id = entry->id();
    }
    else ShowMessage("The ID not found for Payee: " + name);

    return payee_id;
}

int DB_Init_Model::Add_Category(const wxString& name)
{
    Model_Category::Data* cat_entry = Model_Category::instance().create();
    cat_entry->CATEGNAME = name;

    return Model_Category::instance().save(cat_entry);
}

int DB_Init_Model::Category_id(const wxString& category)
{
    int cat_id = -1;
    if (!category.IsEmpty())
    {
        // Get category id using the category name
        Model_Category::Data* entry = Model_Category::instance().get(category);
        if (entry)
        {
            cat_id = entry->id();
        }
        else ShowMessage("The ID not found for Category: " + category);
    }

    return cat_id;
}

int DB_Init_Model::Add_Subcategory(int category_id, const wxString& name)
{
    Model_Subcategory::Data* subcat_entry = Model_Subcategory::instance().create();
    subcat_entry->SUBCATEGNAME = name;
    subcat_entry->CATEGID = category_id;

    return Model_Subcategory::instance().save(subcat_entry);
}

int DB_Init_Model::Subcategory_id(int category_id, const wxString& subcategory)
{
    int subcat_id = -1;
    if (!subcategory.IsEmpty())
    {
        // Get subcategory id using the subcategory name
        Model_Subcategory::Data* entry = Model_Subcategory::instance().get(subcategory, category_id);
        if (entry)
        {
            subcat_id = entry->id();
        }
        else ShowMessage("The ID not found for Subcategory: " + subcategory);
    }

    return subcat_id;
}

int DB_Init_Model::Get_Account_ID(const wxString& account_name)
{
    int account_id = -1;
    Model_Account::Data* account = Model_Account::instance().get(account_name);
    if (account)
    {
        account_id = account->id();
    }
    else ShowMessage("The ID not found for account: " + account_name);

    return account_id;
}

int DB_Init_Model::Add_Trans(const wxString& account_name, Model_Checking::TYPE trans_type, const wxDateTime& date, const wxString& payee, double value
    , const wxString& category, const wxString& subcategory)
{
    if (m_account_name != account_name)
    {
        m_account_name = account_name;
        m_account_id = Get_Account_ID(account_name);
    }

    Model_Checking::Data* tran_entry = Model_Checking::instance().create();
    tran_entry->ACCOUNTID = m_account_id;
    tran_entry->TOACCOUNTID = -1;

    tran_entry->PAYEEID = Get_Payee_id(payee);

    // Set to Deposit
    tran_entry->TRANSCODE = Model_Checking::instance().all_type()[trans_type];
    tran_entry->TRANSAMOUNT = value;
    tran_entry->STATUS = Model_Checking::all_status()[Model_Checking::RECONCILED].Mid(0,1);

    tran_entry->CATEGID = Category_id(category);
    tran_entry->SUBCATEGID = Subcategory_id(tran_entry->CATEGID, subcategory);
    tran_entry->TRANSDATE = date.FormatISODate();
    tran_entry->FOLLOWUPID = 0;
    tran_entry->TOTRANSAMOUNT = value;
    return Model_Checking::instance().save(tran_entry);
}

int DB_Init_Model::Add_Trans_Deposit(const wxString& account_name, const wxDateTime& date, const wxString& payee, double value
    , const wxString& category, const wxString& subcategory)
{
    return Add_Trans(account_name, Model_Checking::DEPOSIT, date, payee, value, category, subcategory);
}

int DB_Init_Model::Add_Trans_Withdrawal(const wxString& account_name, const wxDateTime& date, const wxString& payee
    , double value, const wxString& category, const wxString& subcategory)
{
    return Add_Trans(account_name, Model_Checking::WITHDRAWAL, date, payee, value, category, subcategory);
}

int DB_Init_Model::Add_Trans_Transfer(const wxString& account_name, const wxDateTime& date, const wxString& to_account, double value
    , const wxString& category, const wxString& subcategory, bool advanced, double adv_value)
{
    if (m_account_name != account_name)
    {
        m_account_name = account_name;
        m_account_id = Get_Account_ID(account_name);
    }

    Model_Checking::Data* tran_entry = Model_Checking::instance().create();
    tran_entry->ACCOUNTID = m_account_id;
    tran_entry->TOACCOUNTID = Get_Account_ID(to_account);
    tran_entry->PAYEEID = -1;

    // Set to Transfer
    tran_entry->TRANSCODE = Model_Checking::instance().all_type()[Model_Checking::TRANSFER];
    tran_entry->TRANSAMOUNT = value;
    tran_entry->STATUS = Model_Checking::all_status()[Model_Checking::RECONCILED].Mid(0,1);

    tran_entry->CATEGID = Category_id(category);
    tran_entry->SUBCATEGID = Subcategory_id(tran_entry->CATEGID, subcategory);
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
    if (trans_entry)
    {
        if ((trans_entry->CATEGID > 0) || (trans_entry->SUBCATEGID > 0))
        {
            ShowMessage("Transaction Category reset for Split transaction");
            trans_entry->CATEGID = -1;
            trans_entry->SUBCATEGID = -1;
            Model_Checking::instance().save(trans_entry);
        }

        Model_Splittransaction::Data* trans_split_entry = Model_Splittransaction::instance().create();
        trans_split_entry->TRANSID = trans_id;
        trans_split_entry->SPLITTRANSAMOUNT = value;
        trans_split_entry->CATEGID = Category_id(category);
        trans_split_entry->SUBCATEGID = Subcategory_id(trans_split_entry->CATEGID, subcategory);
        Model_Splittransaction::instance().save(trans_split_entry);
    }
    else ShowMessage("Transaction not found for the Split Transaction");
}

void DB_Init_Model::Add_Bill_Split(int bill_id, double value, const wxString& category, const wxString& subcategory)
{
    Model_Billsdeposits::Data* bill_entry = Model_Billsdeposits::instance().get(bill_id);
    if (bill_entry)
    {
        if ((bill_entry->CATEGID > 0) || (bill_entry->SUBCATEGID > 0))
        {
            ShowMessage("Bill Category reset for Bill Split");
            bill_entry->CATEGID = -1;
            bill_entry->SUBCATEGID = -1;
            Model_Billsdeposits::instance().save(bill_entry);
        }

        Model_Budgetsplittransaction::Data* bill_split_entry = Model_Budgetsplittransaction::instance().create();
        bill_split_entry->TRANSID = bill_id;
        bill_split_entry->SPLITTRANSAMOUNT = value;
        bill_split_entry->CATEGID = Category_id(category);
        bill_split_entry->SUBCATEGID = Subcategory_id(bill_split_entry->CATEGID, subcategory);
        Model_Budgetsplittransaction::instance().save(bill_split_entry);
    }
    else ShowMessage("Bill entry not found for the Bill Split Transaction");
}

void DB_Init_Model::Bill_Start(const wxString& account, const wxDateTime& start_date, Model_Billsdeposits::REPEAT_TYPE repeats, int num_occur)
{
    if (!m_bill_entry)
    {
        m_bill_entry = Model_Billsdeposits::instance().create();

        m_bill_entry->ACCOUNTID = Get_Account_ID(account);
        if (m_bill_entry->ACCOUNTID < 0)
        {
            ShowMessage("Bill Start for account: " + account + " not found.\n");
        }

        m_bill_entry->NEXTOCCURRENCEDATE = start_date.FormatISODate();

        m_bill_entry->REPEATS = repeats;  // This will have extra data added multiplexed to this field.
        m_bill_entry->NUMOCCURRENCES = num_occur;

        m_bill_initialised = true;
    }
    else ShowMessage("Previous bill not saved. \n\nPlease use command: Bill_End(...)\n");
}

void DB_Init_Model::Bill_Trans_Deposit(const wxDateTime& date, const wxString& payee, double value
    , const wxString& category, const wxString& subcategory)
{
    Bill_Transaction(Model_Checking::DEPOSIT, date, payee, value, category, subcategory);
}

void DB_Init_Model::Bill_Trans_Withdrawal(const wxDateTime& date, const wxString& payee, double value
    , const wxString& category, const wxString& subcategory)
{
    Bill_Transaction(Model_Checking::WITHDRAWAL, date, payee, value, category, subcategory);
}

// Common to Bill_Trans_Deposit(...) and Bill_Trans_Withdrawal(...)
void DB_Init_Model::Bill_Transaction(Model_Checking::TYPE trans_type, const wxDateTime& date, const wxString& payee, double value
    , const wxString& category, const wxString& subcategory)
{
    if (m_bill_initialised && !m_bill_transaction_set)
    {
        m_bill_entry->TOACCOUNTID = -1;
        m_bill_entry->PAYEEID = Get_Payee_id(payee);

        // Set to Deposit
        m_bill_entry->TRANSCODE = Model_Checking::instance().all_type()[trans_type];
        m_bill_entry->TRANSAMOUNT = value;
        m_bill_entry->STATUS = Model_Checking::all_status()[Model_Checking::RECONCILED].Mid(0,1);

        m_bill_entry->CATEGID = Category_id(category);
        m_bill_entry->SUBCATEGID = Subcategory_id(m_bill_entry->CATEGID, subcategory);
        m_bill_entry->TRANSDATE = date.FormatISODate();
        m_bill_entry->FOLLOWUPID = 0;
        m_bill_entry->TOTRANSAMOUNT = value;

        m_bill_transaction_set = true;
    }
    else ShowMessage("Bill not initialised.\n\nPlease use command: Bill_Start(...)\n");
}

void DB_Init_Model::Bill_Trans_Transfer(const wxDateTime& date, const wxString& to_account, double value
    , const wxString& category, const wxString& subcategory, bool advanced, double adv_value)
{
    if (m_bill_initialised && !m_bill_transaction_set)
    {
        m_bill_entry->TOACCOUNTID = Get_Account_ID(to_account);
        m_bill_entry->PAYEEID = -1;

        // Set to Transfer
        m_bill_entry->TRANSCODE = Model_Checking::instance().all_type()[Model_Checking::TRANSFER];
        m_bill_entry->TRANSAMOUNT = value;
        m_bill_entry->STATUS = Model_Checking::all_status()[Model_Checking::RECONCILED].Mid(0,1);

        m_bill_entry->CATEGID = Category_id(category);
        m_bill_entry->SUBCATEGID = Subcategory_id(m_bill_entry->CATEGID, subcategory);
        m_bill_entry->TRANSDATE = date.FormatISODate();
        m_bill_entry->FOLLOWUPID = 0;
        m_bill_entry->TOTRANSAMOUNT = value;
        if (advanced) m_bill_entry->TOTRANSAMOUNT = adv_value;

        m_bill_transaction_set = true;
    }
    else ShowMessage("Bill not initialised.\n\nPlease use command: Bill_Start(...)\n");
}

int DB_Init_Model::BILL_End(bool autoExecuteUserAck, bool autoExecuteSilent)
{
    int bill_id = -1;
    if (m_bill_initialised && m_bill_transaction_set)
    {
        //TODO: Put this multiplex function in the model.
        // Multiplex Auto executable onto the repeat field of the database.
        if (autoExecuteUserAck)
        {
            m_bill_entry->REPEATS += BD_REPEATS_MULTIPLEX_BASE;
            if (autoExecuteSilent)
                m_bill_entry->REPEATS += BD_REPEATS_MULTIPLEX_BASE;
        }

        bill_id = Model_Billsdeposits::instance().save(m_bill_entry);

        m_bill_entry = NULL;     // Reset the pointer
        m_bill_initialised = false;
        m_bill_transaction_set = false;
    }
    else ShowMessage(
        "Bill not initialised.\n\n"
        "Please use command: Bill_Start(...)\n"
        "Followed by one Bill_xxx_Transaction(...) command.");

    return bill_id;
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
//----------------------------------------------------------------------------

template <class Value_Type>
double  Value_List<Value_Type>::Current_Total()
{
    double result = 0;
    for (int pos = 0; pos < (int) list.size(); ++pos)
    {
        result += list[pos].Value();
    }

    return result;
}

//----------------------------------------------------------------------------
Single_Name_List::Single_Name_List()
{}

void Single_Name_List::Set_Value(const wxString& primary_name, const double value)
{
    bool names_exists = false;
    int pos = 0;
    while (pos < (int) m_name_list.list.size())
    {
        if (m_name_list.list[pos].Primary_Name() == primary_name)
        {
            names_exists = true;
            break;
        }
        ++pos;
    }

    if (names_exists) // Update the value
    {
        m_name_list.list[pos].Add_Value(value);
    }
    else    // Create the new value.
    {
        Single_Name named_value(primary_name, value);
        m_name_list.list.push_back(named_value);
    }
}

double Single_Name_List::Current_List_Total()
{
    return m_name_list.Current_Total();
}

Single_Name Single_Name_List::Item(const int pos) const
{
    return m_name_list.list[pos];
}

int Single_Name_List::Size()
{
    return m_name_list.list.size();
}

Single_Name::Single_Name(const wxString& primary_name, const double value)
: m_name(primary_name)
, m_value(value)
{}

wxString Single_Name::Primary_Name() const
{
    return m_name;
}

double Single_Name::Value() const
{
    return m_value;
}

void Single_Name::Add_Value(const double value)
{
    m_value += value;
}
//----------------------------------------------------------------------------

Dual_Name_List::Dual_Name_List()
{}

void Dual_Name_List::Set_Value(const wxString& primary_name, const wxString& secondary_name, const double value)
{
    bool names_exists = false;
    int pos = 0;
    while (pos < (int) m_name_list.list.size())
    {
        if ((m_name_list.list[pos].Primary_Name() == primary_name) && (m_name_list.list[pos].Secondary_Name() == secondary_name))
        {
            names_exists = true;
            break;
        }
        ++pos;
    }

    if (names_exists) // Update the value
    {
        m_name_list.list[pos].Add_Value(value);
    }
    else    // Create the new value.
    {
        Dual_Name named_value(primary_name, secondary_name, value);
        m_name_list.list.push_back(named_value);
    }
}

double Dual_Name_List::Current_List_Total()
{
    return m_name_list.Current_Total();
}

Dual_Name Dual_Name_List::Item(const int pos) const
{
    return m_name_list.list[pos];
}

int Dual_Name_List::Size()
{
    return m_name_list.list.size();
}

Dual_Name::Dual_Name(const wxString& primary_name, const wxString& secondary_name, const double value)
: m_primary_name(primary_name)
, m_secondary_name(secondary_name)
, m_value(value)
{}

wxString Dual_Name::Primary_Name() const
{
    return m_primary_name;
}

wxString Dual_Name::Secondary_Name() const
{
    return m_secondary_name;
}

double Dual_Name::Value() const
{
    return m_value;
}

void Dual_Name::Add_Value(const double value)
{
    m_value += value;
}

//----------------------------------------------------------------------------
int DB_Model_Initialise_Statistics::Add_Trans_Deposit(const wxString& account_name, const wxDateTime& date, const wxString& payee, double value, const wxString& category, const wxString& subcategory)
{
    payee_income_list.Set_Value(payee, value);
    category_income_list.Set_Value(category, value);
    subcategory_income_list.Set_Value(category, subcategory, value);
    return DB_Init_Model::Add_Trans_Deposit(account_name, date, payee, value, category, subcategory);
}

int DB_Model_Initialise_Statistics::Add_Trans_Withdrawal(const wxString& account_name, const wxDateTime& date, const wxString& payee, double value, const wxString& category, const wxString& subcategory)
{
    payee_expense_list.Set_Value(payee, value);
    if (!category.IsEmpty())
    {
        category_expense_list.Set_Value(category, value);
        if (!subcategory.IsEmpty())
        {
            subcategory_expense_list.Set_Value(category, subcategory, value);
        }
    }
    return DB_Init_Model::Add_Trans_Withdrawal(account_name, date, payee, value, category, subcategory);
}

int DB_Model_Initialise_Statistics::Add_Trans_Transfer(const wxString& account_name, const wxDateTime& date, const wxString& to_account, double value, const wxString& category, const wxString& subcategory, bool advanced, double adv_value)
{
    if (!category.IsEmpty())
    {
        category_income_list.Set_Value(category, value);
        category_expense_list.Set_Value(category, value);
        if (!subcategory.IsEmpty())
        {
            subcategory_income_list.Set_Value(category, subcategory, value);
            subcategory_expense_list.Set_Value(category, subcategory, value);
        }
    }
    return DB_Init_Model::Add_Trans_Transfer(account_name, date, to_account, value, category, subcategory, advanced, adv_value);
}

void DB_Model_Initialise_Statistics::Add_Trans_Split(int trans_id, double value, const wxString& category, const wxString& subcategory)
{
    if (!category.IsEmpty())
    {
        if (value < 0)
        {
            category_expense_list.Set_Value(category, value);
            if (!subcategory.IsEmpty())
            {
                subcategory_expense_list.Set_Value(category, subcategory, value);
            }
        }
        else
        {
            category_income_list.Set_Value(category, value);
            if (!subcategory.IsEmpty())
            {
                subcategory_income_list.Set_Value(category, subcategory, value);
            }
        }
    }
    DB_Init_Model::Add_Trans_Split(trans_id, value, category, subcategory);
}


void DB_Model_Initialise_Statistics::Current_Single_Name_Stats(const wxString& title, const wxDateTime& starting_date, Single_Name_List& single_name_list)
{
    Add_Asset(title, starting_date, 0, Model_Asset::TYPE::TYPE_OTHER);
    for (int pos = 0; pos < single_name_list.Size(); ++pos)
    {
        Add_Asset(single_name_list.Item(pos).Primary_Name(), starting_date, single_name_list.Item(pos).Value());
    }
}

void DB_Model_Initialise_Statistics::Current_Payee_Income_Stats(const wxString& title, const wxDateTime& starting_date)
{
    Current_Single_Name_Stats(title, starting_date, payee_income_list);
}

void DB_Model_Initialise_Statistics::Current_Payee_Expense_Stats(const wxString& title, const wxDateTime& starting_date)
{
    Current_Single_Name_Stats(title, starting_date, payee_expense_list);
}

void DB_Model_Initialise_Statistics::Current_Category_Income_Stats(const wxString& title, const wxDateTime& starting_date)
{
    Current_Single_Name_Stats(title, starting_date, category_income_list);
}

void DB_Model_Initialise_Statistics::Current_Category_Expense_Stats(const wxString& title, const wxDateTime& starting_date)
{
    Current_Single_Name_Stats(title, starting_date, category_expense_list);
}

void DB_Model_Initialise_Statistics::Current_Dual_Name_Stats(const wxString& title, const wxDateTime& starting_date, Dual_Name_List& dual_name_list)
{
    Add_Asset(title, starting_date, 0, Model_Asset::TYPE::TYPE_OTHER);
    for (int pos = 0; pos < dual_name_list.Size(); ++pos)
    {
        Add_Asset(dual_name_list.Item(pos).Primary_Name() + ":" + dual_name_list.Item(pos).Secondary_Name(), starting_date, dual_name_list.Item(pos).Value());
    }
}

void DB_Model_Initialise_Statistics::Current_Subcategory_Income_Stats(const wxString& title, const wxDateTime& starting_date)
{
    Current_Dual_Name_Stats(title, starting_date, subcategory_income_list);
}

void DB_Model_Initialise_Statistics::Current_Subcategory_Expense_Stats(const wxString& title, const wxDateTime& starting_date)
{
    Current_Dual_Name_Stats(title, starting_date, subcategory_expense_list);
}
//----------------------------------------------------------------------------

void DB_Model_Initialise_Statistics::Total_Payee_Stats(const wxDateTime& starting_date)
{
    Add_Asset("-------------------- Total Income for Payees:", starting_date, payee_income_list.Current_List_Total(), Model_Asset::TYPE::TYPE_OTHER);
    Add_Asset("-------------------- Total Expenses for Payees:", starting_date, payee_expense_list.Current_List_Total(), Model_Asset::TYPE::TYPE_OTHER);
}

void DB_Model_Initialise_Statistics::Total_Category_Stats(const wxDateTime& starting_date)
{
    Add_Asset("-------------------- Total Income for Categories:", starting_date, category_income_list.Current_List_Total(), Model_Asset::TYPE::TYPE_OTHER);
    Add_Asset("-------------------- Total Expenses for Categories:", starting_date, category_expense_list.Current_List_Total(), Model_Asset::TYPE::TYPE_OTHER);
}

void DB_Model_Initialise_Statistics::Total_Subcategory_Stats(const wxDateTime& starting_date)
{
    Add_Asset("-------------------- Total Income for Subcategories:", starting_date, subcategory_income_list.Current_List_Total(), Model_Asset::TYPE::TYPE_OTHER);
    Add_Asset("-------------------- Total Expenses for Subcategories:", starting_date, subcategory_expense_list.Current_List_Total(), Model_Asset::TYPE::TYPE_OTHER);
}
