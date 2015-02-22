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
#include "stock_transdialog.h"
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

    if (m_dbmodel->AccountNotExist("Savings")) m_dbmodel->Add_Bank_Account("Savings", 10000);
    if (m_dbmodel->AccountNotExist("Cheque"))  m_dbmodel->Add_Bank_Account("Cheque", 5000,"",false,"USD");
    if (m_dbmodel->AccountNotExist("Visa"))    m_dbmodel->Add_CreditCard_Account("Visa", -1000);
    if (m_dbmodel->AccountNotExist("Mastercard")) m_dbmodel->Add_CreditCard_Account("Mastercard", -2000);
    if (m_dbmodel->AccountNotExist("Insurance"))  m_dbmodel->Add_Term_Account("Insurance");
    if (m_dbmodel->AccountNotExist("ACME Corp"))  m_dbmodel->Add_Term_Account("ACME Corp");
    if (m_dbmodel->AccountNotExist("Yahoo Finance"))  m_dbmodel->Add_Investment_Account("Yahoo Finance");
    if (m_dbmodel->AccountNotExist("Google Finance")) m_dbmodel->Add_Investment_Account("Google Finance");

    if (m_dbmodel->PayeeNotExist("AMP"))
    {
        m_dbmodel->Add_Payee("AMP");

        int cat_id = m_dbmodel->Add_Category("Share");
        m_dbmodel->Add_Subcategory(cat_id, "Purchase");
        m_dbmodel->Add_Subcategory(cat_id, "Sale");
        m_dbmodel->Add_Subcategory(cat_id, "Dividend");

        m_dbmodel->Add_Payee_Category("AMP", "Share", "Purchase");
    }

    if (m_dbmodel->PayeeNotExist("Yahoo Finance"))
    {
        m_dbmodel->Add_Payee("Yahoo Finance");
    }

    m_dbmodel->SetCurrencyExchangeRate("USD", 1.5);
    m_test_db.Commit();
}

void Test_Stock::tearDown()
{
    m_test_db.Close();
    delete m_base_frame;
    delete m_dbmodel;
}

void Test_Stock::Test_Add_StockTrans_Dialog()
{
    // Create a stock entry
    int stock_id = m_dbmodel->Add_Stock_Entry("Yahoo Finance", wxDateTime::Now().Subtract(wxDateSpan::Years(2)), 1000, 4.5, 500, 4.5, 5000, "AMP", "AMP.AX", "AMP Shares");
    Model_Stock::Data* stock_entry = Model_Stock::instance().get(stock_id);

    // create a new entry transaction using the dialog.
    mmStockTransDialog* dlg = new mmStockTransDialog(m_base_frame, stock_entry, stock_entry->HELDAT);

    int id = dlg->ShowModal();
    while (id == wxID_SAVE)
    {
        id = dlg->ShowModal();
    }
}

void Test_Stock::Test_Edit_Stock_Dialog()
{
    Model_Stock::Data_Set stock_table = Model_Stock::instance().all();
    if (stock_table.size() < 1) return;

    Model_Stock::Data stock_entry = stock_table.at(stock_table.size() - 1);
    //double commission = stock_entry.COMMISSION;
    //double current_price = stock_entry.CURRENTPRICE;
    //double num_shares = stock_entry.NUMSHARES;
    //double purchase_price = stock_entry.PURCHASEPRICE;

    mmStockDialog* dlg = new mmStockDialog(m_base_frame, &stock_entry, stock_entry.HELDAT);
    int id = dlg->ShowModal();
    while (id == wxID_SAVE)
    {
        id = dlg->ShowModal();
    }

    //if (saves > 0)
    //{
    //    stock_table = Model_Stock::instance().all();
    //    Model_Stock::Data new_stock_entry = stock_table.at(stock_table.size() - 1);

    //    CPPUNIT_ASSERT(stock_entry.HELDAT == m_dbmodel->Get_account_id("Yahoo Finance"));
    //    CPPUNIT_ASSERT(new_stock_entry.COMMISSION == commission);
    //    CPPUNIT_ASSERT(new_stock_entry.CURRENTPRICE == current_price);
    //    CPPUNIT_ASSERT(new_stock_entry.NUMSHARES == num_shares);
    //    CPPUNIT_ASSERT(new_stock_entry.PURCHASEPRICE == purchase_price);
    //}
}

void Test_Stock::Test_Stocks_Panel()
{
    /* Get the Stock Portfolio Account */
    int stock_account_id = m_dbmodel->Get_account_id("Yahoo Finance");

    /* Now to display the two rcords. */

    // Create the stocks frame anchored to the base frame, containing the stocks panel.
    TestFrameBase* stocks_frame = new TestFrameBase(m_base_frame, 670, 400);
    new mmStocksPanel(stock_account_id, stocks_frame);
    stocks_frame->Show();

    // Create the account frame anchored to the base frame, containing the account panel.
    TestFrameBase* savings_account_frame = new TestFrameBase(m_base_frame, 670, 400);
    new mmCheckingPanel(savings_account_frame, 0, m_dbmodel->Get_account_id("Savings"));
    savings_account_frame->Show();

    // Create the account frame anchored to the base frame, containing the account panel.
    TestFrameBase* cheque_account_frame = new TestFrameBase(m_base_frame, 670, 400);
    new mmCheckingPanel(cheque_account_frame, 0, m_dbmodel->Get_account_id("Cheque"));
    cheque_account_frame->Show();

    // Anchor the panel. Otherwise it will disappear in the test environment.
    wxMessageBox("Please Examine:\n\n"
        "Stocks Panel,\n"
        "AUD Savings Account,      USD Cheque Account\n"
        "\nContinue other tests ...",
        "Testing: Shares using Stocks Panel", wxOK, wxTheApp->GetTopWindow());
}
//--------------------------------------------------------------------------
