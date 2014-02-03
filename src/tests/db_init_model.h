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
    int Get_Payee_id(const wxString& name);

    int Add_Category(const wxString& name);
    int Get_category_id(const wxString& category);

    int Add_Subcategory(int category_id, const wxString& name);
    int Get_subcategory_id(int category_id, const wxString& subcategory);

    int Get_account_id(const wxString& account_name);
    /**
    Allows the creation of a Split Transaction when a category is not supplied.
    * Returns the trans_id to enable split creation.
    */
    int Add_Trans_Deposit(const wxString& account_name, const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    /**
    Allows the creation of a Split Transaction when a category is not supplied.
    * Returns the trans_id to enable split creation.
    */
    int Add_Trans_Withdrawal(const wxString& account_name, const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    /**
    Allows the creation of a Split Transaction when a category is not supplied.
    * Returns the trans_id to enable split creation.
    */
    int Add_Trans_Transfer(const wxString& account_name, const wxDateTime& date, const wxString& to_account, double value
        , const wxString& category = "", const wxString& subcategory = "", bool advanced = false, double adv_value = 0);

    /** Creates a Split Transaction after a transaction has been created.*/
    void Add_Trans_Split(int trans_id, double value, const wxString& category, const wxString& subcategory = "");

    /**
    Starts the sequence to create a Repeating transaction using commands:
    Bill_Start(...)     - Sets the repeating conditions 
    Bill_Trans_xxx(...) - Creates the bill transaction for the final transaction
    Bill_Trans_End(...) - Provides bill_id for the command: Bill_Split(...)
    */
    void Bill_Start(const wxString& account, const wxDateTime& start_date, Model_Billsdeposits::REPEAT_TYPE repeats, int num_occur = -1);
    /**
    Create the transaction for a Repeating transaction.
    See command: Bill_Start(...)
    */
    void Bill_Trans_Deposit(const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    /**
    Create the transaction for a Repeating transaction.
    See command: Bill_Start(...)
    */
    void Bill_Trans_Withdrawal(const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    /**
    Create the transaction for a Repeating transaction.
    See command: Bill_Start(...)
    */
    void Bill_Trans_Transfer(const wxDateTime& date, const wxString& to_account, double value
        , const wxString& category = "", const wxString& subcategory = "", bool advanced = false, double adv_value = 0);
    /**
    End the sequence to create a Repeating transaction.
    See command: Bill_Start(...)
    * Returns the bill_id to create a Bill_Split(...)
    */
    int BILL_End(bool execute_auto_manual = false, bool execute_full_auto = false);

    /**
    Create a Bill Split Transaction entry, after completion of a Bill entry using the command:
    Bill_Trans_End(...) - See command: Bill_Start(...)
    */
    void Bill_Split(int bill_id, double value, const wxString& category, const wxString& subcategory = "");

    int Add_Asset(const wxString& name, const wxDate& date, double value, Model_Asset::TYPE asset_type = Model_Asset::TYPE_CASH,
        Model_Asset::RATE value_change = Model_Asset::RATE::RATE_NONE, double value_change_rate = 0, const wxString& notes = "");

    void ShowMessage(wxString msg);

private:
    int m_baseCurrency_id;
    wxString m_account_name;    // Initialised by Set_AccountName(...), Used by Add_Trans_XXX(...) Commands.
    int m_account_id;           // Initialised by Set_AccountName(...), Used by Add_Trans_XXX(...) Commands.

    int Add_Trans(const wxString& account_name, Model_Checking::TYPE trans_type, const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");


    bool m_bill_initialised;      // Set to true by Bill_Start(...)
    bool m_bill_transaction_set;  // Set to true by any Bill_xxx_Transaction(...) commands 
    Model_Billsdeposits::Data* m_bill_entry;  // Used by Bill_Set(...) to complete the command and save the transaction.
    void Bill_Transaction(Model_Checking::TYPE trans_type, const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
};
//----------------------------------------------------------------------------

template <class Value_Type>
class Value_List
{
public:
    std::vector<Value_Type> list;

    double Current_Total();
    double Primary_Name_Value(const wxString& name);
};
//----------------------------------------------------------------------------

class Single_Name
{
public:
    Single_Name(const wxString& primary_name, const double value);

    wxString Primary_Name() const;
    double Value() const;
    void Add_Value(const double value);

private:
    wxString m_name;
    double m_value;
};

class Single_Name_List
{
public:
    Single_Name_List();
    void Set_Value(const wxString& primary_name, const double value);
    double Primary_Name_Value(const wxString& name);
    double Current_List_Total();
    Single_Name Item(const int pos) const;
    int Size();

private:
    Value_List<Single_Name> m_name_list;
};
//----------------------------------------------------------------------------

class Dual_Name
{
public:
    Dual_Name(const wxString& primary_name, const wxString& secondary_name, const double value);

    wxString Primary_Name() const;
    wxString Secondary_Name() const;
    double Value() const;
    void Add_Value(const double value);

private:
    wxString m_primary_name;
    wxString m_secondary_name;
    double m_value;
};

class Dual_Name_List
{
public:
    Dual_Name_List();
    void Set_Value(const wxString& primary_name, const wxString& secondary_name, const double value);
    double Primary_Name_Value(const wxString& name);
    double Secondary_Name_Value(const wxString& primary_name, const wxString& secondary_name);
    double Current_List_Total();
    Dual_Name Item(const int pos) const;
    int Size();

private:
    Value_List<Dual_Name> m_name_list;
};
//----------------------------------------------------------------------------

class DB_Model_Initialise_Statistics : public DB_Init_Model
{
public:
    /**
    Allows the creation of a Split Transaction when a category is not supplied.
    * Returns the transaction ID to enable split creation.
    */
    int Add_Trans_Deposit(const wxString& account_name, const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    /**
    Allows the creation of a Split Transaction when a category is not supplied.
    * Returns the transaction ID to enable split creation.
    */
    int Add_Trans_Withdrawal(const wxString& account_name, const wxDateTime& date, const wxString& payee, double value
        , const wxString& category = "", const wxString& subcategory = "");
    /**
    Allows the creation of a Split Transaction when a category is not supplied.
    * Returns the transaction ID to enable split creation.
    */
    int Add_Trans_Transfer(const wxString& account_name, const wxDateTime& date, const wxString& to_account, double value
        , const wxString& category = "", const wxString& subcategory = "", bool advanced = false, double adv_value = 0);

    /** Creates a Split Transaction after a transaction has been created.*/
    void Add_Trans_Split(int trans_id, double value, const wxString& category, const wxString& subcategory = "");

    /* Creates Asset entries for each Payee showing the Income amounts.*/
    void Current_Payee_Income_Stats(const wxString& title, const wxDateTime& starting_date);
    /* Creates Asset entries for each Payee showing the Expense amounts.*/
    void Current_Payee_Expense_Stats(const wxString& title, const wxDateTime& starting_date);

    /* Creates Asset entries for each Category showing the Income amounts.*/
    void Current_Category_Income_Stats(const wxString& title, const wxDateTime& starting_date);
    /* Creates Asset entries for each Category showing the Expense amounts.*/
    void Current_Category_Expense_Stats(const wxString& title, const wxDateTime& starting_date);

    /* Creates Asset entries for each Subcategory showing the Income amounts.*/
    void Current_Subcategory_Income_Stats(const wxString& title, const wxDateTime& starting_date);
    /* Creates Asset entries for each Subcategory showing the Expense amounts.*/
    void Current_Subcategory_Expense_Stats(const wxString& title, const wxDateTime& starting_date);
    
    /* Creates Asset entries for the total Income and Expense amounts for a payee.*/
    void Total_Payee_Stats(const wxDateTime& starting_date);
    /* Creates Asset entries for the total Income and Expense amounts for a category.*/
    void Total_Category_Stats(const wxDateTime& starting_date);
    /* Creates Asset entries for the total Income and Expense amounts for a subcategory.*/
    void Total_Subcategory_Stats(const wxDateTime& starting_date);

    /* Returns the current total Income for the payee.*/
    double Payee_Income(const wxString& name);
    /* Returns the current total Expense for the payee.*/
    double Payee_Expense(const wxString& name);
    /* Returns the current total Income for the Category.*/
    double Category_Income(const wxString& name);
    /* Returns the current total Expense for the Category.*/
    double Category_Expense(const wxString& name);
    /* Returns the current total Income for the Category:Subcategory combination.*/
    double Subcategory_Income(const wxString& primary_name, const wxString& secondary_name);
    /* Returns the current total Expense for the Category:Subcategory combination.*/
    double Subcategory_Expense(const wxString& primary_name, const wxString& secondary_name);

private:
    Single_Name_List m_payee_income_list;
    Single_Name_List m_payee_expense_list;
    Single_Name_List m_category_income_list;
    Single_Name_List m_category_expense_list;
    Dual_Name_List m_subcategory_income_list;
    Dual_Name_List m_subcategory_expense_list;

    void Current_Single_Name_Stats(const wxString& title, const wxDateTime& starting_date, Single_Name_List& single_name_list);
    void Current_Dual_Name_Stats(const wxString& title, const wxDateTime& starting_date, Dual_Name_List& dual_name_list);
};
