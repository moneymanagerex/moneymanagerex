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
#include "test_billsdeposits.h"
#include "billsdepositsdialog.h"
#include "model/Model_Payee.h"
#include "model/Model_Setting.h"
#include "model/Model_Category.h"
#include "mmOption.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_BillsDeposits);

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_BillsDeposits::Test_BillsDeposits()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_model_billsdeposits.mmb";
}

Test_BillsDeposits::~Test_BillsDeposits()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_BillsDeposits::setUp()
{
    CpuTimer time("Setup");
    m_frame = new TestFrameBase(m_this_instance);
    m_frame->Show(true);
   
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
}

void Test_BillsDeposits::tearDown()
{
    m_test_db.Close();
    delete m_frame;
    delete m_dbmodel;
}

void Test_BillsDeposits::ShowMessage(wxString msg)
{
    msg = msg << "   # " << m_this_instance;
    wxMessageBox(msg, "MMEX Bill Deposits Dialog Test", wxOK, wxTheApp->GetTopWindow());
}

void Test_BillsDeposits::test_dialog_add()
{
    // Set the base currency
    Model_Currency::Data base_currency = Model_Currency::instance().GetCurrencyRecord("AUD");
    Model_Currency::instance().SetBaseCurrency(&base_currency);

    // initialise some accounts
    Model_Account::Data* account = Model_Account::instance().create();
    account->ACCOUNTNAME = "Savings";
    account->ACCOUNTTYPE = Model_Account::instance().all_type()[Model_Account/*::TYPE*/::CHECKING];
    account->STATUS = Model_Account::instance().all_status()[Model_Account::OPEN];
    account->CURRENCYID = base_currency.id();
    Model_Account::instance().save(account);

    // create a 2nd account
    account = Model_Account::instance().clone(account);
    account->ACCOUNTNAME = "Cheque";
    Model_Account::instance().save(account);

    // create a 3rd account
    account = Model_Account::instance().clone(account);
    account->ACCOUNTNAME = "Mastercard";
    Model_Account::instance().save(account);
    
    // Initialise some payees
    Model_Payee::Data* payee = Model_Payee::instance().create();
    payee->PAYEENAME = "Workshop";
    Model_Payee::instance().save(payee);

    payee = Model_Payee::instance().clone(payee);
    payee->PAYEENAME = "Supermarket";
    payee->CATEGID = Model_Category::instance().get("Food")->id();
    payee->SUBCATEGID = Model_Subcategory::instance().get("Groceries", payee->CATEGID)->id();
    Model_Payee::instance().save(payee);

    // Perform all tests using same instance.
    //------------------------------------------------------------------------
    // create a new entry using the dialog.
    ShowMessage("New Entry: Perform test using new instance.");
    {
        mmBDDialog* dlg = new mmBDDialog(m_frame, 0, false, false);

        int id = dlg->ShowModal();
        if (id == wxID_CANCEL)
        {
            ShowMessage("New Entry: Cancel");
        }
        if (id == wxID_OK)
        {
            ShowMessage("New Entry: OK");
        }
    }

    //----------------------------------------------------------------------------------------
    // Edit existing entry using the dialog.
    //ShowMessage("Edit Entry: Perform test using same instance.");
    //{
    //    if (Model_Billsdeposits::instance().all().size() < 1)
    //    {
    //        CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("If cancelled, entry not exist.", CPPUNIT_ASSERT(1 == 1));
    //    }

    //    // create a new entry using the dialog.
    //    mmBDDialog* dlg = new mmBDDialog(m_frame, 1, true, false);

    //    int id = dlg->ShowModal();
    //    if (id == wxID_CANCEL)
    //    {
    //        ShowMessage("Edit Entry: Cancel");
    //    }
    //    if (id == wxID_OK)
    //    {
    //        ShowMessage("Edit Entry: OK");
    //    }
    //    //TODO: Test output
    //}

    //----------------------------------------------------------------------------------------
    //// Enter the existing entry using the dialog.
    //ShowMessage("Enter Entry: Perform test using same instance.");
    //{
    //    if (Model_Billsdeposits::instance().all().size() < 1)
    //    {
    //        CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("Cancelled, entry not exist.", CPPUNIT_ASSERT(1 == 1));
    //    }

    //    // create a new entry using the dialog.
    //    mmBDDialog* dlg = new mmBDDialog(m_frame, 1, false, true);

    //    int id = dlg->ShowModal();
    //    if (id == wxID_CANCEL)
    //    {
    //        ShowMessage("Enter Entry: Cancel");
    //    }
    //    if (id == wxID_OK)
    //    {
    //        ShowMessage("Enter Entry: OK");
    //    }
    //    //TODO: Test output
    //}
}

void Test_BillsDeposits::test_dialog_edit()
{
    if (Model_Billsdeposits::instance().all().size() < 1)
    {
        CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("Cancelled, entry not exist.", CPPUNIT_ASSERT(1 == 1));
    }

    ShowMessage("Edit Entry: Perform test using new instance.");
    mmBDDialog* dlg = new mmBDDialog(m_frame, 1, true, false);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        ShowMessage("Edit Entry: Cancel");
    }
    if (id == wxID_OK)
    {
        ShowMessage("Edit Entry: OK");
    }

    //TODO: Test output
}

void Test_BillsDeposits::test_dialog_enter()
{
    if (Model_Billsdeposits::instance().all().size() < 1)
    {
        CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("Cancelled, entry not exist.", CPPUNIT_ASSERT(1 == 1));
    }

    // create a new entry using the dialog.
    ShowMessage("Enter Entry: Perform test using new instance.");
    mmBDDialog* dlg = new mmBDDialog(m_frame, 1, false, true);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        ShowMessage("Enter Entry: Cancel");
    }
    if (id == wxID_OK)
    {
        ShowMessage("Enter Entry: OK");
    }

    //TODO: Test output
}

//--------------------------------------------------------------------------
