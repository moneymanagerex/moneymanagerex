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

#ifdef __MMEX_TESTS__TEST_DATABASE_INITIALISATION
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_DatabaseInitialisation);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_DatabaseInitialisation::Test_DatabaseInitialisation()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "mmex_test_database_" + wxDateTime(wxDateTime::Today()).FormatISODate() + ".mmb";
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
        //wxRemoveFile(m_test_db_filename);
        TestFrameBase base_frame(m_this_instance);
        base_frame.Show();
        wxMessageBox("Database files created for this test have not been deleted."
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

    // Set date to start of financial year.
    int month = starting_date.GetMonth();
    if (month > 6)
    {
        starting_date.Subtract(wxDateSpan::Months(month - 6));
    }
    else starting_date.Subtract(wxDateSpan::Year()).Add(wxDateSpan::Months(6 - month));
    starting_date.Subtract(wxDateSpan::Days(starting_date.GetDay() - 1));

    // Now for the tests:
    //------------------------------------------------------------------------
    mmCurrentFinancialYear current_financial_year(1, 7);
    CPPUNIT_ASSERT_EQUAL(starting_date.FormatISODate(), current_financial_year.start_date().FormatISODate());

    mmLastFinancialYear last_financial_year(1, 7);
    starting_date.Subtract(wxDateSpan::Year());
    CPPUNIT_ASSERT_EQUAL(starting_date.FormatISODate(), last_financial_year.start_date().FormatISODate());

    mmLastYear last_year;
    starting_date.Add(wxDateSpan::Months(6));
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
    m_dbmodel->Add_Investment_Account("Acme Corporation", 0, "Shares");
    m_dbmodel->Add_Term_Account("Insurance Policies");
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

    cat_id = m_dbmodel->Get_category_id("Income");
    m_dbmodel->Add_Subcategory(cat_id, "Tax");
    m_dbmodel->Add_Subcategory(cat_id, "Bank Interest");

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Transaction_Entries()
{
    CpuTimer Start("Transaction Entries");
    // Set date 3 years ago from today.
    wxDateTime starting_date(wxDateTime::Today().Subtract(wxDateSpan::Years(3)));

    // Advance or retard the date to the beginning of that financial year.
    int month = starting_date.GetMonth();
    if (month > 6)
    {
        starting_date.Subtract(wxDateSpan::Months(month - 6));
    }
    else starting_date.Subtract(wxDateSpan::Year()).Add(wxDateSpan::Months(6 - month));
    starting_date.Subtract(wxDateSpan::Days(starting_date.GetDay() - 1));

    wxDateTime trans_date = starting_date;
    m_test_db.Begin();  // Set all data to memory first, then save to database at end.

    // Setting up a personal loan
    int personal_loan_id = m_dbmodel->Add_Trans_Transfer("ANZ - Personal Loan", trans_date, "ANZ - Cheque", 10000, "Transfer");
    Model_Checking::Data* personal_loan = Model_Checking::instance().get(personal_loan_id);
    personal_loan->NOTES = "Initialise $10,000 Personal loan from ANZ -Bank";
    Model_Checking::instance().save(personal_loan);

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
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date.Add(wxDateSpan::Days(1)), "Wallet - Peter", 70, "Transfer");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 80, "Transfer");

        m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(7)), "Cash - Miscellaneous", 70, "Miscellaneous");
        m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 80, "Miscellaneous");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Peter", 70, "Transfer");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 100, "Transfer");

        m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(7)), "Cash - Miscellaneous", 70, "Miscellaneous");
        m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 100, "Miscellaneous");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Peter", 60, "Transfer");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 90, "Transfer");

        // Add split Transactions
        int split_id = m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(8)), "Cash - Miscellaneous", 60);
        m_dbmodel->Add_Trans_Split(split_id, 20, "Leisure", "Magazines");
        m_dbmodel->Add_Trans_Split(split_id, 40, "Miscellaneous");

        split_id = m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 90);
        m_dbmodel->Add_Trans_Split(split_id, 30, "Leisure", "Video Rental");
        m_dbmodel->Add_Trans_Split(split_id, 20, "Miscellaneous");
        m_dbmodel->Add_Trans_Split(split_id, 40, "Healthcare", "Health");

        trans_date = starting_date;
        // Quarterley Transactions
        if ((trans_date.GetMonth() == 3) || (trans_date.GetMonth() == 6) || (trans_date.GetMonth() == 9) || (trans_date.GetMonth() == 12))  // December
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Government Authorities", 250, "Home", "Water Rates");
        }

        trans_date = starting_date;
        // Yearly Transactions - 1
        if ((trans_date.GetMonth() == 6))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 400, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 700, "Home", "Gas");
        }

        trans_date = starting_date;
        // Yearly Transactions - 2
        if ((trans_date.GetMonth() == 12))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 200, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 50, "Home", "Gas");
        }

        trans_date = starting_date;
        // Yearly Transactions - 3
        if ((trans_date.GetMonth() == 8))   // August
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(12)), "Government Authorities", 2200, "Home", "General Rates");
        }

        trans_date = starting_date;
        // Six Monthly Transactions - 1
        if ((trans_date.GetMonth() == 3) || (trans_date.GetMonth() == 9)) // March & September
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 300, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 400, "Home", "Gas");
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

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Cheque", 100, "Transfer");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Cash Manager", 250, "Transfer");
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

