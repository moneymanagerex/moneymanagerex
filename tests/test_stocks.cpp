/*******************************************************
Copyright (C) 2013 James Higley
Copyright (C) 2014..2016 Stefano Giorgio

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
#include "test_stocks.h"
#include "stockdialog.h"
#include "stockspanel.h"
#include "mmcheckingpanel.h"
#include "sharetransactiondialog.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_STOCKS
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Stock);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Stock::Test_Stock()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_model_stock.mmb";

    if (m_this_instance == 1)
    {
        if (wxFileExists(m_test_db_filename))
        {
            wxRemoveFile(m_test_db_filename);
        }
    }
}

Test_Stock::~Test_Stock()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Stock::setUp()
{
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_Model_Stocks(&m_test_db);
}

void Test_Stock::tearDown()
{
    m_test_db.Close();
    delete m_dbmodel;
}

void Test_Stock::Test_Add_Stock_Dialog()
{
    // Set up the window base
    TestFrameBase base_frame(m_this_instance);
    base_frame.Show(true);

    TestFrameBase* user_request = new TestFrameBase(&base_frame);
    user_request->Show();
    user_request->Show_InfoBarMessage(
        "1. **IMPORTANT** Use Company Name: AMP\n\n"
        "* Thousands separator: dot\n"
        "* Decimal separation: comma\n"
        "10.000,1234 Shares @ 2,3456 = 23.456,29\n\n"
        "Use Close to progress to next test.\n");

    int broker_account_id = m_dbmodel->Add_Investment_Account("Broker",0.0,"",false,"ARS");
    m_dbmodel->Add_Account("AMP", Model_Account::SHARES, 0.0, "", false, "ARS");
    m_dbmodel->Add_Payee("AMP Corporation", "Transfer");
    m_dbmodel->Init_BaseCurrency("ARS");

    // create a new entry using the dialog.
    mmStockDialog* dlg = new mmStockDialog(&base_frame, NULL, 0, broker_account_id);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        Model_Stock::Data_Set stock_table = Model_Stock::instance().all();
        CPPUNIT_ASSERT(stock_table.size() > 0);
    }

    Model_Stock::Data_Set stock_table = Model_Stock::instance().find(Model_Stock::HELDAT(broker_account_id), Model_Stock::STOCKNAME("AMP"));
    CPPUNIT_ASSERT(stock_table.size() == 1);

    Model_Stock::Data stock = stock_table.at(0);

    ShareTransactionDialog share_dialog(&base_frame, &stock);
    share_dialog.ShowModal();

}

void Test_Stock::Test_Edit_Stock_Dialog()
{
    // Set up the window base
    TestFrameBase base_frame(m_this_instance);
    base_frame.Show(true);

    Model_Stock::Data_Set stock_table = Model_Stock::instance().all();
    if (stock_table.size() < 1) return;

    Model_Stock::Data stock_entry = stock_table.at(stock_table.size() - 1);
    double commission = stock_entry.COMMISSION;
    double current_price = stock_entry.CURRENTPRICE;
    double num_shares = stock_entry.NUMSHARES;
    double purchase_price = stock_entry.PURCHASEPRICE;

    mmStockDialog* dlg = new mmStockDialog(&base_frame, NULL, &stock_entry, stock_entry.HELDAT);
    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        stock_table = Model_Stock::instance().all();
        Model_Stock::Data new_stock_entry = stock_table.at(stock_table.size() - 1);

        CPPUNIT_ASSERT(stock_entry.HELDAT == m_dbmodel->Get_account_id("Broker"));
        CPPUNIT_ASSERT(new_stock_entry.COMMISSION == commission);
        CPPUNIT_ASSERT(new_stock_entry.CURRENTPRICE == current_price);
        CPPUNIT_ASSERT(new_stock_entry.NUMSHARES == num_shares);
        CPPUNIT_ASSERT(new_stock_entry.PURCHASEPRICE == purchase_price);
    }
}

void Test_Stock::Test_Stock_Panels()
{
    //Create the Broker account.
    int broker_account_id = m_dbmodel->Get_account_id("Broker");

    // Set up the window base. This will destroy all associated test frames.
    TestFrameBase broker_base_frame(m_this_instance);
    broker_base_frame.Show(true);

    // Create a new frame anchored to the base frame.
    TestFrameBase* broker_frame = new TestFrameBase(&broker_base_frame, 670, 400);
    broker_frame->Show();

    // Create the broker panel
    mmStocksPanel* broker_panel = new mmStocksPanel(broker_account_id, NULL, broker_frame);
    broker_panel->Show();
    
    TestFrameBase* user_request = new TestFrameBase(&broker_base_frame);
    user_request->Show();
    user_request->Show_InfoBarMessage(
        "1. Currency: Argentine peso\n\n"
        "* Thousands separator: dot\n"
        "* Decimal separation: comma\n"
        "10.000,1234 Shares @ 2,3456 = 23.456,29\n\n");

    //Locate the Shares account.
    int share_account_id = m_dbmodel->Get_account_id("AMP");
    // Set up the window base. This will destroy all associated test frames.
    TestFrameBase share_base_frame(s_instance_count);
    share_base_frame.Show(true);

    // Create a new frame anchored to the base frame.
    TestFrameBase* share_frame = new TestFrameBase(&share_base_frame, 650, 380);
    share_frame->Show();

    // Create the share panel
    mmCheckingPanel* share_panel = new mmCheckingPanel(share_frame, 0, share_account_id);
    share_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please examine panels: Stock and Shares.\n\nContinue other tests ...",
        "Testing: Stocks Panel", wxOK, wxTheApp->GetTopWindow());
}
//--------------------------------------------------------------------------
