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
    
    int Add_Account(const wxString& name, Model_Account::TYPE account_type, wxString currency_symbol = "AUD");

    int Add_Payee(const wxString& name, const wxString& category = "", const wxString& subcategory = "");
    void Add_payee_category(const wxString& name, const wxString& category_name, const wxString& subcategory_name = ""); 
    int Payee_id(const wxString& name);
    
    int Add_category(const wxString& name);
    int Category_id(const wxString& category);

    int Add_subcategory(const wxString& name, int category_id);
    int Subcategory_id(const wxString& subcategory, int category_id);

    /** Set the account name for Add_Trans_xxx commands */
    void Set_AccountName(const wxString& account_name); 

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

    void Add_Bill_Split(int trans_id, double value, const wxString& category, const wxString& subcategory = "");

//    void Add_Bill(const wxDate& start_date); // set processing_bill
//    void SetNormalRepeat(BILL_REPEAT repeat_type);
//    void SetAdvancedRepeat(BILL_ADVANCED repeat_type, int period = 1);
    void EndBILL();

    int Add_Asset(const wxString& name, const wxDate& date, double value, Model_Asset::TYPE asset_type,
        Model_Asset::RATE value_change, double value_change_rate, const wxString& notes = "");

    void ShowMessage(wxString msg);

private:
    int m_baseCurrency_id;
    wxString m_account_name;
    int m_account_id;
    
    bool m_processing_bill;
};
