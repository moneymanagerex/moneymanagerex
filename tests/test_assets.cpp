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
#include "test_assets.h"
#include "assetspanel.h"
#include "assetdialog.h"
#include "../src/constants.h"
#include "mmcheckingpanel.h"
#include "model/Model_TransferTrans.h"
#include "mmUserPanelTrans.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_ASSETS
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Asset);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Asset::Test_Asset()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_model_asset.mmb";
}

Test_Asset::~Test_Asset()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Asset::setUp()
{
    CpuTimer time("Startup");

    if (m_this_instance == 5)
    {
        m_base_frame = new TestFrameBase(m_this_instance);
        m_base_frame->Show(true);
        m_user_request = new TestFrameBase(m_base_frame);
        m_user_request->Show();
    }

    if (m_this_instance > 5)
    {
        m_base_frame = new TestFrameBase(m_this_instance);
        m_base_frame->Show(true);
    }

    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();

    m_test_db.Begin();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Set_Asset_Columns(&m_test_db);
    m_dbmodel->Set_Checking_Columns(&m_test_db);

    if (!Model_Account::Exist("Savings"))  m_dbmodel->Add_Bank_Account("Savings", 10000);
    if (!Model_Account::Exist("Cheque"))  m_dbmodel->Add_Bank_Account("Cheque", 5000);
    if (!Model_Account::Exist("Visa"))  m_dbmodel->Add_CreditCard_Account("Visa", -1000);
    if (!Model_Account::Exist("Mastercard"))  m_dbmodel->Add_CreditCard_Account("Mastercard", -2000);
    if (!Model_Account::Exist("Insurance"))  m_dbmodel->Add_Term_Account("Insurance");
    if (!Model_Account::Exist("ACME Corp"))  m_dbmodel->Add_Investment_Account("ACME Corp");
    if (!Model_Account::Exist("Qwerty Keyboards"))  m_dbmodel->Add_Investment_Account("Qwerty Keyboards");
    if (!Model_Account::Exist("AMP"))  m_dbmodel->Add_Investment_Account("AMP");

    if (m_dbmodel->PayeeNotExist("Telstra"))
    {
        m_dbmodel->Add_Payee("Telstra");

        int cat_id = m_dbmodel->Add_Category("Share");
        m_dbmodel->Add_Subcategory(cat_id, "Purchase");
        m_dbmodel->Add_Subcategory(cat_id, "Sale");
        m_dbmodel->Add_Subcategory(cat_id, "Dividend");

        m_dbmodel->Add_Payee_Category("Telstra", "Share", "Purchase");

    }

    if (m_dbmodel->PayeeNotExist("Domayne"))
    {
        m_dbmodel->Add_Payee("Domayne");
    }

    if (m_dbmodel->PayeeNotExist("Discount House"))
    {
        m_dbmodel->Add_Payee("Discount House");
        m_dbmodel->Add_Payee_Category("Discount House", "Homeneeds", "Furnishing");
    }

    m_test_db.Commit();
}

void Test_Asset::tearDown()
{
    delete m_dbmodel;
    m_test_db.Close();

    if (m_this_instance > 4)
    {
        delete m_base_frame;
    }
}

void Test_Asset::test_add()
{
    CpuTimer time("add");
    wxDateTime asset_date = wxDateTime::Today().Subtract(wxDateSpan::Years(5));
    int asset_id = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_APPRECIATE, 20.0, "Appreciate 20% pa 5 years");
    CPPUNIT_ASSERT(asset_id == 1);

    asset_id = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_DEPRECIATE, 20.0, "Depreciates 20% pa 5 years");
    CPPUNIT_ASSERT(asset_id == 2);

    asset_id = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_NONE, 20.0, "No change over 5 years");
    CPPUNIT_ASSERT(asset_id == 3);

    asset_id = m_dbmodel->Add_Asset("To be deleted", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_APPRECIATE, 20.0, "Appreciate 20% pa 5 years");
    CPPUNIT_ASSERT(asset_id == 4);

    Model_Asset::Data_Set selected_asset = Model_Asset::instance().find(Model_Asset::ASSETID(2));
    if (selected_asset.size() == 1)
    {
        Model_Asset::Data entry_2 = selected_asset[0];
        CPPUNIT_ASSERT(entry_2.VALUECHANGE == "Depreciates");

        Model_Asset::Data* new_entry = Model_Asset::instance().clone(&entry_2);
        asset_date = asset_date.Subtract(wxDateSpan::Years(5));
        new_entry->STARTDATE = asset_date.FormatISODate();
        new_entry->NOTES = "10 Year depreciation at 20% pa";
        Model_Asset::instance().save(new_entry);
    }

    Model_Asset::Data_Set assets = Model_Asset::instance().all();
    CPPUNIT_ASSERT(assets.size() == 5);
}

