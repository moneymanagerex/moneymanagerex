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
#include "test_translink.h"
#include "stockdialog.h"
#include "stockspanel.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TEST_TRANSLINK
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Translink);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Translink::Test_Translink()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_translink.mmb";

    if (m_this_instance == 1)
    {
        if (wxFileExists(m_test_db_filename))
        {
            wxRemoveFile(m_test_db_filename);
        }
    }
}

Test_Translink::~Test_Translink()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Translink::setUp()
{
    m_base_frame = new TestFrameBase(s_instance_count);
    m_base_frame->Show(true);
   
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_Model_Stocks(&m_test_db);
}

void Test_Translink::tearDown()
{
    m_test_db.Close();
    delete m_base_frame;
    delete m_dbmodel;
}

void Test_Translink::Test_Add_Stock_Dialog()
{
    m_dbmodel->Add_Investment_Account("ACME Corp");
    int account_id = m_dbmodel->Add_Investment_Account("AMP");
    m_dbmodel->Add_Investment_Account("Qwerty Keyboards");

    // create a new entry using the dialog.
    mmStockDialog* dlg = new mmStockDialog(m_base_frame, NULL, 0, account_id);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Stock Dialog Canclled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    if (id == wxID_OK)
    {
        Model_Stock::Data_Set stock_table = Model_Stock::instance().all();
        CPPUNIT_ASSERT(stock_table.size() > 0);
    }
}

void Test_Translink::Test_Edit_Stock_Dialog()
{
    Model_Stock::Data_Set stock_table = Model_Stock::instance().all();
    if (stock_table.size() < 1) return;

    Model_Stock::Data stock_entry = stock_table.at(stock_table.size() - 1);
    double commission = stock_entry.COMMISSION;
    double current_price = stock_entry.CURRENTPRICE;
    double num_shares = stock_entry.NUMSHARES;
    double purchase_price = stock_entry.PURCHASEPRICE;

    mmStockDialog* dlg = new mmStockDialog(m_base_frame, NULL, &stock_entry, stock_entry.HELDAT);
    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Stock Dialog Canclled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    else if (id == wxID_OK)
    {
        stock_table = Model_Stock::instance().all();
        Model_Stock::Data new_stock_entry = stock_table.at(stock_table.size() - 1);

        CPPUNIT_ASSERT(stock_entry.HELDAT == m_dbmodel->Get_account_id("AMP"));
        CPPUNIT_ASSERT(new_stock_entry.COMMISSION == commission);
        CPPUNIT_ASSERT(new_stock_entry.CURRENTPRICE == current_price);
        CPPUNIT_ASSERT(new_stock_entry.NUMSHARES == num_shares);
        CPPUNIT_ASSERT(new_stock_entry.PURCHASEPRICE == purchase_price);
    }
}

void Test_Translink::Test_Stocks_Panel()
{
    // Create a new frame anchored to the base frame.
    TestFrameBase* stocks_frame = new TestFrameBase(m_base_frame, 670, 400);
    stocks_frame->Show();

    int stock_Account_id = m_dbmodel->Get_account_id("AMP");
    m_dbmodel->Add_Stock_Entry(stock_Account_id, wxDate::Now().Subtract(wxDateSpan::Years(5)), 1000, 4.2575, 0, 0, 0, "AMP Initial Share Purchase", "AMP.ax");
    m_dbmodel->Add_Stock_Entry(stock_Account_id, wxDate::Now().Subtract(wxDateSpan::Months(6)), 9550, 5.2575, 0, 0, 0, "AMP Suplement Purchase", "AMP.ax");
    m_dbmodel->Add_Stock_Entry(stock_Account_id, wxDate::Now().Subtract(wxDateSpan::Years(4)), 5, 5.2775, 0, 0, 0, "DRP", "AMP.ax");
    m_dbmodel->Add_Stock_Entry(stock_Account_id, wxDate::Now().Subtract(wxDateSpan::Years(3)), 10, 6.1575, 0, 0, 0, "DRP ", "amp.ax");
    m_dbmodel->Add_Stock_Entry(stock_Account_id, wxDate::Now().Subtract(wxDateSpan::Years(2)), 100, 5.4575, 0, 0, 0, "DRP", "AMP.AX");
    m_dbmodel->Add_Stock_Entry(stock_Account_id, wxDate::Now().Subtract(wxDateSpan::Years(1)), 1000, 4.2775, 0, 0, 0, "DRP", "AMP.ax");
    
    // Create the panel under test
    mmStocksPanel* stocks_panel = new mmStocksPanel(stock_Account_id, NULL, stocks_frame);
    stocks_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: Stocks Panel.\n\nContinue other tests ...",
        "Testing: Stocks Panel", wxOK, wxTheApp->GetTopWindow());
}
//--------------------------------------------------------------------------
