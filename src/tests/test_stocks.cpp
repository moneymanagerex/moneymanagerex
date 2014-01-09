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
#include "test_stocks.h"
#include "framebase_tests.h"
#include "stockdialog.h"
#include "model/Model_Stock.h"
#include "model/Model_Account.h"
#include "model/Model_Infotable.h"

// Registers the fixture into the 'registry'
//CPPUNIT_TEST_SUITE_REGISTRATION(Test_Stock);

static int instance_count = 0;
//----------------------------------------------------------------------------
Test_Stock::Test_Stock()
{
    instance_count++;
    m_test_db_filename = "test_db_model_stock.mmb";
}

Test_Stock::~Test_Stock()
{
    instance_count--;
    if (instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Stock::setUp()
{
    frame = new TestFrameBase(instance_count);
    frame->Show(true);
   
    m_test_db.Open(m_test_db_filename);

    // Initialise the required tables
    Model_Infotable::instance(&m_test_db);
    Model_Currency::instance(&m_test_db);
    Model_Account::instance(&m_test_db);
    Model_Stock::instance(&m_test_db);
}

void Test_Stock::tearDown()
{
    m_test_db.Close();
    delete frame;
}

void Test_Stock::test_dialog_add()
{
    // Set the base currency
    Model_Currency::Data base_currency = Model_Currency::instance().GetCurrencyRecord("AUD");
    Model_Currency::instance().SetBaseCurrency(&base_currency);

    // initialise an account name in the account table
    Model_Account::Data* account = Model_Account::instance().create();
    account->ACCOUNTNAME = "ACME Corp";
    account->ACCOUNTTYPE = Model_Account::instance().all_type()[Model_Account::INVESTMENT];
    account->STATUS = Model_Account::instance().all_status()[Model_Account::OPEN];
    account->CURRENCYID = base_currency.id();
    Model_Account::instance().save(account);

    int account_id = Model_Account::instance().get("ACME Corp")->id();

    // create a new entry using the dialog.
    mmStockDialog* dlg = new mmStockDialog(frame, 0, account_id);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Stock Dialog Canclled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    if (id == wxID_OK)
    {
        wxMessageBox("Stock Dialog Successful", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
}

void Test_Stock::test_dialog_edit()
{
    Model_Stock stock = Model_Stock::instance();
    Model_Stock::Data* my_entry = stock.get(1);
    CPPUNIT_ASSERT(my_entry);

    double commission = my_entry->COMMISSION;
    double current_price = my_entry->CURRENTPRICE;
    double num_shares = my_entry->NUMSHARES;
    double purchase_price = my_entry->PURCHASEPRICE;

    // create a new entry using the dialog.
    mmStockDialog* dlg = new mmStockDialog(frame, my_entry, my_entry->HELDAT);
    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Stock Dialog Canclled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    if (id == wxID_OK)
    {
        Model_Stock::Data* my_new_entry = stock.get(1);

        CPPUNIT_ASSERT(my_new_entry->COMMISSION == commission);
        CPPUNIT_ASSERT(my_new_entry->CURRENTPRICE == current_price);
        CPPUNIT_ASSERT(my_new_entry->NUMSHARES == num_shares);
        CPPUNIT_ASSERT(my_new_entry->PURCHASEPRICE == purchase_price);
        wxMessageBox("Stock Dialog Successful", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
}
//--------------------------------------------------------------------------