void Test_Asset::test_appreciate()
{
    Model_Asset::Data_Set asset_list = Model_Asset::instance().all();
    Model_Asset::Data entry = asset_list[0];

    double value = Model_Asset::value(entry);

    CPPUNIT_ASSERT(entry.VALUECHANGE == "Appreciates");
    CPPUNIT_ASSERT((value > 2488) && (value < 2489));       // values from V 0.9.9.0
    //CPPUNIT_ASSERT((value > 2000) && (value < 2001));     // values from v 0.9.9.2
}

void Test_Asset::test_depreciate()
{
    Model_Asset::Data_Set asset_list = Model_Asset::instance().all();
    Model_Asset::Data entry = asset_list[1];

    double value = Model_Asset::value(entry);

    CPPUNIT_ASSERT(entry.VALUECHANGE == "Depreciates");
    CPPUNIT_ASSERT((value > 327) && (value < 328));     // values from V 0.9.9.0         
    //CPPUNIT_ASSERT(value == 0);                       // values from v 0.9.9.2

    // Entry older than 5 years
    entry = asset_list[4];
    value = Model_Asset::value(entry);

    CPPUNIT_ASSERT(entry.VALUECHANGE == "Depreciates");
    CPPUNIT_ASSERT((value >= 0) && (value < 110));     // values from V 0.9.9.0         
    //CPPUNIT_ASSERT(value == 0);                       // values from v 0.9.9.2
}

void Test_Asset::test_remove()
{
    Model_Asset::instance().remove(4);
    Model_Asset::Data_Set assets = Model_Asset::instance().all();
    CPPUNIT_ASSERT(assets.size() == 4);
}

void Test_Asset::test_dialog()
{
    m_user_request->Show_InfoBarMessage(
        "Assets Dialog Test:\n\n"
        "\n\n"
        "Use Cancel to ignore test results.");

    mmAssetDialog* dlg = new mmAssetDialog(m_base_frame, 0);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Asset::Data_Set asset_table = Model_Asset::instance().all();
        CPPUNIT_ASSERT(asset_table.size() > 0);

        Model_Asset::Data bill_entry = asset_table.at(asset_table.size() - 1);
    }
}

