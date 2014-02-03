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
#include "test_checking.h"
#include "transdialog.h"
#include "mmcheckingpanel.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef __MMEX_TESTS__CHECKING
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Checking);
#endif

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
    if (m_this_instance > 3)
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

    if (m_this_instance > 3)
    {
        delete m_base_frame;
    }
}

void Test_Checking::Set_UP_Database_conditions()
{
    CpuTimer Start("Set Up Database Conditions");
    m_test_db.Begin();
    // Add Bank Accounts
    m_dbmodel->Add_Bank_Account("Savings");
    m_dbmodel->Add_Bank_Account("Cheque");
    m_dbmodel->Add_Bank_Account("Mastercard");

    // Add Term Accounts
    m_dbmodel->Add_Term_Account("Home Loan");
    m_dbmodel->Add_Term_Account("Savings Investment");
    m_dbmodel->Add_Term_Account("Property Management");

    // Add Payees
    m_dbmodel->Add_Payee("Supermarket");
    m_dbmodel->Add_Payee("Aldi");
    m_dbmodel->Add_Payee("Coles");
    m_dbmodel->Add_Payee("Woolworths");
    m_dbmodel->Add_Payee("Work - Salary");
    m_dbmodel->Add_Payee("Investment Property");

    // Add extra categories
    int cat_id = m_dbmodel->Add_Category("Mastercard");
    m_dbmodel->Add_Subcategory(cat_id,"Repayment");

    m_test_db.Commit();
}

void Test_Checking::Add_Transactions()
{
    CpuTimer Start("Add_Transactions");
    m_test_db.Begin();
    wxDateTime start_date(wxDateTime::Today().Subtract(wxDateSpan::Month()));
    m_dbmodel->Add_Trans_Deposit("Savings", start_date, "Aldi", 200.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal("Savings", start_date, "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer("Savings", start_date, "Savings", 30.0, "Gifts", "", true, 40.0);

    m_dbmodel->Add_Trans_Deposit("Mastercard", start_date, "Aldi", 300.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal("Mastercard", start_date, "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer("Mastercard", start_date, "Savings", 30.0, "Gifts", "", true, 40.0);
    m_test_db.Commit();
}

void Test_Checking::Add_Split_Transactions()
{
    CpuTimer Start("Add_Split_Transactions");
//    m_test_db.Begin();

//    m_test_db.Commit();
}

// On closing, the test will crash.
void Test_Checking::Test_Account_View()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    // Create a new frame anchored to the base frame.
    TestFrameBase* account_frame = new TestFrameBase(m_base_frame, 670, 400);
    account_frame->Show();

    int account_id = m_dbmodel->Get_account_id("Savings");

    // Create the panel under test
    mmCheckingPanel* account_panel = new mmCheckingPanel(account_id, account_frame);
    account_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Account Panel being displayed.\n\nContinue other tests ...",
        "Testing Account: Savings", wxOK, wxTheApp->GetTopWindow());
}

void Test_Checking::Test_Transaction_Dialog_Messages()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    int account_id = m_dbmodel->Get_account_id("Savings");
    bool testing = true;
    int test_count = 0;
    while (testing)
    {
        test_count++;
        switch (test_count)
        {
            case 1:
            {
                user_request->Show_InfoBarMessage(
                    "1. Use the OK button to observe error messages.\n"
                    "2. Add appropriate details as required.\n\n"
                    "Use Cancel to exit dialog.");
                mmTransDialog test_dialog(m_base_frame, account_id, 0);
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    testing = false;
                }

                break;
            }

            case 2:
            {
                user_request->Show_InfoBarMessage(
                    "Set the Type to: Transfer\n\n"
                    "1. Use the OK button to observe error messages.\n"
                    "2. Add appropriate details as required.\n\n"
                    "Use Cancel to exit dialog.");
                mmTransDialog test_dialog(m_base_frame, account_id, 0);
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    testing = false;
                }
                break;
            }
            default:
            {
                Model_Checking::Data_Set table = Model_Checking::instance().all();
                Model_Checking::Data trans_entry = table.at(table.size() - 1);

                user_request->Show_InfoBarMessage("This Test Completed\n\n Use Cancel to proceed to next test.");
                mmTransDialog test_dialog(m_base_frame, account_id, trans_entry.id());
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    testing = false;
                }
            }
        }
    }
}

void Test_Checking::Test_Transaction_New_Edit()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    int account_id = m_dbmodel->Get_account_id("Savings");
    bool testing = true;
    int test_count = 0;
    while (testing)
    {
        test_count++;
        switch (test_count)
        {
            case 1:
            {
                user_request->Show_InfoBarMessage(
                      "Create a new 'Transfer' transaction.: \n\n"
                      "1. Set the category to Mastercard/Repayment\n"
                      "   This transaction will be used for editing.\n"
                      "2. Add appropriate details as required.\n\n"
                      "Use Cancel to exit dialog.\n");
                mmTransDialog test_dialog(m_base_frame, account_id, 0);
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    user_request->Show_InfoBarMessage("Test Completed\n\n Refer back to console.");
                    testing = false;
                }
                break;
            }
            default:
            {
                Model_Checking::Data_Set table = Model_Checking::instance().all();
                Model_Checking::Data trans_entry = table.at(table.size() - 1);
                user_request->Show_InfoBarMessage(
                    "Edit the 'Transfer' transaction.: \n\n"
                    "1. Check that the category is Mastercard/Repayment\n"
                    "2. Add appropriate details as required.\n\n"
                    "Use Cancel to exit dialog.\n");
                mmTransDialog test_dialog(m_base_frame, account_id, trans_entry.id());
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    user_request->Show_InfoBarMessage("Test Completed\n\n Refer back to console.");
                    testing = false;
                }
            }
        }
    }
}
//--------------------------------------------------------------------------
