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
//----------------------------------------------------------------------------
#include "test_billsdeposits.h"
#include "framebase_tests.h"
#include "billsdepositsdialog.h"
#include "model/Model_Payee.h"
#include "model/Model_Setting.h"
#include "model/Model_Category.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_BillsDeposits);

static int instance_count = 0;
//----------------------------------------------------------------------------
Test_BillsDeposits::Test_BillsDeposits()
{
    instance_count++;
    m_test_db_filename = "test_db_model_billsdeposits.mmb";
}

Test_BillsDeposits::~Test_BillsDeposits()
{
    instance_count--;
    if (instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_BillsDeposits::setUp()
{
    frame = new TestFrameBase(instance_count);
    frame->Show(true);
   
    m_test_db.Open(m_test_db_filename);

    // Initialise the required tables
    Model_Infotable::instance(&m_test_db);
    Model_Currency::instance(&m_test_db);
    Model_Account::instance(&m_test_db);
    Model_Billsdeposits::instance(&m_test_db);
    Model_Budgetsplittransaction::instance(&m_test_db);

    // subcategory must be initialized before category
    Model_Subcategory::instance(&m_test_db);
    Model_Category::instance(&m_test_db);
    Model_Payee::instance(&m_test_db);

    Model_Checking::instance(&m_test_db);
    Model_Splittransaction::instance(&m_test_db);

    // For the purpose of this test, we will create the
    // settings table in the main database.
    Model_Setting::instance(&m_test_db);
}

void Test_BillsDeposits::tearDown()
{
    m_test_db.Close();
    delete frame;
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
    //------------------------------------------------------------------------
    // create a new entry using the dialog.
    mmBDDialog* dlg = new mmBDDialog(frame, 0, false, false);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Dialog Canclled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    if (id == wxID_OK)
    {
        wxMessageBox("Dialog Successful", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
}

void Test_BillsDeposits::test_dialog_edit()
{
    if (Model_Billsdeposits::instance().all().size() < 1)
    {
        CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("If cancelled, entry not exist.", CPPUNIT_ASSERT(1 == 1));
    }

    // create a new entry using the dialog.
    mmBDDialog* dlg = new mmBDDialog(frame, 1, true, false);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Dialog Canclled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    if (id == wxID_OK)
    {
        wxMessageBox("Dialog Successful", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }

    //TODO: Test output

}

void Test_BillsDeposits::test_dialog_enter()
{
    if (Model_Billsdeposits::instance().all().size() < 1)
    {
        CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("If cancelled, entry not exist.", CPPUNIT_ASSERT(1 == 1));
    }

    // create a new entry using the dialog.
    mmBDDialog* dlg = new mmBDDialog(frame, 1, true, true);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Dialog Canclled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    if (id == wxID_OK)
    {
        wxMessageBox("Dialog Successful", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }

    //TODO: Test output

}

//--------------------------------------------------------------------------
