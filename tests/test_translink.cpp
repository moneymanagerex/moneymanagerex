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
#include "mmcheckingpanel.h"
#include "model/Model_Shareinfo.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_TRANSLINK
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
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_Model_Stocks(&m_test_db);
}

void Test_Translink::tearDown()
{
    m_test_db.Close();
    delete m_dbmodel;
}

void Test_Translink::Test_Australian_Currency()
{
    m_dbmodel->Init_BaseCurrency("AUD");
    int payee_id = m_dbmodel->Add_Payee("AUD Corporation", "Transfer");

    int broker_account_id = m_dbmodel->Add_Investment_Account("AUD Broker", 0.0, "", false, "AUD");
    int stock_entry_id = CreateStockEntry(broker_account_id, wxDateTime(wxDateTime::Today().Subtract(wxDateSpan::Months(7))), "AMP AUD", "AMP.AX", 10000.1234, 2.3456);

    int share_account_id = m_dbmodel->Add_Account("AMP AUD", Model_Account::SHARES, 0.0, "", false, "AUD");
    CreateShareEntry(stock_entry_id, payee_id, wxDateTime(wxDateTime::Today().Subtract(wxDateSpan::Months(7))));

    //Set up the broker panel
    TestFrameBase broker_base_frame(m_this_instance);
    broker_base_frame.Show(true);
    TestFrameBase* broker_frame = new TestFrameBase(&broker_base_frame, 670, 400);
    broker_frame->Show();
    mmStocksPanel* broker_panel = new mmStocksPanel(broker_account_id, NULL, broker_frame);
    broker_panel->Show();

    TestFrameBase* user_request = new TestFrameBase(&broker_base_frame);
    user_request->Show();
    user_request->Show_InfoBarMessage(
        "1. Currency: Australian Dollar\n\n"
        "* Thousands separator: comma\n"
        "* Decimal separation: dot\n"
        "10,000.1234 Shares @ 2.3456 = 23,456.29\n\n");


    // Set up the Shares panel
    TestFrameBase share_base_frame(s_instance_count);
    share_base_frame.Show(true);
    TestFrameBase* share_frame = new TestFrameBase(&share_base_frame, 650, 380);
    share_frame->Show();
    mmCheckingPanel* share_panel = new mmCheckingPanel(share_frame, 0, share_account_id);
    share_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please examine panels: Stock and Shares.\n\nContinue other tests ...",
        "Testing: Stocks and Shares", wxOK, wxTheApp->GetTopWindow());
}
//--------------------------------------------------------------------------

void Test_Translink::Test_Argentina_Currency()
{
    m_dbmodel->Init_BaseCurrency("ARS");
    int payee_id = m_dbmodel->Add_Payee("ARS Corporation", "Transfer");

    int broker_account_id = m_dbmodel->Add_Investment_Account("ARS Broker", 0.0, "", false, "ARS");
    int stock_entry_id = CreateStockEntry(broker_account_id, wxDateTime(wxDateTime::Today().Subtract(wxDateSpan::Months(14))), "AMP ARS", "AMP.AX", 10000.1234, 2.3456);

    int share_account_id = m_dbmodel->Add_Account("AMP ARS", Model_Account::SHARES, 0.0, "", false, "ARS");
    CreateShareEntry(stock_entry_id, payee_id, wxDateTime(wxDateTime::Today().Subtract(wxDateSpan::Months(14))));

    //Set up the broker panel
    TestFrameBase broker_base_frame(m_this_instance);
    broker_base_frame.Show(true);
    TestFrameBase* broker_frame = new TestFrameBase(&broker_base_frame, 670, 400);
    broker_frame->Show();
    mmStocksPanel* broker_panel = new mmStocksPanel(broker_account_id, NULL, broker_frame);
    broker_panel->Show();

    TestFrameBase* user_request = new TestFrameBase(&broker_base_frame);
    user_request->Show();
    user_request->Show_InfoBarMessage(
        "1. Currency: Argentine peso\n\n"
        "* Thousands separator: dot\n"
        "* Decimal separation: comma\n"
        "10.000,1234 Shares @ 2,3456 = 23.456,29\n\n");

    // Set up the Shares panel
    TestFrameBase share_base_frame(s_instance_count);
    share_base_frame.Show(true);
    TestFrameBase* share_frame = new TestFrameBase(&share_base_frame, 650, 380);
    share_frame->Show();
    mmCheckingPanel* share_panel = new mmCheckingPanel(share_frame, 0, share_account_id);
    share_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please examine panels: Stock and Shares.\n\nContinue other tests ...",
        "Testing: Stocks and Shares", wxOK, wxTheApp->GetTopWindow());
}
//--------------------------------------------------------------------------

int Test_Translink::CreateStockEntry(int broker_account_id, wxDateTime date, const wxString& company_name, const wxString stock_symbol,
    double num_shares, double price)
{
    Model_Stock::Data* stock_entry = Model_Stock::instance().create();
    stock_entry->HELDAT = broker_account_id;
    stock_entry->PURCHASEDATE = date.FormatISODate();
    stock_entry->STOCKNAME = company_name;
    stock_entry->SYMBOL = stock_symbol;
    stock_entry->NUMSHARES = num_shares;
    stock_entry->PURCHASEPRICE = price;
    stock_entry->NOTES = "";
    stock_entry->CURRENTPRICE = price;
    stock_entry->VALUE = num_shares * price;
    stock_entry->COMMISSION = 0.0;
    return Model_Stock::instance().save(stock_entry);
}

void Test_Translink::CreateShareEntry(int stock_entry_id, int payee_id, wxDateTime date,
    const wxString& category, const wxString& subcategory,
    int trans_code, Model_Translink::CHECKING_TYPE trans_type)
{
    Model_Stock::Data* stock_entry = Model_Stock::instance().get(stock_entry_id);
    Model_StockHistory::instance().addUpdate(stock_entry->SYMBOL, date, stock_entry->CURRENTPRICE, Model_StockHistory::MANUAL);

    int share_account_id = m_dbmodel->Get_account_id(stock_entry->STOCKNAME);
    Model_Checking::Data* share_entry = Model_Checking::instance().create();
    share_entry->ACCOUNTID = share_account_id;
    share_entry->TOACCOUNTID = trans_type;
    share_entry->PAYEEID = payee_id;
    share_entry->TRANSCODE = Model_Checking::instance().all_type()[trans_code];
    share_entry->TRANSAMOUNT = (stock_entry->NUMSHARES * stock_entry->PURCHASEPRICE) + stock_entry->COMMISSION;
    share_entry->STATUS = Model_Checking::all_status()[trans_code].Mid(0, 1);
    share_entry->TRANSACTIONNUMBER = Model_Account::get_account_name(stock_entry->HELDAT);
    share_entry->NOTES = stock_entry->NOTES;
    share_entry->CATEGID = m_dbmodel->Get_category_id(category);
    share_entry->SUBCATEGID = -1;
    share_entry->TRANSDATE = stock_entry->PURCHASEDATE;
    share_entry->FOLLOWUPID = 0;
    share_entry->TOTRANSAMOUNT = share_entry->TRANSAMOUNT;
    int share_entry_id = Model_Checking::instance().save(share_entry);

    Model_Translink::SetStockTranslink(stock_entry_id, share_entry_id, trans_type);
    Model_Shareinfo::ShareEntry(share_entry_id, stock_entry->NUMSHARES, stock_entry->PURCHASEPRICE, stock_entry->COMMISSION, "");
    Model_Translink::UpdateStockValue(stock_entry);
}
