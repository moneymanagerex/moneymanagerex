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
    if (wxFileExists(m_test_db_filename))
    {
        wxRemoveFile(m_test_db_filename);
    }
}

Test_DatabaseInitialisation::~Test_DatabaseInitialisation()
{
    s_instance_count--;
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

void Test_DatabaseInitialisation::Financial_Year_Date_Range()
{
    wxDateTime starting_date(wxDateTime::Today());

    // Set date to start of current financial year.
    int month = starting_date.GetMonth();
    if (month > wxDateTime::Jun)
    {
        starting_date.Subtract(wxDateSpan::Months(month - wxDateTime::Jul));
    }
    else starting_date.Subtract(wxDateSpan::Year()).Add(wxDateSpan::Months(wxDateTime::Jul - month));

    // readjust day to beginning of the month
    starting_date.Subtract(wxDateSpan::Days(starting_date.GetDay() - 1));

    // Now for the tests:
    //------------------------------------------------------------------------
    mmCurrentFinancialYear current_financial_year(1, 7);
    CPPUNIT_ASSERT_EQUAL(starting_date.FormatISODate(), current_financial_year.start_date().FormatISODate());

    mmLastFinancialYear last_financial_year(1, 7);
    starting_date.Subtract(wxDateSpan::Year());
    CPPUNIT_ASSERT_EQUAL(starting_date.FormatISODate(), last_financial_year.start_date().FormatISODate());

    mmLastYear last_year;
    starting_date.Add(wxDateSpan::Months(6)).Subtract(wxDateSpan::Year());
    CPPUNIT_ASSERT_EQUAL(starting_date.FormatISODate(), last_year.start_date().FormatISODate());
}

void Test_DatabaseInitialisation::Add_Account_Entries()
{
    CpuTimer Start("Account  Entries");

    // Add accounts
    m_test_db.Begin();
    m_dbmodel->Add_Bank_Account("NAB - Savings", 1000, "Joint - General Account");
    m_dbmodel->Add_Term_Account("ANZ - Cash Manager", 0, "Savings for special ocasions, Higher Interest Account", false);
    m_dbmodel->Add_Term_Account("ANZ - Personal Loan", 0, "$10,000 @ 20% pa", false);

    m_dbmodel->Add_Bank_Account("ANZ - Cheque", 0, "", false);
    m_dbmodel->Add_Term_Account("Home Loan", -500000, "House Mortgage", false);
    
    m_dbmodel->Add_Bank_Account("ANZ - Mastercard", 0, "Credit Card");
    m_dbmodel->Add_Bank_Account("Wallet - Peter", 0, "Cash Money - Daily Expenses");
    m_dbmodel->Add_Bank_Account("Wallet - Mary", 0, "Cash Money - Daily Expenses");

    m_dbmodel->Add_Investment_Account("ABC Corporation", 0, "Shares");
    m_dbmodel->Add_Term_Account("ACME Corporation Shares", 0, "Share Dividends");
    m_dbmodel->Add_Investment_Account("ACME Corporation", 0, "Shares");
    m_dbmodel->Add_Term_Account("ABC Corporation Shares", 0, "Share Dividends");
    m_dbmodel->Add_Term_Account("Insurance Policies");

    m_dbmodel->Add_Investment_Account("Yahoo Finance", 0, "Stocks");

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

void Test_DatabaseInitialisation::Add_Stock_Entries(const wxDateTime& starting_date)
{
    wxDateTime trans_date = starting_date;
    // Using Stocks to handle Shares
    int stock_ABC_Account_id = m_dbmodel->Get_account_id("ABC Corporation");
    int stock_ACME_Account_id = m_dbmodel->Get_account_id("ACME Corporation");

    m_dbmodel->Add_Stock_Entry(stock_ABC_Account_id, trans_date.Add(wxDateSpan::Months(2)), 1000, 4.2575, 0, 0, 0, "ABC Initial Share Purchase", "AMP.ax");
    m_dbmodel->Add_Stock_Entry(stock_ACME_Account_id, trans_date, 1000, 4.2575, 0, 0, 0, "ACME Initial Share Purchase", "AMP.ax");

    m_dbmodel->Add_Stock_Entry(stock_ABC_Account_id, trans_date.Add(wxDateSpan::Months(6)), 9550, 5.2575, 0, 0, 0, "ABC Suplement Purchase", "AMP.ax");
    m_dbmodel->Add_Stock_Entry(stock_ACME_Account_id, trans_date, 9550, 5.2575, 0, 0, 0, "ACME Suplement Purchase", "AMP.ax");

    m_dbmodel->Add_Stock_Entry(stock_ABC_Account_id, trans_date.Add(wxDateSpan::Months(6)), 5, 5.2775, 0, 0, 0, "DRP", "AMP.ax");
    m_dbmodel->Add_Stock_Entry(stock_ACME_Account_id, trans_date, 5, 5.2775, 0, 0, 0, "DRP", "AMP.ax");

    m_dbmodel->Add_Stock_Entry(stock_ABC_Account_id, trans_date.Add(wxDateSpan::Months(6)), 10, 6.1575, 0, 0, 0, "DRP ", "amp.ax");
    m_dbmodel->Add_Stock_Entry(stock_ACME_Account_id, trans_date, 10, 6.1575, 0, 0, 0, "DRP ", "amp.ax");

    m_dbmodel->Add_Stock_Entry(stock_ABC_Account_id, trans_date.Add(wxDateSpan::Months(6)), 100, 5.4575, 0, 0, 0, "DRP", "AMP.AX");
    m_dbmodel->Add_Stock_Entry(stock_ACME_Account_id, trans_date, 100, 5.4575, 0, 0, 0, "DRP", "AMP.AX");

    m_dbmodel->Add_Stock_Entry(stock_ABC_Account_id, trans_date.Add(wxDateSpan::Months(6)), 1000, 4.2775, 0, 0, 0, "DRP", "AMP.ax");
    m_dbmodel->Add_Stock_Entry(stock_ACME_Account_id, trans_date, 1000, 4.2775, 0, 0, 0, "DRP", "AMP.ax");

    // Setting up a stock portfolio
    int stock_Yahoo_Finance_id = m_dbmodel->Get_account_id("Yahoo Finance");

    // Setting up history for Telstra at start of a financial year
    trans_date = starting_date;
    int stock_entry_id = m_dbmodel->Add_Stock_Entry(stock_Yahoo_Finance_id, trans_date, 1000, 4.25, 0, 4.25, 0, "Telstra", "tls.ax");
    trans_date.Add(wxDateSpan::Days(10));

    double share_value = 4.75;
    double share_dif = 0.25;
    int share_cycle = 0;
    for (int i = 0; trans_date < wxDateTime::Today(); ++i)
    {
        m_dbmodel->Add_StockHistory_Entry(stock_entry_id, trans_date, 1000 * share_value, Model_StockHistory::MANUAL);
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
    trans_date.Add(wxDateSpan::Months(6));
    stock_entry_id = m_dbmodel->Add_Stock_Entry(stock_Yahoo_Finance_id, trans_date, 1000, 2.25, 0, 2.25, 0, "AMP", "amp.ax");
    trans_date.Add(wxDateSpan::Days(15));

    share_value = 2.75;
    share_dif = 0.25;
    share_cycle = 0;
    for (int i = 0; trans_date < wxDateTime::Today(); ++i)
    {
        m_dbmodel->Add_StockHistory_Entry(stock_entry_id, trans_date, 1000 * share_value, Model_StockHistory::MANUAL);
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
    stock_entry_id = m_dbmodel->Add_Stock_Entry(stock_Yahoo_Finance_id, trans_date, 1000, 3.25, 0, 3.25, 0, "IAG", "iag.ax");
    trans_date = wxDateTime::Today().Subtract(wxDateSpan::Days(90));

    share_value = 3.75;
    share_dif = 0.25;
    share_cycle = 0;
    for (int i = 0; i < 45; ++i)
    {
        m_dbmodel->Add_StockHistory_Entry(stock_entry_id, trans_date, 1000 * share_value, Model_StockHistory::ONLINE);
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
    // Set date 3 years ago from today.
    wxDateTime starting_date(wxDateTime::Today().Subtract(wxDateSpan::Years(3)));

    // Advance or retard the date to the beginning of that financial year.
    int month = starting_date.GetMonth();
    if (month > wxDateTime::Jun)
    {
        starting_date.Subtract(wxDateSpan::Months(month - wxDateTime::Jul));
    }
    else starting_date.Subtract(wxDateSpan::Year()).Add(wxDateSpan::Months(wxDateTime::Jul - month));

    // readjust day to beginning of the month
    starting_date.Subtract(wxDateSpan::Days(starting_date.GetDay() - 1));

    wxDateTime trans_date = starting_date;
    m_test_db.Begin();  // Set all data to memory first, then save to database at end.

    // Setting up a personal loan
    int personal_loan_id = m_dbmodel->Add_Trans_Transfer("ANZ - Personal Loan", trans_date, "ANZ - Cheque", 10000, "Transfer", "Bank Loan");
    Model_Checking::Data* personal_loan = Model_Checking::instance().get(personal_loan_id);
    personal_loan->NOTES = "Initialise $10,000 Personal loan from ANZ -Bank";
    Model_Checking::instance().save(personal_loan);

    // Setting up all entries for Stocks
    Add_Stock_Entries(starting_date);

    // Create transactions for a single month. These are repeated untill current month.
    int month_count = 0;
    bool display_month_totals = true;
    while (starting_date < wxDateTime::Today())
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
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Bank - NAB", 3500, "Home", "Loan Repayments");
        m_dbmodel->Add_Trans_Deposit("Home Loan", trans_date, "Bank - NAB", 3500, "Home", "Loan Offset");
        m_dbmodel->Add_Trans_Withdrawal("Home Loan", trans_date, "Bank - NAB", 500, "Home", "Loan Interest");

        m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 150, "Home", "Phone/Internet");

        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Bank - ANZ", 180, "Personal Loan", "Repayments");
        m_dbmodel->Add_Trans_Deposit("ANZ - Personal Loan", trans_date, "Bank - ANZ", 180, "Personal Loan", "Offset");
        m_dbmodel->Add_Trans_Withdrawal("ANZ - Personal Loan", trans_date, "Bank - ANZ", ((10000 - (180 * month_count)) * 0.20) / 12 , "Personal Loan", "Interest");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Cheque", 100, "Transfer", "Saving");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Cash Manager", 150, "Transfer", "Saving");
        m_dbmodel->Add_Trans_Deposit("ANZ - Cash Manager", trans_date, "Bank - ANZ", (250 * month_count * .05)/12, "Income", "Bank Interest");


        //--------------------------------------------------------------------
        if (display_month_totals)
        {
            m_dbmodel->Current_Payee_Income_Stats("---------- Payee Income: First Month Totals ----------", starting_date);
            m_dbmodel->Current_Payee_Expense_Stats("---------- Payee Expense: First Month Totals ----------", starting_date);

            m_dbmodel->Current_Category_Income_Stats("---------- Category Income: First Month Totals ----------", starting_date);
            m_dbmodel->Current_Category_Expense_Stats("---------- Category Expense: First Month Totals ----------", starting_date);
            
            m_dbmodel->Current_Subcategory_Income_Stats("---------- Subcategory Income: First Month Totals ----------", starting_date);
            m_dbmodel->Current_Subcategory_Expense_Stats("---------- Subcategory Expense: First Month Totals ----------", starting_date);

            //Collect Totals Data.
            m_dbmodel->Total_Payee_Stats(starting_date);
            m_dbmodel->Total_Category_Stats(starting_date);
            m_dbmodel->Total_Subcategory_Stats(starting_date);

            display_month_totals = false;
        }

        // -------------------------------------------------------------------

        // Set start of next month transactions
        starting_date.Add(wxDateSpan::Month());
    }

    // -----------------------------------------------------------------------
    // Report Totals for payees and categories
    m_dbmodel->Current_Payee_Income_Stats("---------- Payee Income: All Time Totals ----------", starting_date);
    m_dbmodel->Current_Payee_Expense_Stats("---------- Payee Expense: All Time Totals ----------", starting_date);

    m_dbmodel->Current_Category_Income_Stats("---------- Category Income: All Time Totals ----------", starting_date);
    m_dbmodel->Current_Category_Income_Stats("---------- Category Income: All Time Totals ----------", starting_date);

    m_dbmodel->Current_Subcategory_Income_Stats("---------- Subcategory Expense: All Time Totals ----------", starting_date);
    m_dbmodel->Current_Subcategory_Income_Stats("---------- Subcategory Expense: All Time Totals ----------", starting_date);

    m_dbmodel->Total_Payee_Stats(starting_date);
    m_dbmodel->Total_Category_Stats(starting_date);
    m_dbmodel->Total_Subcategory_Stats(starting_date);

    //------------------------------------------------------------------------ 
    m_test_db.Commit(); // Finalise the database entries.
}

void Test_DatabaseInitialisation::Add_Repeat_Transaction_Entries()
{
    CpuTimer Start("Repeat Transaction Entries");
    wxDateTime start_date = wxDateTime::Today().Add(wxDateSpan::Day());

    m_test_db.Begin();

    m_dbmodel->Bill_Start("ANZ - Mastercard", start_date, Model_Billsdeposits::REPEAT_WEEKLY);
    m_dbmodel->Bill_Trans_Withdrawal(start_date, "Woolworths", 100);
    int bill_id = m_dbmodel->BILL_End();
   
    m_dbmodel->Bill_Split(bill_id, 10, "Food", "Groceries");
    m_dbmodel->Bill_Split(bill_id, 30, "Food", "Groceries");
    m_dbmodel->Bill_Split(bill_id, 60, "Food", "Groceries");

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Database_Encryption_Password_test_db()
{
    wxString encryption_password = "test_db";
    wxString target_encrypted_filename = "test_mmex_db_encrypted.emb";

    // Backup the existing database - with encryption.
    if (m_test_db.IsOpen())
    {
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
