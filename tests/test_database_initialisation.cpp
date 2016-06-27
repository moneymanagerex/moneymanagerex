/*******************************************************
Copyright (C) 2013 James Higley
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
Foundation, Inc., 59 Temple Placeuite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "defs.h"
#include <cppunit/config/SourcePrefix.h>
#include "cpu_timer.h"
#include "db_init_model.h"
#include "framebase_tests.h"
//----------------------------------------------------------------------------
#include "test_database_initialisation.h"
#include "reports/mmDateRange.h"
#include "model/Model_Translink.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_TEST_DATABASE_INITIALISATION
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_DatabaseInitialisation);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_DatabaseInitialisation::Test_DatabaseInitialisation()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_mmex_db_" + wxDateTime(wxDateTime::Today()).FormatISODate() + ".mmb";
    // recreate the file if run twice on same day.
    if ((m_this_instance == 1) && wxFileExists(m_test_db_filename))
    {
        wxRemoveFile(m_test_db_filename);
    }
}

Test_DatabaseInitialisation::~Test_DatabaseInitialisation()
{
    s_instance_count--;
    m_this_instance = s_instance_count;
    if (s_instance_count < 1)
    {
        TestFrameBase base_frame(m_this_instance);
        base_frame.Show();
        wxMessageBox("Test database files: test_mmex_db*.* have not been deleted."
            , "Test: DatabaseInitialisation", wxOK | wxICON_WARNING, &base_frame);
    }
}

void Test_DatabaseInitialisation::setUp()
{
    CpuTimer time("Startup");

    m_test_db.Open(m_test_db_filename);

    m_dbmodel = new DB_Model_Initialise_Statistics();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_DatabaseInitialisation::tearDown()
{
    delete m_dbmodel;
    m_test_db.Close();
}

void Test_DatabaseInitialisation::Add_Account_Entries()
{
    CpuTimer Start("Account  Entries");

    // Add accounts
    m_test_db.Begin();
    m_dbmodel->Add_Bank_Account("NAB - Savings", 0, "Joint - General Account");
    m_dbmodel->Add_Bank_Account("ANZ - Cheque", 0, "", false);

    m_dbmodel->Add_Term_Account("ANZ - Cash Manager", 0, "Savings for special ocasions, Higher Interest Account", false);

    m_dbmodel->Add_Account("Loan: Personal ANZ", Model_Account::LOAN, 0, "$10,000 @ 20% pa", false);

    m_dbmodel->Add_Bank_Account("ANZ - Mastercard", 0, "Credit Card");
    m_dbmodel->Add_Bank_Account("Wallet - Peter", 0, "Cash Money - Daily Expenses");
    m_dbmodel->Add_Bank_Account("Wallet - Mary", 0, "Cash Money - Daily Expenses");

    m_dbmodel->Add_Account("ACME Corporation Shares", Model_Account::SHARES, 0, "Share Dividends", false);
    m_dbmodel->Add_Account("ABC Corporation Shares", Model_Account::SHARES, 0, "Share Dividends", false);
    m_dbmodel->Add_Term_Account("Insurance Policies");

    m_dbmodel->Add_Investment_Account("Broker Jim", 0, "Stocks");

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Payee_Entries()
{
    CpuTimer Start("Payee  Entries");

    // Add Payees
    m_test_db.Begin();
    m_dbmodel->Add_Payee("Supermarket", "Food", "Groceries");
    m_dbmodel->Add_Payee("Aldi", "Food", "Groceries");
    m_dbmodel->Add_Payee("Coles", "Food", "Groceries");
    m_dbmodel->Add_Payee("Woolworths", "Food", "Groceries");
    m_dbmodel->Add_Payee("Peter", "Income", "Salary");
    m_dbmodel->Add_Payee("Mary", "Income", "Salary");
    m_dbmodel->Add_Payee("Bank - NAB");
    m_dbmodel->Add_Payee("Bank - ANZ");
    m_dbmodel->Add_Payee("Government Authorities");
    m_dbmodel->Add_Payee("Utility Provider");
    m_dbmodel->Add_Payee("Cash - Miscellaneous");

    m_dbmodel->Add_Payee("ACME Corporation");
    m_dbmodel->Add_Payee("ABC Corporation");
    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Category_Entries()
{
    CpuTimer Start("Category Entries");
    m_test_db.Begin();

    // Category structure: "Family Home"
    int cat_id = m_dbmodel->Add_Category("Home");
    m_dbmodel->Add_Subcategory(cat_id, "General Rates");
    m_dbmodel->Add_Subcategory(cat_id, "Water Rates");
    m_dbmodel->Add_Subcategory(cat_id, "Electricity");
    m_dbmodel->Add_Subcategory(cat_id, "Gas");
    m_dbmodel->Add_Subcategory(cat_id, "Phone/Internet");
    m_dbmodel->Add_Subcategory(cat_id, "Insurance");
    m_dbmodel->Add_Subcategory(cat_id, "Loan Repayments");
    m_dbmodel->Add_Subcategory(cat_id, "Loan Offset");
    m_dbmodel->Add_Subcategory(cat_id, "Loan Interest");
    m_dbmodel->Add_Subcategory(cat_id, "Purchase");

    // Category structure: "Mastercard"
    cat_id = m_dbmodel->Add_Category("Mastercard");
    m_dbmodel->Add_Subcategory(cat_id, "Repayment");
    m_dbmodel->Add_Subcategory(cat_id, "Annual Fee");

    // Category structure: "Personal Loan"
    cat_id = m_dbmodel->Add_Category("Personal Loan");
    m_dbmodel->Add_Subcategory(cat_id, "Repayments");
    m_dbmodel->Add_Subcategory(cat_id, "Offset");
    m_dbmodel->Add_Subcategory(cat_id, "Interest");

    // Extend Category structure: "Income"
    cat_id = m_dbmodel->Get_category_id("Income");
    m_dbmodel->Add_Subcategory(cat_id, "Tax");
    m_dbmodel->Add_Subcategory(cat_id, "Bank Interest");
    m_dbmodel->Add_Subcategory(cat_id, "Share Dividend");

    // Extend Category structure: "Transfer"
    cat_id = m_dbmodel->Get_category_id("Transfer");
    m_dbmodel->Add_Subcategory(cat_id, "Share Dividend");
    m_dbmodel->Add_Subcategory(cat_id, "Share Purchase");
    m_dbmodel->Add_Subcategory(cat_id, "Bank Loan");
    m_dbmodel->Add_Subcategory(cat_id, "Spending Money");
    m_dbmodel->Add_Subcategory(cat_id, "Saving");

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Asset_Entries()
{
    CpuTimer Start("Asset Entries");

    mmCurrentFinancialYear current_financial_year(1, 7);
    wxDateTime starting_date = current_financial_year.start_date();

    wxDateTime asset_date = starting_date.Subtract(wxDateSpan::Years(8));
    m_test_db.Begin();

    //----------------------------------------------------------------------------------------
    // Asset: Family Home

    // Set up some additional details
    m_dbmodel->Add_Payee("Toyota Car Sales");
    m_dbmodel->Add_Subcategory(m_dbmodel->Get_category_id("Automobile"), "Purchase");
    const wxString auto_asset = "Asset: Toyota Sedan";

    // Create the asset entry
    int asset_id = m_dbmodel->Add_Asset(auto_asset, asset_date, 10000, Model_Asset::TYPE_AUTO, Model_Asset::RATE_DEPRECIATE, 5.0, "New Car Depreciates 5% pa");

    // Create the asset account and the initial transaction entry
    m_dbmodel->Add_Account(auto_asset, Model_Account::ASSET, 0, "New car - Toyota Sedan", false);
    int checking_id = m_dbmodel->Add_Trans_Withdrawal(auto_asset, asset_date, "Toyota Car Sales", 10000, "Automobile", "Purchase", auto_asset);

    // Link the asset entry to the transaction
    Model_Translink::SetAssetTranslink(asset_id, checking_id);

    // Create a loan to pay off the car.
    m_dbmodel->Add_Account("Loan: Toyota Sedan", Model_Account::LOAN, 0, "$10,000 @ 20% pa", false);
    m_dbmodel->Add_Trans_Transfer("Loan: Toyota Sedan", asset_date, auto_asset, 10000, "Automobile", "Purchase");

    //----------------------------------------------------------------------------------------
    // Asset: Family Home
    asset_date = starting_date.Add(wxDateSpan::Years(1).Subtract(wxDateSpan::Months(3)));
    m_dbmodel->Add_Payee("Global Realestate Sales");
    const wxString asset_familyhome = "Asset: Family Home";

    // Create the asset entry
    asset_id = m_dbmodel->Add_Asset(asset_familyhome, asset_date, 250000, Model_Asset::TYPE_PROPERTY, Model_Asset::RATE_APPRECIATE, 2.0, "Home apprectates at 2% pa");

    // Create the asset account and the initial transaction entry
    m_dbmodel->Add_Account(asset_familyhome, Model_Account::ASSET, 0, "Family home purchase", false);
    checking_id = m_dbmodel->Add_Trans_Withdrawal(asset_familyhome, asset_date, "Global Realestate Sales", 250000, "Home", "Purchase", asset_familyhome);

    // Link the asset entry to the transaction
    Model_Translink::SetAssetTranslink(asset_id, checking_id);

    // Create a loan to pay off the Family Home.
    m_dbmodel->Add_Account("Loan: Family Home", Model_Account::LOAN, 0, "Family Home Mortgage", false);
    m_dbmodel->Add_Trans_Transfer("Loan: Family Home", asset_date, asset_familyhome, 250000, "Home", "Purchase");

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Recurring_Transaction_Entries()
{
    CpuTimer Start("Repeat Transaction Entries");


    m_test_db.Begin();

    wxDateTime bill_date = wxDateTime::Today().Add(wxDateSpan::Days(3));
    m_dbmodel->Bill_Start("ANZ - Mastercard", bill_date, Model_Billsdeposits::REPEAT_WEEKLY);
    m_dbmodel->Bill_Trans_Withdrawal(bill_date, "Woolworths", 100);
    int bill_id = m_dbmodel->BILL_End();

    m_dbmodel->Bill_Split(bill_id, 10, "Food", "Groceries");
    m_dbmodel->Bill_Split(bill_id, 30, "Food", "Groceries");
    m_dbmodel->Bill_Split(bill_id, 60, "Food", "Groceries");

    mmCurrentFinancialYear current_financial_year(1, 7);
    bill_date = current_financial_year.start_date();
    bill_date.Subtract(wxDateSpan::Years(7).Subtract(wxDateSpan::Months(4)));

    while (bill_date < wxDateTime::Today())
    {
        m_dbmodel->Add_Trans_Withdrawal("Loan: Family Home", bill_date, "Bank - NAB", 2000, "Home", "Loan Interest");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", bill_date, "Loan: Family Home", 3500, "Home", "Loan Repayments");
        bill_date.Add(wxDateSpan::Month());
    }

    m_dbmodel->Bill_Start("Loan: Family Home", bill_date, Model_Billsdeposits::REPEAT_MONTHLY);
    m_dbmodel->Bill_Trans_Withdrawal(bill_date, "Bank - NAB", 2000, "Home", "Loan Interest");
    m_dbmodel->BILL_End(true);

    m_dbmodel->Bill_Start("NAB - Savings", bill_date, Model_Billsdeposits::REPEAT_MONTHLY);
    m_dbmodel->Bill_Trans_Transfer(bill_date, "Loan: Family Home", 3500, "Home", "Loan Repayments");
    m_dbmodel->BILL_End(true);


    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Stock_and_Share_Entries()
{
    CpuTimer Start("Stock and Share Entries");

    mmCurrentFinancialYear current_financial_year(1, 7);
    wxDateTime starting_date = current_financial_year.start_date();

    // Set start date 3 years ago.
    starting_date.Subtract(wxDateSpan::Years(3));
 
    m_test_db.Begin();

    // Setting up all entries for Stocks
    Add_Stock_Entries(starting_date);

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Stock_Entries(const wxDateTime& starting_date)
{
    wxDateTime trans_date = starting_date;

    // Setting up a stock portfolio
    int stock_Yahoo_Finance_id = m_dbmodel->Add_Investment_Account("Yahoo Finance", 0, "Stocks");

    // Setting up history for Telstra at start of a financial year
    trans_date = starting_date;
    wxString stock_symbol = "tls.ax";
    double num_shares = 1000;
    double share_value = 4.75;
    int stock_entry_id = m_dbmodel->Add_Stock_Entry(stock_Yahoo_Finance_id, trans_date, num_shares, share_value, 0, share_value, 0, "Telstra", stock_symbol);
    trans_date.Add(wxDateSpan::Days(10));

    double share_dif = 0.25;
    int share_cycle = 0;
    for (int i = 0; trans_date < wxDateTime::Today(); ++i)
    {
        Model_StockHistory::instance().addUpdate(stock_symbol, trans_date, share_value, Model_StockHistory::MANUAL);
        if (share_cycle <= 15)
        {
            share_value += share_dif;
            ++share_cycle;
            if (share_cycle == 15) share_cycle = 35;
        }
        else
        {
            share_value -= share_dif;
            --share_cycle;
            if (share_cycle == 25) share_cycle = 0;
        }
        trans_date.Add(wxDateSpan::Days(7));
    }

    // Setting up history for AMP at start of a year
    trans_date = starting_date;
    stock_symbol = "amp.ax";
    num_shares = 750;
    share_value = 2.75;
    trans_date.Add(wxDateSpan::Months(6));
    stock_entry_id = m_dbmodel->Add_Stock_Entry(stock_Yahoo_Finance_id, trans_date, num_shares, share_value, 0, share_value, 0, "AMP", stock_symbol);
    trans_date.Add(wxDateSpan::Days(15));

    share_dif = 0.25;
    share_cycle = 0;
    for (int i = 0; trans_date < wxDateTime::Today(); ++i)
    {
        Model_StockHistory::instance().addUpdate(stock_symbol, trans_date, share_value, Model_StockHistory::MANUAL);
        if (share_cycle <= 10)
        {
            share_value += share_dif;
            ++share_cycle;
            if (share_cycle == 10) share_cycle = 25;
        }
        else
        {
            share_value -= share_dif;
            --share_cycle;
            if (share_cycle == 20) share_cycle = 0;
        }
        trans_date.Add(wxDateSpan::Days(3));
    }

    // Setting up history for IAG
    trans_date = wxDateTime::Today().Subtract(wxDateSpan::Year());
    stock_symbol = "iag.ax";
    num_shares = 500;
    share_value = 3.75;
    stock_entry_id = m_dbmodel->Add_Stock_Entry(stock_Yahoo_Finance_id, trans_date, num_shares, share_value, 0, share_value, 0, "IAG", stock_symbol);
    trans_date = wxDateTime::Today().Subtract(wxDateSpan::Days(90));

    share_dif = 0.25;
    share_cycle = 0;
    for (int i = 0; i < 45; ++i)
    {
        Model_StockHistory::instance().addUpdate(stock_symbol, trans_date, share_value, Model_StockHistory::ONLINE);
        if (share_cycle <= 5)
        {
            share_value += share_dif;
            ++share_cycle;
            if (share_cycle == 5) share_cycle = 10;
        }
        else
        {
            share_value -= share_dif;
            --share_cycle;
            if (share_cycle == 5) share_cycle = 0;
        }
        trans_date.Add(wxDateSpan::Days(2));
    }
}

void Test_DatabaseInitialisation::Add_Transaction_Entries()
{
    CpuTimer Start("Transaction Entries");

    wxDateTime starting_date(wxDateTime::Today());

    // Set date to the beginning of current financial year. Assume 1st July.
    int month = starting_date.GetMonth();
    if (month < wxDateTime::Jul)
    {
        starting_date.Subtract(wxDateSpan::Year()).Add(wxDateSpan::Months(wxDateTime::Jul - month));
    }
    else
    {
        starting_date.Subtract(wxDateSpan::Months(month - wxDateTime::Jul));
    }

    // readjust day to beginning of the month
    starting_date.Subtract(wxDateSpan::Days(starting_date.GetDay() - 1));

    mmCurrentFinancialYear current_financial_year(1, 7);
    CPPUNIT_ASSERT_EQUAL(starting_date.FormatISODate(), current_financial_year.start_date().FormatISODate());

    // Set start date 3 years ago from today.
    starting_date.Subtract(wxDateSpan::Years(3));

    wxDateTime trans_date = starting_date;
    m_test_db.Begin();  // Set all data to memory first, then save to database at end.

    // Setting up a personal loan
    int personal_loan_id = m_dbmodel->Add_Trans_Transfer("Loan: Personal ANZ", trans_date, "ANZ - Cheque", 10000, "Transfer", "Bank Loan");
    Model_Checking::Data* personal_loan = Model_Checking::instance().get(personal_loan_id);
    personal_loan->NOTES = "Initialise $10,000 Personal loan from ANZ -Bank";
    Model_Checking::instance().save(personal_loan);

    // Create transactions for a single month. These are repeated untill current month.
    int month_count = 0;

    while (trans_date < wxDateTime::Today())
    {
        month_count++;
        trans_date = starting_date;
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(8)), "Peter", 1200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Peter", 300.0, "Income", "Tax");
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(7)), "Peter", 1500.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Peter", 375.0, "Income", "Tax");
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(7)), "Peter", 1200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Peter", 300.0, "Income", "Tax");
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(7)), "Peter", 1200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Peter", 300.0, "Income", "Tax");
        //--------------------------------------------------------------------

        trans_date = starting_date;
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(14)), "Mary", 1600.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Mary", 375.0, "Income", "Tax");
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(14)), "Mary", 1700.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Mary", 375.0, "Income", "Tax");
        //--------------------------------------------------------------------

        trans_date = starting_date;
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(1)), "Aldi", 50, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 30, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Woolworths", 40, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 100, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 30, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Aldi", 40, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 60, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 80, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(2)), "Woolworths", 30, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Coles", 40, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 50, "Food", "Groceries");
        //--------------------------------------------------------------------

        trans_date = starting_date;
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date.Add(wxDateSpan::Days(1)), "Wallet - Peter", 70, "Transfer", "Spending Money");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 80, "Transfer", "Spending Money");

        m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(7)), "Cash - Miscellaneous", 70, "Miscellaneous");
        m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 80, "Miscellaneous");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Peter", 70, "Transfer", "Spending Money");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 100, "Transfer", "Spending Money");

        m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(7)), "Cash - Miscellaneous", 70, "Miscellaneous");
        m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 100, "Miscellaneous");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Peter", 60, "Transfer", "Spending Money");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 90, "Transfer", "Spending Money");

        // Add split Transactions
        int split_id = m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(8)), "Cash - Miscellaneous", 60);
        m_dbmodel->Add_Trans_Split(split_id, 20, "Leisure", "Magazines");
        m_dbmodel->Add_Trans_Split(split_id, 40, "Miscellaneous");

        split_id = m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 90);
        m_dbmodel->Add_Trans_Split(split_id, 30, "Leisure", "Video Rental");
        m_dbmodel->Add_Trans_Split(split_id, 20, "Miscellaneous");
        m_dbmodel->Add_Trans_Split(split_id, 40, "Healthcare", "Health");

        trans_date = starting_date;
        // Quarterley Transactions March, June, September, December
        if ((trans_date.GetMonth() == wxDateTime::Mar) || (trans_date.GetMonth() == wxDateTime::Jun) || (trans_date.GetMonth() == wxDateTime::Sep) || (trans_date.GetMonth() == wxDateTime::Dec))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Government Authorities", 250, "Home", "Water Rates");
        }

        trans_date = starting_date;
        // Yearly Transactions
        if ((trans_date.GetMonth() == wxDateTime::Jun))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 400, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 700, "Home", "Gas");
        }

        trans_date = starting_date;
        // Yearly Transactions
        if ((trans_date.GetMonth() == wxDateTime::Dec))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 200, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 50, "Home", "Gas");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Bank - ANZ", 80, "Mastercard", "Annual Fee");
        }

        trans_date = starting_date;
        // Yearly Transactions - August 
        if ((trans_date.GetMonth() == wxDateTime::Aug))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(12)), "Government Authorities", 2200, "Home", "General Rates");
        }

        trans_date = starting_date;
        // Six Monthly Transactions
        if ((trans_date.GetMonth() == wxDateTime::Mar) || (trans_date.GetMonth() == wxDateTime::Sep))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 300, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 400, "Home", "Gas");

            m_dbmodel->Add_Trans_Deposit("ACME Corporation Shares", trans_date.Add(wxDateSpan::Days(7)), "ACME Corporation", 25.75, "Income", "Share Dividend");
            m_dbmodel->Add_Trans_Transfer("ACME Corporation Shares", trans_date, "ANZ - Cash Manager", 25.75, "Transfer", "Share Dividend");
            m_dbmodel->Add_Trans_Deposit("ABC Corporation Shares", trans_date.Add(wxDateSpan::Days(7)), "ABC Corporation", 555.25, "Income", "Share Dividend");
            m_dbmodel->Add_Trans_Transfer("ABC Corporation Shares", trans_date, "ANZ - Cash Manager", 555.25, "Transfer", "Share Dividend");
        }
        //--------------------------------------------------------------------

        // End of Month Transactions
        trans_date = starting_date;
        trans_date.SetToLastMonthDay();

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Mastercard", 625.0, "Mastercard", "Repayment");
        m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 150, "Home", "Phone/Internet");

        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Bank - ANZ", 180, "Personal Loan", "Repayments");
        m_dbmodel->Add_Trans_Deposit("Loan: Personal ANZ", trans_date, "Bank - ANZ", 180, "Personal Loan", "Offset");
        m_dbmodel->Add_Trans_Withdrawal("Loan: Personal ANZ", trans_date, "Bank - ANZ", ((10000 - (180 * month_count)) * 0.20) / 12, "Personal Loan", "Interest");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Cheque", 100, "Transfer", "Saving");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Cash Manager", 150, "Transfer", "Saving");
        m_dbmodel->Add_Trans_Deposit("ANZ - Cash Manager", trans_date, "Bank - ANZ", (250 * month_count * .05)/12, "Income", "Bank Interest");
        // -------------------------------------------------------------------

        // Set start of next month transactions
        starting_date.Add(wxDateSpan::Month());
    }

    //------------------------------------------------------------------------ 
    m_test_db.Commit(); // Finalise the database entries.
}

void Test_DatabaseInitialisation::Encrypt_Database_with_Password_test_db()
{
    wxString encryption_password = "test_db";
    wxString target_encrypted_filename = "test_mmex_db_encrypted_" + wxDateTime(wxDateTime::Today()).FormatISODate() + ".emb";

    // Backup the existing database - with encryption.
    if (m_test_db.IsOpen())
    {
        if (wxFileExists(target_encrypted_filename))
        {
            wxRemoveFile(m_test_db_filename);
        }
        m_test_db.Backup(target_encrypted_filename, encryption_password);
    }
    else
    {
        CPPUNIT_FAIL("Test Database is not open to back it up.");
    }

    bool file_ok = false;
    wxSQLite3Database protected_test_db;
    if (!protected_test_db.IsOpen())
    {
        // Open database using correct password.
        protected_test_db.Open(target_encrypted_filename, encryption_password);
        if (protected_test_db.IsOpen() && protected_test_db.IsEncrypted())
        {
            try // access to the database
            {
                protected_test_db.TableExists("INFOTABLE_V1");
                protected_test_db.ExecuteQuery("select * from infotable_v1;");
                file_ok = true;
            }
            catch (...)
            {
                file_ok = false;
            }
        }
        protected_test_db.Close();
    }
    CPPUNIT_ASSERT(file_ok == true);

    file_ok = true;
    if (!protected_test_db.IsOpen())
    {
        // Open database using no password should fail.
        protected_test_db.Open(target_encrypted_filename);
        if (protected_test_db.IsOpen() && !protected_test_db.IsEncrypted())
        {
            try // access to the database
            {
                protected_test_db.TableExists("INFOTABLE_V1");
                protected_test_db.ExecuteQuery("select * from infotable_v1;");
                file_ok = true;
            }
            catch (...)
            {
                file_ok = false;
            }
        }
        protected_test_db.Close();
    }
    CPPUNIT_ASSERT(file_ok == false);

    file_ok = true;
    if (!protected_test_db.IsOpen())
    {
        // Open database using wrong password should fail.
        protected_test_db.Open(target_encrypted_filename, "wrong_password");
        if (protected_test_db.IsOpen() && protected_test_db.IsEncrypted())
        {
            try // access to the database.
            {
                protected_test_db.TableExists("INFOTABLE_V1");
                protected_test_db.ExecuteQuery("select * from infotable_v1;");
                file_ok = true;
            }
            catch (...)
            {
                file_ok = false;
            }
        }
        protected_test_db.Close();
    }
    CPPUNIT_ASSERT(file_ok == false);
}
//--------------------------------------------------------------------------