void Test_DatabaseInitialisation::Backup_Database()
{
    wxString encryption_password = "test_db";
    wxString target_encrypted_backup_filename = "test_db_encrypted_backup.emb";
    wxString target_backup_filename = "test_db_non-encrypted_backup.mmb";

    // Backup the existing database - with and without encryption.
    m_test_db.Backup(target_encrypted_backup_filename, encryption_password);
    m_test_db.Backup(target_backup_filename);

    bool file_ok = false;
    wxSQLite3Database protected_test_db;
    if (!protected_test_db.IsOpen())
    {
        protected_test_db.Open(target_encrypted_backup_filename, encryption_password);
        if (protected_test_db.IsEncrypted())
        {
            try
            {
                // Test the access to the database
                protected_test_db.ExecuteQuery("select * from infotable_v1;");
                file_ok = true;
            }
            catch (...) { file_ok = false; }
        }
        protected_test_db.Close();
    }
    CPPUNIT_ASSERT(file_ok == true);

    file_ok = false;
    if (!protected_test_db.IsOpen())
    {
        protected_test_db.Open(target_encrypted_backup_filename);
        if (!protected_test_db.IsEncrypted())
        {
            try
            {
                // Test the access to the database
                protected_test_db.ExecuteQuery("select * from infotable_v1;");
                file_ok = true;
            }
            catch (...) { file_ok = false; }
        }
        protected_test_db.Close();
    }
    CPPUNIT_ASSERT(file_ok == false);


    file_ok = false;
    wxSQLite3Database non_protected_test_db;
    if (!non_protected_test_db.IsOpen())
    {
        non_protected_test_db.Open(target_backup_filename);
        if (!protected_test_db.IsEncrypted())
        {
            try
            {
                // Test the access to the database
                non_protected_test_db.ExecuteQuery("select * from infotable_v1;");
                file_ok = true;
            }
            catch (...) { file_ok = false; }
        }
        non_protected_test_db.Close();
    }
    CPPUNIT_ASSERT(file_ok == true);

    file_ok = false;
    if (!non_protected_test_db.IsOpen())
    {
        non_protected_test_db.Open(target_backup_filename, encryption_password);
        if (!protected_test_db.IsEncrypted())
        {
            try
            {
                // Test the access to the database
                non_protected_test_db.ExecuteQuery("select * from infotable_v1;");
                file_ok = true;
            }
            catch (...) { file_ok = false; }
        }
    }
    CPPUNIT_ASSERT(file_ok == false);
}

