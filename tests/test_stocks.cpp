/*******************************************************
Copyright (C) 2013 James Higley
Copyright (C) 2014..2015 Stefano Giorgio

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
#include "model/Model_TransferTrans.h"

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
    m_base_frame = new TestFrameBase(s_instance_count);
    m_base_frame->Show(true);
   
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();
    m_test_db.Begin();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Set_Stock_Columns(&m_test_db);
    m_dbmodel->Set_Checking_Columns(&m_test_db);
    m_test_db.Commit();
}

void Test_Stock::tearDown()
{
    m_test_db.Close();
    delete m_base_frame;
    delete m_dbmodel;
}

void Test_Stock::Test_Add_Stock_Dialog()
{
    m_dbmodel->Add_Investment_Account("ACME Corp");
    int account_id = m_dbmodel->Add_Investment_Account("AMP");
    m_dbmodel->Add_Investment_Account("Qwerty Keyboards");

    // create a new entry using the dialog.
    mmStockDialog* dlg = new mmStockDialog(m_base_frame, 0, account_id);

    int id = dlg->ShowModal();
    if (id == wxID_CANCEL)
    {
        wxMessageBox("Stock Dialog Cancelled", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
    if (id == wxID_OK)
    {
        Model_Stock::Data_Set stock_table = Model_Stock::instance().all();
        CPPUNIT_ASSERT(stock_table.size() > 0);
    }
}

void Test_Stock::Test_Edit_Stock_Dialog()
{
    Model_Stock::Data_Set stock_table = Model_Stock::instance().all();
    if (stock_table.size() < 1) return;

    Model_Stock::Data stock_entry = stock_table.at(stock_table.size() - 1);
    double commission = stock_entry.COMMISSION;
    double current_price = stock_entry.CURRENTPRICE;
    double num_shares = stock_entry.NUMSHARES;
    double purchase_price = stock_entry.PURCHASEPRICE;

    mmStockDialog* dlg = new mmStockDialog(m_base_frame, &stock_entry, stock_entry.HELDAT);
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

void Test_Stock::Test_Stocks_Panel()
{
    wxDate start_date(wxDate::Now());
    start_date.Subtract(wxDateSpan::Months(36));

    // Get User Details
    const wxString purchase_account = "Savings";
    const wxString dividend_account = "Cheque";
    const wxString stock_account = "Telstra";

    const wxString t1_share_account = "Telstra_T1";
    const int t1_number_share = 2000;
    const double t1_commission = 0;

    const wxDate t1_init_purchase_date = start_date;
    const double t1_init_share_price = 1.95; //1st part payment
    const double t1_init_purchase_value = t1_number_share * t1_init_share_price;

    const wxDate t1_final_purchase_date = start_date.Add(wxDateSpan::Months(8));
    const double t1_final_share_price = 2.00; //final payment added to T1
    const double t1_final_purchase_value = t1_number_share * t1_final_share_price;

    const wxDate t1_dividend_date_1 = start_date.Add(wxDateSpan::Months(6));
    const wxDate t1_dividend_date_2 = start_date.Add(wxDateSpan::Months(12));
    const double t1_dividend_value = 200.00;

    const wxDate t2_purchase_date = start_date.Add(wxDateSpan::Months(10));
    const wxString t2_share_account = "Telstra_T2";
    double t2_share_price = 7.95;
    int t2_share_number = 300;
    double t2_commission = 15;
    double t2_purchase_value = t2_share_price * t2_share_number + t2_commission;

    // Set up some data in table
    m_test_db.Begin();
    m_dbmodel->Add_Bank_Account(purchase_account, 10000);
    m_dbmodel->Add_Bank_Account(dividend_account);

    m_dbmodel->Add_Payee(stock_account);

    int cat_id = m_dbmodel->Add_Category("Share");
    m_dbmodel->Add_Subcategory(cat_id, "Purchase");
    m_dbmodel->Add_Subcategory(cat_id, "Sale");
    m_dbmodel->Add_Subcategory(cat_id, "Dividend");

    /* Create the Stock Account as the portfolio account */
    int stock_account_id = m_dbmodel->Add_Investment_Account(stock_account);
    
    /* Create the Share Account purchase entry*/
    int t1_shares_id = m_dbmodel->Add_Stock_Entry(stock_account_id, t1_init_purchase_date, t1_number_share
        , t1_init_share_price, t1_commission, t1_init_share_price, 0, t1_share_account, "TLS.AX", "Initial Purchase");

    int trans_id;
    /*  Set up the transfer table for t1 init purchase */
    trans_id = m_dbmodel->Add_Trans_Withdrawal(purchase_account, t1_init_purchase_date, stock_account, t1_init_purchase_value, "Share", "Purchase", "Telstra_T1: Purchase");
    Model_TransferTrans::SetShareTransferTransaction(t1_shares_id, trans_id, t1_init_share_price, t1_number_share, t1_commission, Model_TransferTrans::AS_TRANSFER);

    /*  Set up the transfer table for t1 final purchase */
    trans_id = m_dbmodel->Add_Trans_Withdrawal(purchase_account, t1_final_purchase_date, stock_account, t1_final_purchase_value, "Share", "Purchase", "Telstra_T1: Final Purchase");
    Model_TransferTrans::SetShareTransferTransaction(t1_shares_id, trans_id, t1_final_share_price, t1_number_share, Model_TransferTrans::AS_TRANSFER);

    /*  Set up the transfer table for the dividend_1*/
    trans_id = m_dbmodel->Add_Trans_Deposit(purchase_account, t1_dividend_date_1, stock_account, t1_dividend_value / 10, "Share", "Dividend", "Telstra_T1: Dividend");
    Model_TransferTrans::SetShareTransferTransaction(t1_shares_id, trans_id);

    /*  Set up the transfer table for the dividend_2*/
    trans_id = m_dbmodel->Add_Trans_Deposit(purchase_account, t1_dividend_date_2, stock_account, t1_dividend_value / 10, "Share", "Dividend", "Telstra_T1: Dividend");
    Model_TransferTrans::SetShareTransferTransaction(t1_shares_id, trans_id);

    /*  Set up the transfer table for t2 purchase */
    trans_id = m_dbmodel->Add_Trans_Withdrawal(purchase_account, t2_purchase_date, stock_account, t2_purchase_value, "Share", "Purchase", "Telstra_T2: Purchase");
    Model_TransferTrans::SetShareTransferTransaction(t1_shares_id, trans_id, t2_share_price, t2_share_number, t2_commission, Model_TransferTrans::AS_TRANSFER);

    /*
    Now to display the two rcords.
    */
    m_test_db.Commit();

    // Create a new frame anchored to the base frame.
    TestFrameBase* stocks_frame = new TestFrameBase(m_base_frame, 670, 400);
    stocks_frame->Show();

    // Create the panel under test
    mmStocksPanel* stocks_panel = new mmStocksPanel(stock_account_id, stocks_frame);
    stocks_panel->Show();

    TestFrameBase* account_frame = new TestFrameBase(m_base_frame, 670, 400);
    account_frame->Show();

    mmCheckingPanel* account_panel = new mmCheckingPanel(account_frame, 0, m_dbmodel->Get_account_id(purchase_account));
    account_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine:\n\n"
        "Stocks Panel\n"
        "Account panel\n"
        "\n\nContinue other tests ...",
        "Testing: Shares using Stocks Panel", wxOK, wxTheApp->GetTopWindow());

    delete(account_panel);
}
//--------------------------------------------------------------------------
