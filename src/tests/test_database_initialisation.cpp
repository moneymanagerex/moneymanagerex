/*******************************************************
Copyright (C) 2013 James Higley
Copyright (C) 2013 Stefano Giorgio

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

#include "defined_test_selection.h"
#include "defs.h"
#include <cppunit/config/SourcePrefix.h>
#include "cpu_timer.h"
#include "db_init_model.h"
#include "framebase_tests.h"
//----------------------------------------------------------------------------
#include "test_database_initialisation.h"
#include <wx/frame.h>
#include "transdialog.h"

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

void Test_DatabaseInitialisation::add_account_entries()
{
    CpuTimer Start("Account  Entries");

    // Add accounts
    m_test_db.Begin();
    m_dbmodel->Add_Account("Savings", Model_Account::TYPE::CHECKING);
    m_dbmodel->Add_Account("Investment Savings", Model_Account::TYPE::TERM, false);
    
    m_dbmodel->Add_Account("Cheque", Model_Account::TYPE::CHECKING, false);
    m_dbmodel->Add_Account("Home Loan", Model_Account::TYPE::TERM);
    
    m_dbmodel->Add_Account("Mastercard", Model_Account::TYPE::CHECKING);
    m_dbmodel->Add_Account("Wallet - Cash", Model_Account::TYPE::CHECKING);

    m_dbmodel->Add_Account("Property Management", Model_Account::TYPE::TERM);
    m_dbmodel->Add_Account("Insurance Policies", Model_Account::TYPE::TERM);
    m_test_db.Commit();
}

void Test_DatabaseInitialisation::add_payee_entries()
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

void Test_DatabaseInitialisation::add_category_entries()
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

void Test_DatabaseInitialisation::add_transaction_entries()
{
    CpuTimer Start("Transaction Entries");

    m_dbmodel->Set_AccountName("Mastercard");

    m_test_db.Begin();
    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 300.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
    m_test_db.Commit();
}

void Test_DatabaseInitialisation::add_repeat_transaction_entries()
{
    CpuTimer Start("Repeat Transaction Entries");

    m_dbmodel->Set_AccountName("Mastercard");

    m_test_db.Begin();
    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 300.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
    m_test_db.Commit();
}

//--------------------------------------------------------------------------
