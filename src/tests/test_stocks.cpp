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
#include "db_init_model.h"
#include "framebase_tests.h"
//----------------------------------------------------------------------------
#include "test_stocks.h"
#include "stockdialog.h"
#include "model/Model_Stock.h"
#include "model/Model_Account.h"
#include "model/Model_Infotable.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Stock);

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
    m_frame = new TestFrameBase(instance_count);
    m_frame->Show(true);
   
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Stocks(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_Stock::tearDown()
{
    m_test_db.Close();
    delete m_frame;
    delete m_dbmodel;
}

void Test_Stock::test_dialog_add()
{
    m_dbmodel->Add_Account("AMP", Model_Account::INVESTMENT);
    int account_id = m_dbmodel->Add_Account("ACME Corp", Model_Account::INVESTMENT);
    m_dbmodel->Add_Account("Qwerty Keyboards", Model_Account::INVESTMENT);

    // create a new entry using the dialog.
    mmStockDialog* dlg = new mmStockDialog(m_frame, 0, account_id);

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
    Model_Stock::Data* my_entry = Model_Stock::instance().get(1);
    CPPUNIT_ASSERT(my_entry);

    double commission = my_entry->COMMISSION;
    double current_price = my_entry->CURRENTPRICE;
    double num_shares = my_entry->NUMSHARES;
    double purchase_price = my_entry->PURCHASEPRICE;

    // create a new entry using the dialog.
    mmStockDialog* dlg = new mmStockDialog(m_frame, my_entry, my_entry->HELDAT);
    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Stock Dialog Canclled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    if (id == wxID_OK)
    {
        Model_Stock::Data* my_new_entry = Model_Stock::instance().get(1);

        CPPUNIT_ASSERT(my_new_entry->COMMISSION == commission);
        CPPUNIT_ASSERT(my_new_entry->CURRENTPRICE == current_price);
        CPPUNIT_ASSERT(my_new_entry->NUMSHARES == num_shares);
        CPPUNIT_ASSERT(my_new_entry->PURCHASEPRICE == purchase_price);
        wxMessageBox("Stock Dialog Successful", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
}
//--------------------------------------------------------------------------
