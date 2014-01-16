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
#include "mmOption.h"

// Registers the fixture into the 'registry'
//CPPUNIT_TEST_SUITE_REGISTRATION( Test_FilterTrans_Dialog );

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
    m_frame = new TestFrameBase(m_this_instance);
    m_frame->Show(true);
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_FilterTrans_Dialog::tearDown()
{
    m_test_db.Close();
    delete m_frame;
    delete m_dbmodel;
}

void Test_FilterTrans_Dialog::ShowMessage(wxString msg)
{
    msg = msg << "\nInstance # " << m_this_instance;
    wxMessageBox(msg, "Test: FilterTrans Dialog", wxOK, wxTheApp->GetTopWindow());
}

void Test_FilterTrans_Dialog::test_dialog()
{
    CpuTimer Start("Entries");

    Model_Checking::instance().Begin();
    {
        // Add some payees.
        m_dbmodel->Add_Payee("Supermarket");
        m_dbmodel->Add_Payee("Aldi");
        m_dbmodel->Add_Payee("Coles");
        m_dbmodel->Add_Payee("Woolworths");

        // Add some accounts
        m_dbmodel->Add_Account("Savings", Model_Account::TYPE::CHECKING);
        m_dbmodel->Add_Account("Cheque", Model_Account::TYPE::CHECKING);
        m_dbmodel->Add_Account("Mastercard", Model_Account::TYPE::CHECKING);

        // Put some data in each account
        m_dbmodel->Set_AccountName("Cheque");
        m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 100.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);

        m_dbmodel->Set_AccountName("Savings");
        m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);

        m_dbmodel->Set_AccountName("Mastercard");
        m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 300.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
        m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
    }
    Model_Checking::instance().Commit();

    //ShowMessage("Please relocate Insurance/Auto to Automobile/Registration\n\nThis should result in 6 records being changed.\n");
    mmFilterTransactionsDialog dlg(m_frame);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msg = "Test Complete: No data checking performed.\n\nStill under construction.\n";
        //msg << dlg.updatedCategoriesCount() << " records changed.";
        ShowMessage(msg);
    }
}