void Test_DatabaseInitialisation::Change_Database_Password()
{
    wxString encryption_password = "correct_password";
    wxString encrypted_database_filename = "test_correct_encrypted_database.emb";

    // Create a backup of the main database and encrypt it
    // This ensures we have a proper database to work with.
    m_test_db.Backup(encrypted_database_filename, encryption_password);

    // Open the encrypted database.
    wxSQLite3Database protected_test_db;
    if (!protected_test_db.IsOpen())
    {
        // Open the encrypted database
        protected_test_db.Open(encrypted_database_filename, encryption_password);

        if (protected_test_db.IsEncrypted())
        {
            bool password_changed = false;
            
            // Test incorrect password
            // Reopen it with a duplicate.
            wxSQLite3Database protected_test_db_duplicate;
            try
            {
                protected_test_db_duplicate.Open(encrypted_database_filename, "incorrect_password");
                if (protected_test_db_duplicate.IsOpen())
                {
                    protected_test_db_duplicate.ExecuteQuery("select * from infotable_v1;");
                    protected_test_db_duplicate.ReKey("new_password");
                    password_changed = true;
                }
            }
            catch (...)
            { 
                password_changed = false;
            }

            protected_test_db_duplicate.Close();
            CPPUNIT_ASSERT(password_changed == false);

            // Test correct password
            try
            {
                protected_test_db_duplicate.Open(encrypted_database_filename, "correct_password");
                if (protected_test_db_duplicate.IsOpen())
                {
                    protected_test_db_duplicate.ExecuteQuery("select * from infotable_v1;");
                    protected_test_db_duplicate.ReKey("new_password");
                    password_changed = true;
                }
            }
            catch (...)
            {
                password_changed = false;
            }

            protected_test_db_duplicate.Close();
            CPPUNIT_ASSERT(password_changed == true);
        }
        protected_test_db.Close();

        if (!protected_test_db.IsOpen())
        {
            protected_test_db.Open(encrypted_database_filename, "new_password");
            bool password_change_successful;
            try
            {
                protected_test_db.ExecuteQuery("select * from infotable_v1");
                password_change_successful = true;
            }
            catch (...) { password_change_successful = false; }
            CPPUNIT_ASSERT(password_change_successful == true);
        }
    }

    // change the password back, because the file is not deleted at the start of the testing.
    protected_test_db.ReKey(encryption_password);
    protected_test_db.Close();
}

void Test_DatabaseInitialisation::Failed_Password_Change_Attempt()
{
    wxString encryption_password = "test_db";
    wxString encrypted_database_filename = "test_db_encrypted_backup.emb";

    TestFrameBase base_frame(m_this_instance);
    base_frame.Show();
    wxPasswordEntryDialog password_entry(&base_frame, "Please provide a new password:", "Test Password Change: " + encryption_password);

    bool old_password_correct = false;
    if (password_entry.ShowModal() == wxID_OK)
    {
        wxString password_from_user = password_entry.GetValue();
        wxSQLite3Database protected_test_db;
        try
        {
            protected_test_db.Open(encrypted_database_filename, password_from_user);
            protected_test_db.ExecuteQuery("select * from infotable_v1;");  // Will throw exception if opened incorrectly.
            old_password_correct = true;
        }
        catch (...)
        {
            /* Expected errors: Do Nothing */
        }
        protected_test_db.Close();

        if (old_password_correct)
        {
            wxMessageBox("Correct password supplied.\n\nPassword Change: Will be successful", "Test Password Change", wxOK | wxICON_INFORMATION, &base_frame);
        }
        else wxMessageBox("Password Change will cause problems", "Test Password Change", wxOK | wxICON_WARNING, &base_frame);
    }
}
//--------------------------------------------------------------------------
