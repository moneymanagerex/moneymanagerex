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

#include "defs.h"
#include <cppunit/config/SourcePrefix.h>
#include "cpu_timer.h"
#include "db_init_model.h"
#include "framebase_tests.h"
//----------------------------------------------------------------------------
#include "test_checking.h"
#include <wx/frame.h>
#include "transdialog.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Checking);

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Checking::Test_Checking()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_model_checking.mmb";
}

Test_Checking::~Test_Checking()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Checking::setUp()
{
    CpuTimer time("Startup");
    

    // Only set up the base frame if test requires it.
    // wxWidgets will clean up the test frames on completion.
    if (m_this_instance == 4)
    {
        m_base_frame = new TestFrameBase(m_this_instance);
        m_base_frame->Show(true);
    }

    m_commit_hook = new CommitCallbackHook();
    m_test_db.Open(m_test_db_filename);
    m_test_db.SetCommitHook(m_commit_hook);

    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_Checking::tearDown()
{
    delete m_dbmodel;

    m_test_db.SetCommitHook(0);
    m_test_db.Close();
    delete m_commit_hook;
}

void Test_Checking::add_entries()
{
    CpuTimer Start("Entries");

    // Add accounts
    m_dbmodel->Add_Account("Savings", Model_Account::TYPE::CHECKING);
    m_dbmodel->Add_Account("Cheque", Model_Account::TYPE::CHECKING);
    m_dbmodel->Add_Account("Mastercard", Model_Account::TYPE::CHECKING);

    m_dbmodel->Add_Account("Home Loan", Model_Account::TYPE::TERM);
    m_dbmodel->Add_Account("Savings Investment", Model_Account::TYPE::TERM);
    m_dbmodel->Add_Account("Property Management", Model_Account::TYPE::TERM);

    // Add Payees
    m_dbmodel->Add_Payee("Supermarket");
    m_dbmodel->Add_Payee("Aldi");
    m_dbmodel->Add_Payee("Coles");
    m_dbmodel->Add_Payee("Woolworths");

    // No need to add categories as these are initialised by main database.


    m_dbmodel->Set_AccountName("Cheque");

    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 100.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
}

void Test_Checking::add_entries_savings()
{
    CpuTimer Start("Entries_Savings");

    m_dbmodel->Set_AccountName("Savings");

    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 200.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
}

void Test_Checking::add_entries_mc()
{
    CpuTimer Start("Entries_mc");

    m_dbmodel->Set_AccountName("Mastercard");

    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 300.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
}

void Test_Checking::Test_Transaction_Dialog()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    bool testing = true;
    int test_count = 0;
    while (testing)
    {
        test_count++;
        switch (test_count)
        {
            case 1:
                user_request->Show_InfoBarMessage(
                    "1. Use the OK button to observe error messages.\n"
                    "2. Add appropriate details as required.\n\n"
                    "Use Cancel to exit dialog.");
                break;

            case 2:
                user_request->Show_InfoBarMessage(
                    "Set the Type to: Transfer\n\n"
                    "1. Use the OK button to observe error messages.\n"
                    "2. Add appropriate details as required.\n\n"
                    "Use Cancel to exit dialog.");
                break;

            case 3:
                user_request->Show_InfoBarMessage(
                    "Test Completed.\n\n"
                    "Use Cancel to exit dialog.");
                break;

            default:
                user_request->Show_InfoBarMessage("Use 'Cancel' to exit dialog.");
        }

        mmTransDialog test_dialog(m_base_frame, 1, 0);
        if (wxID_CANCEL == test_dialog.ShowModal())
        {
            user_request->Show_InfoBarMessage("Test Completed\n\n Refer back to console.");
            testing = false;
        }
    }
}

//--------------------------------------------------------------------------
