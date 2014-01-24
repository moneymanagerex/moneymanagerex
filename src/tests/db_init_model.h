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

#pragma once

#include "model/Model_Account.h"
#include "model/Model_Asset.h"

class Model_Checking;

class DB_Init_Model
{
public:
    DB_Init_Model();
    virtual ~DB_Init_Model();

    /** Initialise all tables and options required for the database */
    void Init_Model_Tables(wxSQLite3Database* test_db);

    /** Initialise only the tables required for Assets in the database */
    void Init_Model_Assets(wxSQLite3Database* test_db);

    /** Initialise only the tables required for Stocks in the database */
    void Init_Model_Stocks(wxSQLite3Database* test_db);

    // Used only once. Set the username and base currency
    void Init_BaseCurrency(const wxString& base_currency_symbol = "AUD", const wxString& user_name = "Test Database");
    
    int Add_Bank_Account(const wxString& name, double initial_value = 0, const wxString& notes = "", bool favorite = true, const wxString& currency_symbol = "AUD");
    int Add_Investment_Account(const wxString& name, double initial_value = 0, const wxString& notes = "", bool favorite = true, const wxString& currency_symbol = "AUD");
    int Add_Term_Account(const wxString& name, double initial_value = 0, const wxString& notes = "", bool favorite = true, const wxString& currency_symbol = "AUD");
    int Add_Account(const wxString& name, Model_Account::TYPE account_type, double initial_value, const wxString& notes, bool favorite, const wxString& currency_symbol = "AUD");

    int Add_Payee(const wxString& name, const wxString& category = "", const wxString& subcategory = "");
    void Add_payee_category(const wxString& name, const wxString& category_name, const wxString& subcategory_name = ""); 
    int Payee_id(const wxString& name);
    
    int Add_Category(const wxString& name);
    int Category_id(const wxString& category);

    int Add_Subcategory(int category_id, const wxString& name);
    int Subcategory_id(int category_id, const wxString& subcategory);

    /** Set the account name for Add_Trans_xxx commands */
    void Set_AccountName(const wxString& account_name);
    int Get_Account_ID(const wxString& account_name);
    /**
    If category not supplied, assume that it is a split.
    * Returns the transaction ID to enamle split creation.
    */
    int Add_Trans_Deposit(const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    
    /**
    If category not supplied, assume that it is a split.
    * Returns the transaction ID to enamle split creation.
    */
    int Add_Trans_Withdrawal(const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    
    /**
    If category not supplied, assume that it is a split.
    * Returns the transaction ID to enamle split creation.
    */
    int Add_Trans_Transfer(const wxDateTime& date, const wxString& to_account, double value
        , const wxString& category = "", const wxString& subcategory = "", bool advanced = false, double adv_value = 0);
    
    /** Add_Trans command If category not supplied, assume that it is a split. */
    void Add_Trans_Split(int trans_id, double value, const wxString& category, const wxString& subcategory = "");

    /**
    Starts the sequence to create a Repeating transaction using commands:
    Bill_Start(...)
    Bill_Trans_xxx(...)
    Bill_Trans_End(...) - Provides ID to use command:Add_Bill_Split(...)
    */
    void Bill_Start(const wxString& account, const wxDateTime& start_date, Model_Billsdeposits::REPEAT_TYPE repeats, int num_occur = -1);
    void Bill_Trans_Deposit(const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    void Bill_Trans_Withdrawal(const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    void Bill_Trans_Transfer(const wxDateTime& date, const wxString& to_account, double value
        , const wxString& category = "", const wxString& subcategory = "", bool advanced = false, double adv_value = 0);
    int BILL_End(bool execute_auto_manual = false, bool execute_full_auto = false);
    void Add_Bill_Split(int bill_id, double value, const wxString& category, const wxString& subcategory = "");

    int Add_Asset(const wxString& name, const wxDate& date, double value, Model_Asset::TYPE asset_type,
        Model_Asset::RATE value_change, double value_change_rate, const wxString& notes = "");

    void ShowMessage(wxString msg);

private:
    int m_baseCurrency_id;
    wxString m_account_name;    // Initialised by Set_AccountName(...), Used by Add_Trans_XXX(...) Commands.
    int m_account_id;           // Initialised by Set_AccountName(...), Used by Add_Trans_XXX(...) Commands.

    int Add_Trans(Model_Checking::TYPE trans_type, const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");


    bool m_bill_initialised;      // Set to true by Bill_Start(...)
    bool m_bill_transaction_set;  // Set to true by any Bill_xxx_Transaction(...) commands 
    Model_Billsdeposits::Data* m_bill_entry;  // Used by Bill_Set(...) to complete the command and save the transaction.
    void Bill_Transaction(Model_Checking::TYPE trans_type, const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
};
