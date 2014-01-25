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
#include <wx/frame.h>
#include "transdialog.h"

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

    m_dbmodel = new DB_Init_Model();
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
    m_dbmodel->Add_Bank_Account("Savings", 1000, "Joint - General account");
    m_dbmodel->Add_Term_Account("Investment Savings", 0, "Savings for special ocasions", false);
    
    m_dbmodel->Add_Bank_Account("Cheque", 0, "", false);
    m_dbmodel->Add_Term_Account("Home Loan", -500000, "", false);
    
    m_dbmodel->Add_Bank_Account("Mastercard", 0, "Credit Card");
    m_dbmodel->Add_Bank_Account("Wallet - Peter", 0, "Cash Money - Daily Expenses");
    m_dbmodel->Add_Bank_Account("Wallet - Mary", 0, "Cash Money - Daily Expenses");

    m_dbmodel->Add_Investment_Account("ABC Corporation", 0, "Shares");
    m_dbmodel->Add_Investment_Account("Acme Corporation", 0, "Shares");
    m_dbmodel->Add_Term_Account("Property Management");
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
    m_dbmodel->Add_Payee("Work: Peter", "Income", "Salary");
    m_dbmodel->Add_Payee("Work: Mary", "Income", "Salary");
    m_dbmodel->Add_Payee("Bank: NAB");
    m_dbmodel->Add_Payee("Bank: ANZ");
    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Category_Entries()
{
    CpuTimer Start("Category Entries");
    m_test_db.Begin();

    // Category structure: "Family Home"
    int cat_id = m_dbmodel->Add_Category("Family Home");
    m_dbmodel->Add_Subcategory(cat_id, "General Rates");
    m_dbmodel->Add_Subcategory(cat_id, "Water Rates");
    m_dbmodel->Add_Subcategory(cat_id, "Gas");
    m_dbmodel->Add_Subcategory(cat_id, "Electricity");
    m_dbmodel->Add_Subcategory(cat_id, "Phone/Internet");
    m_dbmodel->Add_Subcategory(cat_id, "Home Insurance");

    // Category structure: "Mastercard"
    cat_id = m_dbmodel->Add_Category("Mastercard");
    m_dbmodel->Add_Subcategory(cat_id, "Repayment");
    m_dbmodel->Add_Subcategory(cat_id, "Annual Fee");

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Transaction_Entries()
{
    CpuTimer Start("Transaction Entries");

    wxDateTime starting_date(wxDateTime::Today().Subtract(wxDateSpan::Years(3)));

    // Set date to start of financial year.
    int month = starting_date.GetMonth() - 6;
    if (month < 0)
    {
        starting_date.Subtract(wxDateSpan::Months(month));
    }
    else starting_date.Add(wxDateSpan::Months(month));
    starting_date.Subtract(wxDateSpan::Days(starting_date.GetDay() - 1));

    double payee_work_peter = 0;
    double payee_work_mary = 0;

    double payee_Aldi = 0;
    double payee_Coles = 0;
    double payee_woolworths = 0;
    double payee_supermarket = 0;

    double cat_income_salary = 0;
    double cat_mastercard_repayment = 0;
    double cat_food_groceries = 0;

    wxDateTime trans_date = starting_date;

    // Create transactions for a single month. These are repeated for till present day.
    m_test_db.Begin();
    while (starting_date < wxDateTime::Today())
    {
        //wxLogDebug(starting_date.FormatISODate() + " trans: " + trans_date.FormatISODate());
        trans_date = starting_date;
        m_dbmodel->Add_Trans_Deposit("Savings", trans_date.Add(wxDateSpan::Days(8)), "Work: Peter", 1200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Deposit("Savings", trans_date.Add(wxDateSpan::Days(7)), "Work: Peter", 1500.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Deposit("Savings", trans_date.Add(wxDateSpan::Days(7)), "Work: Peter", 1200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Deposit("Savings", trans_date.Add(wxDateSpan::Days(7)), "Work: Peter", 1200.0, "Income", "Salary");
        int payee_work_peter_month = 5100;
        int cat_income_salary_month = 5100;

        trans_date = starting_date;
        m_dbmodel->Add_Trans_Deposit("Savings", trans_date.Add(wxDateSpan::Days(14)), "Work: Mary", 1600.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Deposit("Savings", trans_date.Add(wxDateSpan::Days(14)), "Work: Mary", 1700.0, "Income", "Salary");
        int payee_work_mary_month = 3300;
        cat_income_salary_month += 3300;

        trans_date = starting_date;
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(1)), "Aldi", 50, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 30, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(3)), "Woolworths", 40, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 120, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 20.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(3)), "Aldi", 40.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 60.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 80.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(2)), "Woolworths", 20.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(3)), "Coles", 60.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 40.0, "Food", "Groceries");
        int payee_Aldi_month = 90;
        int payee_Coles_month = 170;
        int payee_supermarket_month = 240;
        int payee_woolworths_month = 60;
        int cat_food_groceries_month = 560;

        trans_date = starting_date;
        m_dbmodel->Add_Trans_Transfer("Savings", trans_date.Add(wxDateSpan::Days(27)), "Mastercard", 30.0, "Repayment", "", true, 40.0);
        int cat_mastercard_repayment_month = 30;

        
        
        
        trans_date = starting_date;
        // Quarterley Transactions
        if ((trans_date.GetMonth() == 3) || (trans_date.GetMonth() == 6) || (trans_date.GetMonth() == 9) || (trans_date.GetMonth() == 12))  // December
        {


        }

        trans_date = starting_date;
        // Yearly Transactions - 1
        if ((trans_date.GetMonth() == 2))
        {


        }

        trans_date = starting_date;
        // Yearly Transactions - 2
        if ((trans_date.GetMonth() == 2))
        {


        }

        trans_date = starting_date;
        // Yearly Transactions - 3
        if ((trans_date.GetMonth() == 2))
        {


        }

        trans_date = starting_date;
        // Six Monthly Transactions - 1
        if ((trans_date.GetMonth() == 3) || (trans_date.GetMonth() == 6))
        {


        }

        trans_date = starting_date;
        // Six Monthly Transactions - 2
        if ((trans_date.GetMonth() == 3) || (trans_date.GetMonth() == 6))
        {


        }

        m_dbmodel->Add_Asset("payee_work_peter", starting_date, payee_work_peter_month);
        m_dbmodel->Add_Asset("payee_work_mary", starting_date, payee_work_mary_month);
        m_dbmodel->Add_Asset("cat_income_salary", starting_date, cat_income_salary_month);

        m_dbmodel->Add_Asset("payee_Aldi", starting_date, payee_Aldi_month);
        m_dbmodel->Add_Asset("payee_Coles", starting_date, payee_Coles_month);
        m_dbmodel->Add_Asset("payee_supermarket", starting_date, payee_supermarket_month);
        m_dbmodel->Add_Asset("payee_woolworths", starting_date, payee_woolworths_month);

        m_dbmodel->Add_Asset("cat_mastercard_repayment", starting_date, cat_mastercard_repayment_month);
        m_dbmodel->Add_Asset("cat_food_groceries", starting_date, cat_food_groceries_month);

        // End of transactions for month---------------------------------------
        starting_date.Add(wxDateSpan::Month());

        payee_work_peter += payee_work_peter_month;
        payee_work_mary += payee_work_mary_month;

        payee_Aldi += payee_Aldi_month;
        payee_Coles += payee_Coles_month;
        payee_woolworths += payee_woolworths_month;
        payee_supermarket += payee_supermarket_month;

        cat_income_salary += cat_income_salary_month;
        cat_mastercard_repayment += cat_mastercard_repayment_month;
        cat_food_groceries += cat_food_groceries_month;
    }

    m_dbmodel->Add_Asset("payee_work_peter", starting_date, payee_work_peter);
    m_dbmodel->Add_Asset("payee_work_mary", starting_date, payee_work_mary);
    m_dbmodel->Add_Asset("cat_income_salary", starting_date, cat_income_salary);

    m_dbmodel->Add_Asset("payee_Aldi", starting_date, payee_Aldi);
    m_dbmodel->Add_Asset("payee_Coles", starting_date, payee_Coles);
    m_dbmodel->Add_Asset("payee_supermarket", starting_date, payee_supermarket);
    m_dbmodel->Add_Asset("payee_woolworths", starting_date, payee_woolworths);

    m_dbmodel->Add_Asset("cat_mastercard_repayment", starting_date, cat_mastercard_repayment);
    m_dbmodel->Add_Asset("cat_food_groceries", starting_date, cat_food_groceries);

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Repeat_Transaction_Entries()
{
    CpuTimer Start("Repeat Transaction Entries");
    wxDateTime start_date = wxDateTime::Today().Add(wxDateSpan::Day());

    m_test_db.Begin();

    m_dbmodel->Bill_Start("Mastercard", start_date, Model_Billsdeposits::REPEAT_WEEKLY);
    m_dbmodel->Bill_Trans_Withdrawal(start_date, "Woolworths", 100);
    int bill_id = m_dbmodel->BILL_End();
   
    m_dbmodel->Add_Bill_Split(bill_id, 10, "Food", "Groceries");
    m_dbmodel->Add_Bill_Split(bill_id, 30, "Food", "Groceries");
    m_dbmodel->Add_Bill_Split(bill_id, 60, "Food", "Groceries");

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