void Test_Asset::test_assetpanel()
{
    // Get user details
    wxDateTime sale_date(wxDateTime::Today());
    wxDateTime purchase_date = sale_date;
    purchase_date.Subtract(wxDateSpan::Days(100));

    const wxString asset_name = "Single Bed";
    const wxString purchase_payee = "Domayne";          // store of purchase
    const wxString sale_account_name = "Savings";       // where money going to
    const wxString sale_payee = "Discount House";       // store of purchase
    const wxString category = "Homeneeds";
    const wxString subcategory = "Furnishing";
    double purchase_value = 800.0;
    double sale_value = 300.0;

    // Asset is created from assets, specifying Buy/withdrawal, Sell/income,
    int asset_id = m_dbmodel->Add_Asset(asset_name
        , purchase_date, purchase_value
        , Model_Asset::TYPE_HOUSE, Model_Asset::RATE_DEPRECIATE, 20, "Purchase of New Bed");

    /* Set up an asset transaction
       Purchase of the asset will be a withdrawal from the Savings account
       Sale of an asset will be a deposit in the checking account.
    */

    /*  Set up the purchase transaction */
    int purchase_entry_id = m_dbmodel->Add_Trans_Withdrawal(sale_account_name
        , purchase_date, purchase_payee, purchase_value
        , category, subcategory, "Asset: Bed - Purchase");
    Model_TransferTrans::SetAssetTransferTransaction(asset_id, purchase_entry_id
        , Model_TransferTrans::AS_TRANSFER);

    /*  Set up the maintenance transaction */
    int maintenance_entry_id = m_dbmodel->Add_Trans_Withdrawal(sale_account_name
        , sale_date, sale_payee, sale_value
        , category, "Others", "Asset: Bed - Fix");
    Model_TransferTrans::SetAssetTransferTransaction(asset_id, maintenance_entry_id
        , Model_TransferTrans::AS_INCOME_EXPENSE);

    /*  Set up the sale transaction */
    int sale_entry_id = m_dbmodel->Add_Trans_Deposit(sale_account_name
        , sale_date, sale_payee, sale_value
        , category, subcategory, "Asset: Bed - Sale");
    Model_TransferTrans::SetAssetTransferTransaction(asset_id, sale_entry_id
        , Model_TransferTrans::AS_TRANSFER);
    
    /* 
    The way this works

    ASSETS_V1: No change to table structure
    CHECKINGACCOUNT_V1: No change to table structure

    Identify the CHECKINGACCOUNT_V1 record as belonging to another table:
    TRANSCODE == Model_Checking::WITHDRAWAL or Model_Checking::DEPOSIT
    TOACCOUNTID > 0; // 1: process as is. 2 process as a transfer

    TRANSCODE = Model_Checking::TRANSFER
    TOACCOUNTID = ACCOUNTID; this is the other account

    Asset <--------- TransferTrans ---------------> Checking (Transaction record)
      |                   | <----------------------o find self in list to determine link type
      |                                            o TOACCOUNTID - determine external type 
      | <----------- TransferTrans ---------------> Checking (Transaction record)
      |                   | <----------------------o find self in list to determine link type
      |                                            o TOACCOUNTID - determine external type 
    */

    Model_TransferTrans::Data_Set trans_asset_list = Model_TransferTrans::TransferList(Model_TransferTrans::ASSETS, asset_id);
    Model_Checking::Data_Set trans_asset_records = Model_TransferTrans::TransferRecordList(Model_TransferTrans::ASSETS, asset_id);
    Model_TransferTrans::Data purchase_entry = Model_TransferTrans::TransferEntry(purchase_entry_id);

    CPPUNIT_ASSERT(purchase_entry.ID_CHECKINGACCOUNT == trans_asset_records[0].id());
    CPPUNIT_ASSERT(purchase_entry.ID_CHECKINGACCOUNT == trans_asset_list[0].ID_CHECKINGACCOUNT);
    /* Now to display the two rcords. */

    // Create a new frame anchored to the base frame.
    TestFrameBase* asset_frame = new TestFrameBase(m_base_frame, 670, 400);
    asset_frame->Show();

    // Create the panel under test
    mmAssetsPanel* asset_panel = new mmAssetsPanel(asset_frame, mmID_ASSETS);
    asset_panel->Show();

    TestFrameBase* account_frame = new TestFrameBase(m_base_frame, 670, 400);
    account_frame->Show();

    mmCheckingPanel* account_panel = new mmCheckingPanel(account_frame, 0, m_dbmodel->Get_account_id(sale_account_name));
    account_panel->Show();

    // Anchor the panels. Otherwise they will disappear.
    wxMessageBox(
        "Please Examine:\n\n"
        "  Asset Panel\n"
        "  Account panel\n"
        "\n\nContinue other tests ...",
        "Testing: Asset to Account", wxOK, wxTheApp->GetTopWindow());

    delete(asset_panel);
    delete(account_panel);

    Model_TransferTrans::RemoveTransferTransactions(Model_TransferTrans::ASSETS, asset_id);

    asset_panel = new mmAssetsPanel(asset_frame, mmID_ASSETS);
    asset_panel->Show();

    account_panel = new mmCheckingPanel(account_frame, 0, m_dbmodel->Get_account_id(sale_account_name));
    account_panel->Show();

    wxMessageBox(
        "Please re Examine:\n\n"
        "  Asset Panel\n"
        "  Account panel\n"
        "\n\nContinue other tests ...",
        "Testing: Asset to Account", wxOK, wxTheApp->GetTopWindow());
    delete(account_panel);
}

void Test_Asset::test_trans_user_panel()
{
    // Create a new frame anchored to the base frame.
    TestFrameBase* trans_user_panel_frame = new TestFrameBase(m_base_frame, 318, 455);
    trans_user_panel_frame->Show();

    // Create the panel under test
    mmUserPanelTrans* trans_user_panel = new mmUserPanelTrans(trans_user_panel_frame,0, wxID_ANY);
    trans_user_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: mmtrans_user_panel.\n\nContinue other tests ...",
        "Testing: Assets", wxOK, wxTheApp->GetTopWindow());

    mmAssetDialog* dlg = new mmAssetDialog(m_base_frame, 0);
    dlg->ShowModal();
}

//--------------------------------------------------------------------------
