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
    m_dbmodel->Add_Term_Account("Home Loan", 0, "", false);
    
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

    // Adding a complete structure
    int family_home = m_dbmodel->Add_Category("Family Home");
    m_dbmodel->Add_Subcategory(family_home, "General Rates");
    m_dbmodel->Add_Subcategory(family_home, "Water Rates");
    m_dbmodel->Add_Subcategory(family_home, "Gas");
    m_dbmodel->Add_Subcategory(family_home, "Electricity");
    m_dbmodel->Add_Subcategory(family_home, "Phone/Internet");
    m_dbmodel->Add_Subcategory(family_home, "Home Insurance");

    m_test_db.Commit();
}

void Test_DatabaseInitialisation::Add_Transaction_Entries()
{
    CpuTimer Start("Transaction Entries");

    m_dbmodel->Set_AccountName("Mastercard");

    m_test_db.Begin();
    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 300.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
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
            catch (...) { password_changed = false; }

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
            catch (...) { password_changed = false; }

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
//--------------------------------------------------------------------------
