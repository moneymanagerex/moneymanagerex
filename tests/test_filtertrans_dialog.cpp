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
#include "test_filtertrans_dialog.h"
#include "filtertransdialog.h"
//----------------------------------------------------------------------------
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_Billsdeposits.h"
#include "option.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_FILTER_TRANS_DIALOG
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Test_FilterTrans_Dialog );
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_FilterTrans_Dialog::Test_FilterTrans_Dialog()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_filtertrans.mmb";
}

Test_FilterTrans_Dialog::~Test_FilterTrans_Dialog()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_FilterTrans_Dialog::setUp()
{
    CpuTimer time("Startup");
    m_base_frame = new TestFrameBase(m_this_instance);
    m_base_frame->Show(true);

    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();

    // set up entries in the database
    Model_Checking::instance().Savepoint();
    {
        CpuTimer Start("Entries");
        // Add some payees.
        m_dbmodel->Add_Payee("Supermarket");
        m_dbmodel->Add_Payee("Aldi");
        m_dbmodel->Add_Payee("Coles");
        m_dbmodel->Add_Payee("Woolworths");

        // Add some accounts
        m_dbmodel->Add_Bank_Account("Savings");
        m_dbmodel->Add_Bank_Account("Cheque");
        m_dbmodel->Add_Bank_Account("Mastercard");

        wxDateTime trans_date = wxDateTime::Today();

        // Put some data in each account
        m_dbmodel->Add_Trans_Deposit("Cheque", trans_date, "Aldi", 100.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("Cheque", trans_date.Subtract(wxDateSpan::Days(3)), "Coles", 20.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Transfer("Cheque", trans_date.Subtract(wxDateSpan::Days(5)), "Mastercard", 30.0, "Gifts", "", true, 40.0);

        m_dbmodel->Add_Trans_Deposit("Savings", trans_date.Subtract(wxDateSpan::Weeks(1)), "Aldi", 200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date.Subtract(wxDateSpan::Weeks(2)), "Coles", 20.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Transfer("Savings", trans_date.Subtract(wxDateSpan::Weeks(3)), "Mastercard", 30.0, "Gifts", "", true, 40.0);

        m_dbmodel->Add_Trans_Deposit("Mastercard", trans_date.Subtract(wxDateSpan::Months(1)).Subtract(wxDateSpan::Weeks(1)), "Aldi", 300.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("Mastercard", trans_date.Subtract(wxDateSpan::Months(1)).Subtract(wxDateSpan::Weeks(2)), "Coles", 20.0, "Food", "Groceries");
        // Sddition of split transactions.
        int trans_id = m_dbmodel->Add_Trans_Withdrawal("Mastercard", trans_date.Subtract(wxDateSpan::Months(1)).Subtract(wxDateSpan::Weeks(2)), "Woolworths", 0.0);
        m_dbmodel->Add_Trans_Split(trans_id, 300, "Healthcare", "Dental");
        m_dbmodel->Add_Trans_Split(trans_id, 400, "Healthcare", "Eyecare");
        m_dbmodel->Add_Trans_Split(trans_id, 100, "Healthcare", "Prescriptions");

        trans_id = m_dbmodel->Add_Trans_Withdrawal("Mastercard", trans_date.Subtract(wxDateSpan::Months(1)).Subtract(wxDateSpan::Weeks(2)), "Coles", 20.0);
        m_dbmodel->Add_Trans_Split(trans_id, 150, "Food", "Groceries");
        m_dbmodel->Add_Trans_Split(trans_id, 400, "Homeneeds", "Others");
    }
    Model_Checking::instance().ReleaseSavepoint();
}

void Test_FilterTrans_Dialog::tearDown()
{
    delete m_dbmodel;
    m_test_db.Close();
    delete m_base_frame;
}

void Test_FilterTrans_Dialog::ShowMessage(wxString msg)
{
    msg = msg << "\nInstance # " << m_this_instance;
    wxMessageBox(msg, "Test: FilterTrans Dialog", wxOK, wxTheApp->GetTopWindow());
}

void Test_FilterTrans_Dialog::test_mmFilterTransactionsDialog()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    // Create it here because we want it to remember last actions.
    mmFilterTransactionsDialog test_dialog(m_base_frame);

    bool testing_dialog = true;
    int test_count = 0;
    while (testing_dialog && test_count <= 2)
    {
        test_count++;
        switch (test_count)
        {
        case 1:
            user_request->Show_InfoBarMessage("Please set Account as 'Mastercard'\n\n Cancel to bypass this test.");
            if (test_dialog.ShowModal() == wxID_OK)
            {
                int account_id = -1;
                if (test_dialog.getAccountCheckBox())
                {
                    account_id = test_dialog.getAccountID();
                }
                CPPUNIT_ASSERT(account_id == 3);
            }
            else testing_dialog = false;

            break;

        case 2:
            user_request->Show_InfoBarMessage("Please set payee to 'Aldi'\n\n Cancel to bypass this test.");
            if (test_dialog.ShowModal() == wxID_OK)
            {
                //                dlg.checkAll()


                CPPUNIT_ASSERT(test_dialog.somethingSelected());
            }
            else testing_dialog = false;

            break;

        default:
            user_request->Show_InfoBarMessage("Test Completed\n\n Refer back to console.");
            testing_dialog = false;
        }
    }
}
